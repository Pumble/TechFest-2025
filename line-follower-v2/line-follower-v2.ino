// PENDIENTE: EL MOTOR DERECHO VA MUY LENTO - AJUSTAR HARDWARE O VELOCIDAD?

/* ================================  VARS ================================ */

// PINES DE PUENTE H
const int AIA = 6;   // PWM motor A (antes 8 no-PWM)
const int AIB = 7;   // dirección motor A (digital)
const int BIA = 10;  // PWM motor B
const int BIB = 9;   // dirección motor B (digital)
byte SPEED = 200;

// PINES DE MODULO DE SEGUIDOR DE LINEA
// Cuando hay una deteccion, cambia a LOW, default: HIGH
const int S2 = A5;                         // Izquierda
const int S3 = A4;                         // Centro
const int S4 = A3;                         // Derecha
const unsigned long SENSOR_INTERVAL = 30;  // ms entre lecturas
const int SENSOR_CONFIRM_COUNT = 2;        // lecturas consecutivas necesarias (1 = sin debounce, respuesta rápida)

// Tiempo de espera antes de iniciar el robot
unsigned long WAIT_FOR_START = 3000;

// Enum para manejar estados del robot
enum State {
  STOPPED,
  SEARCHING,
  FOLLOWING_LINE
};
const unsigned long STATE_INTERVAL = 60;  // ms entre actualizaciones de estado

// timeout de seguridad para giros (ms)
const unsigned long TURN_MAX_MS = 3500;  // aumentado para giros lentos

/* ================================  CURRENT STATE ================================ */
State CURRENT_STATE = STOPPED;
bool D2_STATE = false;
bool D3_STATE = false;  // estado central (actualizado por muestreo)
bool D4_STATE = false;
unsigned long LAST_SENSOR_MILLIS = 0;
unsigned long LAST_STATE_MILLIS = 0;c:\Users\Juan\Desktop\TechFest-2025\sumo-bot\sumo-bot.ino
// Flags para giros no bloqueantes
bool TURNING_LEFT_UNTIL_CENTER = false;
unsigned long LEFT_START_MILLIS = 0;
bool TURNING_RIGHT_UNTIL_CENTER = false;
unsigned long RIGHT_START_MILLIS = 0;
int D2_COUNT = 0;
int D3_COUNT = 0;
int D4_COUNT = 0;
// timeout por tiempo en ms (mejor para motores lentos)
unsigned long LAST_SEEN_LINE_MS = 0;
const unsigned long NO_LINE_TIMEOUT_MS = 3000;          // umbral base (ms) - ajustado para motores lentos
const unsigned long TURNING_NO_LINE_TIMEOUT_MS = 5000;  // umbral cuando se está girando (más tolerancia)

// Ventana de seguridad después de cualquier movimiento: evita entrar a SEARCHING
unsigned long LAST_MOVE_MS = 0;
const unsigned long JUST_MOVED_MS = 1500;  // ms donde ignoramos temporalmente SEARCHING (aumentado para motores lentos)

// --- Variables para búsqueda no bloqueante ---
bool SEARCH_ACTIVE = false;
int SEARCH_PHASE = 0;  // 0:left, 1:right, 2:forward
unsigned long SEARCH_PHASE_START = 0;
int SEARCH_MOVING_TIME = 120;  // ms inicio giro/avance
int SEARCH_ATTEMPTS = 0;
const int SEARCH_MAX_ATTEMPTS = 12;
const int SEARCH_MOVE_INCREMENT = 60;  // incremento ms por intento
const int SEARCH_MAX_MOVE_MS = 800;

// ----------------- Telemetría LED (no bloqueante) -----------------
unsigned long LAST_STATUS_TICK = 0;
const unsigned long STATUS_INTERVAL_MS = 2000;  // cada cuánto iniciar patrón
const unsigned long STATUS_STEP_MS = 120;       // tiempo entre pasos del patrón
int statusPhase = 0;                            // 0=idle, 1=state blinks, 2=gap, 3=sensor pulses, 4=done
int stateBlinksRemaining = 0;
int stateBlinkOn = 0;
int sensorIndex = 0;
unsigned long statusStepStart = 0;

// ----------------- Centro estable durante giros -----------------
const unsigned long CENTER_HOLD_MS = 300;  // tiempo que S3 debe estar detectando antes de acabar giro (aumentado para giros lentos)
unsigned long centerDetectStart = 0;
bool centerDetecting = false;

