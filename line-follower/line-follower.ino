// PINES DE PUENTE H
const int AIA = 6;   // PWM motor A (antes 8 no-PWM)
const int AIB = 7;   // dirección motor A (digital)
const int BIA = 10;  // PWM motor B
const int BIB = 9;   // dirección motor B (digital)
byte SPEED = 200;
int8_t LAST_TURN = 0;  // -1 = izquierda, 0 = recto, 1 = derecha

// PINES DE MODULO DE SEGUIDOR DE LINEA
// Cuando hay una deteccion, cambia a LOW, default: HIGH
const int S1 = A0;  // Extremo izquierdo
const int S2 = A5;  // Izquierda
const int S3 = A4;  // Centro
const int S4 = A3;  // Derecha
const int S5 = A2;  // Extremo derecho
// const int CLP = A1;  // Choque, no se usara
// const int NEAR = A0; // Sensor de proximidad, no se usara

enum State {
  STOPPED,
  SEARCHING,
  FOLLOWING_LINE
};
State CURRENT_STATE = STOPPED;
unsigned long WAIT_FOR_START = 3000;  // tiempo de inicio para esperar 3s antes del primer movimiento

int NO_LINE_COUNT = 0;            // contador de lecturas sin línea
const int NO_LINE_THRESHOLD = 3;  // umbral de iteraciones sin línea antes de buscar

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
  delay(WAIT_FOR_START);  // esperar antes de iniciar
  CURRENT_STATE = FOLLOWING_LINE;
}

/**
 * @brief Bucle principal. Lee los sensores y decide la maniobra.
 *
 * Lectura: 1 = blanco (sin línea), 0 = negro (línea).
 * Prioriza centro, luego correcciones leves, giros fuertes y búsqueda.
 */
void loop() {
  int s1 = digitalRead(S1);
  int s2 = digitalRead(S2);
  int s3 = digitalRead(S3);
  int s4 = digitalRead(S4);
  int s5 = digitalRead(S5);

  // convertir a detección de linea (true = negro)
  bool d1 = (s1 == 0);
  bool d2 = (s2 == 0);
  bool d3 = (s3 == 0);
  bool d4 = (s4 == 0);
  bool d5 = (s5 == 0);

  // actualizar estado
  updateCurrentState(d1, d2, d3, d4, d5);

  switch (CURRENT_STATE) {
    case FOLLOWING_LINE:
      // Prioridad: centro -> leve ajuste -> giro fuerte -> búsqueda
      if (d3 && !d2 && !d4) {
        forward();  // línea en el centro: seguir recto
        LAST_TURN = 0;
      } else if (d2 || d1) {
        // línea hacia la izquierda
        if (d2 && !d1) {
          forwardLeft();  // leve corrección izquierda
        } else {
          left();  // fuerte corrección / giro en sitio
        }
        LAST_TURN = -1;
      } else if (d4 || d5) {
        // línea hacia la derecha
        if (d4 && !d5) {
          forwardRight();  // leve corrección derecha
        } else {
          right();  // fuerte corrección / giro en sitio
        }
        LAST_TURN = 1;
      } else {
        // no detecta ahora pero aún no llegó al umbral -> mantener último giro o parar suave
        if (LAST_TURN == -1) {
          left();
        } else if (LAST_TURN == 1) {
          right();
        } else {
          softStop();
        }
      }
      break;

    case SEARCHING:
      searching();
      break;
  }

  delay(50);  // controlamos la velocidad del lazo
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

/**
 * @brief Configura los pines del módulo seguidor de línea como entradas.
 *
 * Activa los 5 pines del sensor. No retorna valor.
 */
void lineFollower() {
  pinMode(S1, INPUT);
  pinMode(S2, INPUT);
  pinMode(S3, INPUT);
  pinMode(S4, INPUT);
  pinMode(S5, INPUT);
  // pinMode(CLP, INPUT);
  // pinMode(NEAR, INPUT);

  // digitalWrite(S1, LOW);
  // digitalWrite(S2, LOW);
  // digitalWrite(S3, LOW);
  // digitalWrite(S4, LOW);
  // digitalWrite(S5, LOW);

  // configurar LED integrado para indicación de búsqueda
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
}

/* ================================  LINE FOLLOWER ================================ */

/**
 * @brief Rutina de búsqueda cuando la línea se pierde.
 *
 * Estrategia:
 *  - Alterna giros a izquierda/derecha con tiempo incremental.
 *  - Comprueba todos los sensores en cada iteración (LOW = línea).
 *  - Limita el número de intentos para evitar bloqueos indefinidos.
 *  - Avanza ligeramente al encontrar la línea para estabilizar el seguimiento.
 */
void searching() {
  Serial.println("searching...");
  CURRENT_STATE = SEARCHING;  // indicar explícitamente estado de búsqueda

  // parpadeo 3 veces antes de iniciar la búsqueda
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(150);
    digitalWrite(LED_BUILTIN, LOW);
    delay(150);
  }

  // indicador encendido durante la búsqueda
  digitalWrite(LED_BUILTIN, HIGH);

  int movingTime = 50;  // comenzar más rápido
  bool izquierda = true;
  int attempts = 0;

  while (attempts < 10) {  // límite de intentos en lugar de tiempo
    // verificar todos los sensores, no solo el centro
    if (digitalRead(S1) == LOW || digitalRead(S2) == LOW || digitalRead(S3) == LOW || digitalRead(S4) == LOW || digitalRead(S5) == LOW) {
      Serial.println("línea encontrada!");
      digitalWrite(LED_BUILTIN, LOW);
      softStop();
      LAST_TURN = 0;
      NO_LINE_COUNT = 0;               // reset contador al encontrar la línea
      CURRENT_STATE = FOLLOWING_LINE;  // volver a seguir la línea
      delay(50);
      return;
    }

    if (izquierda) {
      left();
    } else {
      right();
    }

    delay(movingTime);
    izquierda = !izquierda;
    movingTime = min(movingTime + 30, 200);  // incremento gradual
    attempts++;
  }

  // apagar indicador si no encuentra y continuar
  digitalWrite(LED_BUILTIN, LOW);
  forward();
  delay(200);

  // al terminar búsqueda volver a intentar seguir línea
  CURRENT_STATE = FOLLOWING_LINE;
}