/**
 * @brief Configuración inicial del robot.
 *
 * Configura pines, comunicación serial y espera antes de iniciar.
 */
void setup() {
  hBridge();
  serial();
  lineFollower();

  softStop();
  // registrar momento inicial en el que consideramos la línea "vista"
  LAST_SEEN_LINE_MS = millis();
  // registrar instante inicial de movimiento (evitar SEARCHING justo al arrancar)
  LAST_MOVE_MS = millis();
  delay(WAIT_FOR_START);  // esperar antes de iniciar
  CURRENT_STATE = FOLLOWING_LINE;
}

void loop() {
  // Actualizar sensores sin bloquear
  pollSensorsNonBlocking();

  // Procesar tareas de giro no bloqueantes
  processTurningTasks();

  // Actualizar estado sin bloquear (ejecuta periodicamente)
  updateStateNonBlocking();

  // Telemetría LED no bloqueante (debug offline)
  indicateStatusNonBlocking();

  // Usar el ultimo estado actualizado
  // sd2: snapshot de D2
  bool sd2 = D2_STATE;
  bool sd3 = D3_STATE;
  bool sd4 = D4_STATE;

  // Actualizar estado
  updateCurrentState(sd2, sd3, sd4);

  // indicar con LED_BUILTIN: encendido únicamente cuando seguimos la línea
  if (CURRENT_STATE == FOLLOWING_LINE) digitalWrite(LED_BUILTIN, HIGH);
  else digitalWrite(LED_BUILTIN, LOW);

  switch (CURRENT_STATE) {
    case SEARCHING:
      searching();
      break;
    case FOLLOWING_LINE:
      // Serial.println("Siguiendo la linea");
      /* Ejecutar acción según qué sensor detecta la línea:
       * - sd2 (izquierda): iniciar giro a la izquierda hasta que S3 vea la línea.
       * - sd3 (centro): avanzar recto.
       * - sd4 (derecha): iniciar giro a la derecha hasta que S3 vea la línea.
       * - ninguno: detenerse (no deberíamos llegar aquí en FOLLOWING_LINE).
       */

      // Ejecutar únicamente la acción prioritaria en esta iteración
      if (sd2) {  // izquierda
        startLeftUntilCenter();
      } else if (sd3) {  // centro
        forward();
      } else if (sd4) {  // derecha
        startRightUntilCenter();
      }
      // Si ningún sensor detecta, NO llamar softStop aquí — dejar que el movimiento
      // actual continúe hasta que updateCurrentState cambie a SEARCHING.
      // Esto evita paradas innecesarias durante pérdidas momentáneas de línea.

      break;
    default:
      Serial.println("No se que hacer, ayuda!");
      break;
  }
}

/* ================================  SETUPS ================================ */

/**
 * @brief Configura los pines del puente H como salidas.
 *
 * No tiene parámetros ni valor de retorno. Debe llamarse en setup().
 */
void hBridge() {
  pinMode(AIA, OUTPUT);  // fijar los pines como salidas
  pinMode(AIB, OUTPUT);
  pinMode(BIA, OUTPUT);
  pinMode(BIB, OUTPUT);
}

/**
 * @brief Inicializa la comunicación serial para depuración.
 *
 * Configura Serial a 9600 baudios e imprime un mensaje inicial.
 */
void serial() {
  Serial.begin(9600);  // Inicia la comunicación a 9600 baudios
  Serial.println("Hola, consola serial!");
}

/* ================================  ACTIONS ================================ */

/**
 * @brief Configura los pines del módulo seguidor de línea como entradas.
 *
 * Activa los 3 pines del sensor (S2, S3, S4) y configura LED_BUILTIN como salida
 * para indicación de estado. No retorna valor.
 */
void lineFollower() {
  pinMode(S2, INPUT);
  pinMode(S3, INPUT);
  pinMode(S4, INPUT);

  // configurar LED integrado como salida (se usa para indicar FOLLOWING_LINE)
  pinMode(LED_BUILTIN, OUTPUT);
}

/**
 * @brief Actualiza el estado CURRENT_STATE según lecturas de sensores y timeouts.
 *
 * Lógica de decisión:
 * - Si estamos en un giro controlado (TURNING_*) -> mantiene FOLLOWING_LINE.
 * - Si algún sensor detecta línea -> FOLLOWING_LINE y resetea LAST_SEEN_LINE_MS.
 * - Si acabamos de mover (dentro de JUST_MOVED_MS) -> mantiene FOLLOWING_LINE.
 * - Si no hay detección y han pasado NO_LINE_TIMEOUT_MS -> cambia a SEARCHING.
 *
 * @param d2 Estado del sensor izquierdo (true = detecta línea)
 * @param d3 Estado del sensor central (true = detecta línea)
 * @param d4 Estado del sensor derecho  (true = detecta línea)
 */
void updateCurrentState(bool d2, bool d3, bool d4) {
  unsigned long now = millis();

  // Si estamos ejecutando un giro controlado, mantenemos FOLLOWING_LINE
  if (TURNING_LEFT_UNTIL_CENTER || TURNING_RIGHT_UNTIL_CENTER) {
    CURRENT_STATE = FOLLOWING_LINE;
    return;
  }

  // si hay detección inmediata -> seguir
  if (d2 || d3 || d4) {
    CURRENT_STATE = FOLLOWING_LINE;
    LAST_SEEN_LINE_MS = now;
    return;
  }

  // si recientemente hemos movido el robot, damos una ventana donde no pasaremos a SEARCHING
  if (now - LAST_MOVE_MS < JUST_MOVED_MS) {
    CURRENT_STATE = FOLLOWING_LINE;
    return;
  }

  // calcular cuánto tiempo hace que no vemos la línea
  unsigned long elapsed = now - LAST_SEEN_LINE_MS;
  unsigned long threshold = NO_LINE_TIMEOUT_MS;

  // durante giros controlados, ampliar el umbral de tiempo permitido sin línea
  if (TURNING_LEFT_UNTIL_CENTER || TURNING_RIGHT_UNTIL_CENTER) threshold = TURNING_NO_LINE_TIMEOUT_MS;

  if (elapsed >= threshold) {
    // pasa a búsqueda solo si el timeout de ms ha expirado
    CURRENT_STATE = SEARCHING;
  } else {
    // aun dentro del tiempo de tolerancia, seguir intentando seguir
    CURRENT_STATE = FOLLOWING_LINE;
  }
}

/**
 * @brief Búsqueda no bloqueante cuando no hay línea detectada.
 *
 * Alterna: girar izquierda -> girar derecha -> avanzar corto.
 * Aumenta duración gradualmente. Finaliza cuando algún sensor detecta la línea
 * (D2_STATE || D3_STATE || D4_STATE) o cuando supera intentos máximos.
 */
void searching() {
  /* Hay que buscar a la izquierda durante un tiempo
   * si no encuentra, buscar a la derecha
   * sino, podemos echar para atras un poco
   * y volver a buscar hasta encontrar algo
   */

  Serial.println("Estoy buscando...");

  // Si en algún momento se detecta línea salir inmediatamente
  if (D2_STATE || D3_STATE || D4_STATE) {
    SEARCH_ACTIVE = false;
    SEARCH_ATTEMPTS = 0;
    SEARCH_PHASE = 0;
    softStop();
    CURRENT_STATE = FOLLOWING_LINE;
    return;
  }

  unsigned long now = millis();

  // iniciar búsqueda si no está activa
  if (!SEARCH_ACTIVE) {
    SEARCH_ACTIVE = true;
    SEARCH_PHASE = 0;
    SEARCH_PHASE_START = now;
    SEARCH_MOVING_TIME = 120;
    SEARCH_ATTEMPTS = 0;
    // empezar girando a la izquierda
    left();
    return;
  }

  // comprobar si la fase actual terminó
  if (now - SEARCH_PHASE_START >= (unsigned long)SEARCH_MOVING_TIME) {
    // avanzar a siguiente fase
    SEARCH_PHASE = (SEARCH_PHASE + 1) % 3;
    SEARCH_PHASE_START = now;

    // si completamos un ciclo (left->right->forward) contamos intento
    if (SEARCH_PHASE == 0) {
      SEARCH_ATTEMPTS++;
      // aumentar tiempo de movimiento para ampliar búsqueda
      SEARCH_MOVING_TIME = min(SEARCH_MOVING_TIME + SEARCH_MOVE_INCREMENT, SEARCH_MAX_MOVE_MS);
    }

    // elegir acción para la fase entrante
    if (SEARCH_PHASE == 0) {
      // girar izquierda
      left();
    } else if (SEARCH_PHASE == 1) {
      // girar derecha
      right();
    } else {  // fase 2: avanzar un poco
      forward();
      // usar un avance más corto que el giro (ej. 200 ms)
      SEARCH_PHASE_START = now;
      SEARCH_MOVING_TIME = 200;
    }
  } else {
    // mantener la acción actual: ayuda a asegurar salidas de motor repetidas
    if (SEARCH_PHASE == 0) left();
    else if (SEARCH_PHASE == 1) right();
    else forward();
  }

  // condiciones de fallo/recuperación
  if (SEARCH_ATTEMPTS >= SEARCH_MAX_ATTEMPTS) {
    // fallback: marcha atrás corta y reset de búsqueda
    backward();
    unsigned long bt = now;
    // no bloquear: esperar virtualmente backwards durante 220 ms
    while (millis() - bt < 220) {
      // permitir otras tareas; salir si detecta línea
      if (D2_STATE || D3_STATE || D4_STATE) break;
      processTurningTasks();
      pollSensorsNonBlocking();
    }
    softStop();
    // reset de parámetros de búsqueda para intentar de nuevo
    SEARCH_ACTIVE = false;
    SEARCH_ATTEMPTS = 0;
    SEARCH_MOVING_TIME = 120;
    // quedarse en SEARCHING para reintentar
  }
}