/**
 * @brief Actualiza CURRENT_STATE según las lecturas de los sensores.
 *
 * Si cualquiera detecta línea -> FOLLOWING_LINE.
 * Si ninguna detecta línea de forma sostenida (NO_LINE_THRESHOLD) -> SEARCHING.
 */
void updateCurrentState(bool d1, bool d2, bool d3, bool d4, bool d5) {
  if (d1 || d2 || d3 || d4 || d5) {
    NO_LINE_COUNT = 0;
    CURRENT_STATE = FOLLOWING_LINE;
  } else {
    NO_LINE_COUNT++;
    if (NO_LINE_COUNT >= NO_LINE_THRESHOLD) {
      CURRENT_STATE = SEARCHING;
    }
  }
}

/* ================================  MOVEMENT ================================ */

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

/**
 * @brief Parada dura (freno activo según puente H).
 *
 * Activa ambas salidas para frenar los motores bruscamente.
 */
void hardStop() {
  Serial.println("hard stop");

  digitalWrite(AIA, HIGH);
  digitalWrite(AIB, HIGH);

  digitalWrite(BIB, HIGH);
  digitalWrite(BIA, HIGH);
}

/**
 * @brief Avanza curvando ligeramente a la izquierda.
 *
 * Reduce la velocidad del motor izquierdo para generar la curva.
 */
void forwardLeft() {
  Serial.println("forward Left");

  // reduce velocidad del motor izquierdo para curvar suavemente a la izquierda
  uint8_t slow = max(40, SPEED * 60 / 100);  // 60% de SPEED, mínimo 40
  analogWrite(AIA, slow);
  analogWrite(AIB, 0);
  analogWrite(BIA, SPEED);
  analogWrite(BIB, 0);
}

/**
 * @brief Avanza curvando ligeramente a la derecha.
 *
 * Reduce la velocidad del motor derecho para generar la curva.
 */
void forwardRight() {
  Serial.println("forward Right");

  // reduce velocidad del motor derecho para curvar suavemente a la derecha
  uint8_t slow = max(40, SPEED * 60 / 100);
  analogWrite(AIA, SPEED);
  analogWrite(AIB, 0);
  analogWrite(BIA, slow);
  analogWrite(BIB, 0);
}

/* ================================  TESTING ================================ */

/**
 * @brief Secuencia para probar movimientos básicos.
 *
 * Ejecuta una serie de comandos de movimiento con pausas para ver el comportamiento.
 */
void testMovement() {
  forward();
  delay(5000);
  softStop();
  delay(1000);

  backward();
  delay(5000);
  softStop();
  delay(1000);

  left();
  delay(5000);
  softStop();
  delay(1000);

  right();
  delay(5000);
  softStop();
  delay(1000);
}

/**
 * @brief Imprime el estado de los sensores del seguidor de línea.
 *
 * Muestra por serial el valor lógico de cada pin de sensor.
 */
void testLineFollowerSensor() {
  Serial.print("S1: " + String(digitalRead(S1)));
  Serial.print(", S2: " + String(digitalRead(S2)));
  Serial.print(", S3: " + String(digitalRead(S3)));
  Serial.print(", S4: " + String(digitalRead(S4)));
  Serial.print(", S5: " + String(digitalRead(S5)));
  // Serial.print(", CLP: " + String(digitalRead(CLP)));
  // Serial.print(", NEAR: " + String(digitalRead(NEAR)));
  Serial.println();
  delay(1000);
}