/* ================================  THREADS? ================================ */

/**
 * @brief Muestrea sensores de línea de forma no bloqueante.
 *
 * Lee S2, S3 y S4 a intervalos definidos por SENSOR_INTERVAL y actualiza
 * las variables globales D2_STATE, D3_STATE y D4_STATE.
 * LOW en el pin se interpreta como detección -> true.
 */
void pollSensorsNonBlocking() {
  unsigned long now = millis();
  if (now - LAST_SENSOR_MILLIS < SENSOR_INTERVAL) return;
  LAST_SENSOR_MILLIS = now;

  bool raw2 = (digitalRead(S2) == LOW);
  bool raw3 = (digitalRead(S3) == LOW);
  bool raw4 = (digitalRead(S4) == LOW);

  D2_COUNT = raw2 ? min(D2_COUNT + 1, SENSOR_CONFIRM_COUNT) : 0;
  D3_COUNT = raw3 ? min(D3_COUNT + 1, SENSOR_CONFIRM_COUNT) : 0;
  D4_COUNT = raw4 ? min(D4_COUNT + 1, SENSOR_CONFIRM_COUNT) : 0;

  D2_STATE = (D2_COUNT >= SENSOR_CONFIRM_COUNT);
  D3_STATE = (D3_COUNT >= SENSOR_CONFIRM_COUNT);
  D4_STATE = (D4_COUNT >= SENSOR_CONFIRM_COUNT);

  // actualizar último instante en que la línea fue vista
  if (D2_STATE || D3_STATE || D4_STATE) {
    LAST_SEEN_LINE_MS = now;
  }
}

/**
 * @brief Actualiza el estado global de la máquina de estados sin bloquear.
 *
 * Ejecuta updateCurrentState() a intervalos definidos por STATE_INTERVAL
 * usando las últimas lecturas muestreadas por pollSensorsNonBlocking().
 */
void updateStateNonBlocking() {
  unsigned long now = millis();
  if (now - LAST_STATE_MILLIS < STATE_INTERVAL) return;
  LAST_STATE_MILLIS = now;

  // usa las variables globales ya muestreadas
  updateCurrentState(D2_STATE, D3_STATE, D4_STATE);
}

/**
 * @brief Telemetría no bloqueante para debugging sin Serial.
 *
 * NOTA: Actualmente deshabilitada para no interferir con LED_BUILTIN que indica FOLLOWING_LINE.
 * Patrón original: N blinks para estado + 3 pulsos para sensores izq/centro/der.
 * Las salidas de LED están comentadas; se mantiene la lógica para referencia.
 */
void indicateStatusNonBlocking() {
  unsigned long now = millis();

  // iniciar patrón periódicamente
  if (statusPhase == 0) {
    if (now - LAST_STATUS_TICK < STATUS_INTERVAL_MS) return;
    LAST_STATUS_TICK = now;
    // número de blinks para el estado actual
    stateBlinksRemaining = (CURRENT_STATE == STOPPED) ? 1 : (CURRENT_STATE == SEARCHING ? 2 : 3);
    stateBlinkOn = 0;
    sensorIndex = 0;
    statusPhase = 1;
    statusStepStart = now;
    // telemetry uses Serial now; avoid changing LED_BUILTIN
    // digitalWrite(LED_BUILTIN, LOW);
    return;
  }

  // administrar pasos con temporizador
  if (now - statusStepStart < STATUS_STEP_MS) return;

  statusStepStart = now;

  if (statusPhase == 1) {  // blinks de estado
    if (stateBlinksRemaining <= 0) {
      statusPhase = 2;  // pasar a gap antes de sensores
      // digitalWrite(LED_BUILTIN, LOW);
      return;
    }
    // alternar on/off
    if (!stateBlinkOn) {
      // digitalWrite(LED_BUILTIN, HIGH);
      stateBlinkOn = 1;
    } else {
      // digitalWrite(LED_BUILTIN, LOW);
      stateBlinksRemaining--;
      stateBlinkOn = 0;
    }
    return;
  }

  if (statusPhase == 2) {  // gap breve
    statusPhase = 3;
    return;
  }

  if (statusPhase == 3) {  // 3 pulses = sensores izq/centro/der
    if (sensorIndex >= 3) {
      statusPhase = 4;
      // digitalWrite(LED_BUILTIN, LOW);
      return;
    }
    bool s = (sensorIndex == 0 ? D2_STATE : (sensorIndex == 1 ? D3_STATE : D4_STATE));
    if (s) {
      // digitalWrite(LED_BUILTIN, HIGH);
    } else {
      // digitalWrite(LED_BUILTIN, LOW);
    }
    sensorIndex++;
    return;
  }

  if (statusPhase == 4) {  // final: volver a idle (LED_BUILTIN controlado por loop)
    statusPhase = 0;
    return;
  }
}

/**
 * @brief Procesa las tareas asociadas a giros no bloqueantes.
 *
 * - Asegura exclusión mutua entre giros izquierdo/derecho.
 * - Requiere que S3 (D3_STATE debounced) detecte la línea de forma estable durante
 *   CENTER_HOLD_MS antes de terminar el giro.
 * - Al finalizar (por detección o timeout TURN_MAX_MS), transiciona a forward()
 *   directamente (NO softStop) para evitar paradas intermedias.
 * - Actualiza LAST_MOVE_MS para evitar reinicios inmediatos del giro.
 */
void processTurningTasks() {
  // Seguridad: si por alguna razón ambas flags están activas -> cancelar ambas
  if (TURNING_LEFT_UNTIL_CENTER && TURNING_RIGHT_UNTIL_CENTER) {
    TURNING_LEFT_UNTIL_CENTER = false;
    TURNING_RIGHT_UNTIL_CENTER = false;
    softStop();
    return;
  }

  // Procesar giro a la izquierda si está activo
  if (TURNING_LEFT_UNTIL_CENTER) {
    // Requiere que el centro detecte de forma estable antes de terminar el giro
    if (D3_STATE) {
      if (!centerDetecting) {
        centerDetecting = true;
        centerDetectStart = millis();
      }
      // Solo termina si S3 estuvo activo >= CENTER_HOLD_MS (150 ms)
      if (millis() - centerDetectStart >= CENTER_HOLD_MS) {
        TURNING_LEFT_UNTIL_CENTER = false;
        centerDetecting = false;
        // En vez de softStop, seguimos hacia adelante para evitar paradas durante giro
        forward();
        LAST_MOVE_MS = millis();
        return;
      }
    } else {
      centerDetecting = false;
    }
    // Timeout de seguridad
    if (millis() - LEFT_START_MILLIS > TURN_MAX_MS) {
      TURNING_LEFT_UNTIL_CENTER = false;
      centerDetecting = false;
      // timeout: continuar hacia adelante en vez de parar
      forward();
      LAST_MOVE_MS = millis();
      return;
    }
    // Mantener el giro a la izquierda
    left();
    return;  // prioriza terminar el giro iniciado
  }

  // Procesar giro a la derecha si está activo
  if (TURNING_RIGHT_UNTIL_CENTER) {
    if (D3_STATE) {
      if (!centerDetecting) {
        centerDetecting = true;
        centerDetectStart = millis();
      }
      if (millis() - centerDetectStart >= CENTER_HOLD_MS) {
        TURNING_RIGHT_UNTIL_CENTER = false;
        centerDetecting = false;
        forward();
        LAST_MOVE_MS = millis();
        return;
      }
    } else {
      centerDetecting = false;
    }
    if (millis() - RIGHT_START_MILLIS > TURN_MAX_MS) {
      TURNING_RIGHT_UNTIL_CENTER = false;
      centerDetecting = false;
      forward();
      LAST_MOVE_MS = millis();
      return;
    }
    right();
  }
}

/* ================================  MOVEMENT ================================ */

/**
 * @brief Inicia un giro a la izquierda hasta que el sensor central (S3) vea la línea.
 *
 * Asegura exclusión mutua con el giro a la derecha desactivando la flag opuesta,
 * registra el instante de inicio (LEFT_START_MILLIS) y aplica la salida de motor.
 */
void startLeftUntilCenter() {
  // Si ya estamos girando a la izquierda o el centro ya detecta la línea,
  // no (re)iniciamos el giro — evita softStop/left rápidos.
  unsigned long now = millis();
  if (TURNING_LEFT_UNTIL_CENTER) return;
  if (D3_STATE) return;                            // centro ya ve la línea
  if (now - LAST_MOVE_MS < JUST_MOVED_MS) return;  // pequeña ventana tras movimiento

  // nunca permitir ambos true a la vez: desactivar derecha antes de activar izquierda
  TURNING_RIGHT_UNTIL_CENTER = false;
  TURNING_LEFT_UNTIL_CENTER = true;
  LEFT_START_MILLIS = now;
  LAST_MOVE_MS = now;
  left();  // aplicamos las salidas de motor para girar a la izquierda
}

/**
 * @brief Inicia un giro a la derecha hasta que el sensor central (S3) vea la línea.
 *
 * Asegura exclusión mutua con el giro a la izquierda desactivando la flag opuesta,
 * registra el instante de inicio (RIGHT_START_MILLIS) y aplica la salida de motor.
 */
void startRightUntilCenter() {
  // Evitar reinicios rápidos — solo iniciar si no estamos ya girando
  unsigned long now = millis();
  if (TURNING_RIGHT_UNTIL_CENTER) return;
  if (D3_STATE) return;                            // centro ya ve la línea
  if (now - LAST_MOVE_MS < JUST_MOVED_MS) return;  // pequeña ventana tras movimiento

  TURNING_LEFT_UNTIL_CENTER = false;
  TURNING_RIGHT_UNTIL_CENTER = true;
  RIGHT_START_MILLIS = now;
  LAST_MOVE_MS = now;
  right();  // aplicamos las salidas de motor para girar a la derecha
}

/**
 * @brief Mueve el robot hacia atrás a la velocidad configurada.
 *
 * Usa PWM según SPEED para invertir ambos motores.
 */
void backward() {
  Serial.println("backward");

  analogWrite(AIA, 0);
  analogWrite(AIB, SPEED);
  analogWrite(BIA, 0);
  analogWrite(BIB, SPEED);
  LAST_MOVE_MS = millis();
}

/**
 * @brief Mueve el robot hacia adelante a la velocidad configurada.
 *
 * Activa ambos motores hacia adelante con el valor SPEED.
 */
void forward() {
  Serial.println("forward");

  analogWrite(AIA, SPEED);
  analogWrite(AIB, 0);
  analogWrite(BIA, SPEED);
  analogWrite(BIB, 0);
  LAST_MOVE_MS = millis();
}

/**
 * @brief Giro en sitio hacia la izquierda.
 *
 * Un motor hacia adelante y el otro en sentido inverso para rotar.
 */
void left() {
  Serial.println("left");

  analogWrite(AIA, SPEED);
  analogWrite(AIB, 0);
  analogWrite(BIA, 0);
  analogWrite(BIB, SPEED);
  LAST_MOVE_MS = millis();
}

/**
 * @brief Giro en sitio hacia la derecha.
 *
 * Rotacion a la derecha.
 */
void right() {
  Serial.println("right");

  analogWrite(AIA, 0);
  analogWrite(AIB, SPEED);
  analogWrite(BIA, SPEED);
  analogWrite(BIB, 0);
  LAST_MOVE_MS = millis();
}

/**
 * @brief Parada suave (motores en flotación / 0).
 *
 * Desactiva las salidas para dejar los motores libres.
 */
void softStop() {
  Serial.println("soft stop");

  digitalWrite(AIA, LOW);
  digitalWrite(AIB, LOW);

  digitalWrite(BIB, LOW);
  digitalWrite(BIA, LOW);
}

/* ================================  TESTING ================================ */


// (Removed test helpers: testMovement and testLineFollowerSensor)