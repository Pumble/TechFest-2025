// PINES DE PUENTE H
const int AIA = 8;   // (pwm) pin 6 conectado a pin A-IA
const int AIB = 7;   // (pwm) pin 5 conectado a pin A-IB
const int BIA = 10;  // (pwm) pin 10 conectado a pin B-IA --> velodicdad
const int BIB = 9;   // (pwm) pin 9 conectado a pin B-IB  --> direccion
byte SPEED = 200;
int8_t lastTurn = 0;  // -1 = izquierda, 0 = recto, 1 = derecha

// PINES DE MODULO DE SEGUIDOR DE LINEA
// Cuando hay una deteccion, cambia a LOW, default: HIGH
const int S1 = A0;  // Extremo izquierdo
const int S2 = A5;  // Izquierda
const int S3 = A4;  // Centro
const int S4 = A3;  // Derecha
const int S5 = A2;  // Extremo derecho
// const int CLP = A1;  // Choque, no se usara
// const int NEAR = A0;

void setup() {
  hBridge();
  serial();
  lineFollower();
}

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

  // Prioridad: centro -> leve ajuste -> giro fuerte -> búsqueda
  if (d3 && !d2 && !d4) {
    forward();  // línea en el centro: seguir recto
    lastTurn = 0;
  } else if (d2 || d1) {
    // línea hacia la izquierda
    if (d2 && !d1) {
      forwardLeft();  // leve corrección izquierda
    } else {
      left();  // fuerte corrección / giro en sitio
    }
    lastTurn = -1;
  } else if (d4 || d5) {
    // línea hacia la derecha
    if (d4 && !d5) {
      forwardRight();  // leve corrección derecha
    } else {
      right();  // fuerte corrección / giro en sitio
    }
    lastTurn = 1;
  } else {
    // ninguna detección: intentar mantener la dirección previa o buscar
    if (lastTurn == -1) {
      left();
    } else if (lastTurn == 1) {
      right();
    } else {
      searching();
    }
  }

  delay(30);  // controlamos la velocidad del lazo
}

/* ================================  SETUPS ================================ */
void hBridge() {
  pinMode(AIA, OUTPUT);  // fijar los pines como salidas
  pinMode(AIB, OUTPUT);
  pinMode(BIA, OUTPUT);
  pinMode(BIB, OUTPUT);
}

void serial() {
  Serial.begin(9600);  // Inicia la comunicación a 9600 baudios
  Serial.println("Hola, consola serial!");
}

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
}

/* ================================  LINE FOLLOWER ================================ */
void searching() {
  Serial.println("searching...");

  int movingTime = 50;  // comenzar más rápido
  bool izquierda = true;
  int maxTime = 1500;  // aumentar tiempo de búsqueda
  int attempts = 0;

  while (attempts < 10) {  // límite de intentos en lugar de tiempo
    // verificar todos los sensores, no solo el centro
    if (digitalRead(S1) == LOW || digitalRead(S2) == LOW || digitalRead(S3) == LOW || digitalRead(S4) == LOW || digitalRead(S5) == LOW) {
      Serial.println("línea encontrada!");
      forward();
      delay(150);  // avanzar un poco para estabilizar
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

  // si no encuentra, ir recto e intentar de nuevo
  forward();
  delay(200);
}

/* ================================  MOVEMENT ================================ */
void backward() {
  Serial.println("backward");

  analogWrite(AIA, 0);
  analogWrite(AIB, SPEED);
  analogWrite(BIA, 0);
  analogWrite(BIB, SPEED);
}

void forward() {
  Serial.println("forward");

  analogWrite(AIA, SPEED);
  analogWrite(AIB, 0);
  analogWrite(BIA, SPEED);
  analogWrite(BIB, 0);
}

void left() {
  Serial.println("left");

  analogWrite(AIA, SPEED);
  analogWrite(AIB, 0);
  analogWrite(BIA, 0);
  analogWrite(BIB, SPEED);
}

void right() {
  Serial.println("right");

  analogWrite(AIA, 0);
  analogWrite(AIB, SPEED);
  analogWrite(BIA, SPEED);
  analogWrite(BIB, 0);
}

void softStop() {
  Serial.println("soft stop");

  digitalWrite(AIA, LOW);
  digitalWrite(AIB, LOW);

  digitalWrite(BIB, LOW);
  digitalWrite(BIA, LOW);
}

void hardStop() {
  Serial.println("hard stop");

  digitalWrite(AIA, HIGH);
  digitalWrite(AIB, HIGH);

  digitalWrite(BIB, HIGH);
  digitalWrite(BIA, HIGH);
}

// Movimientos suaves: avanzar con corrección hacia izquierda/derecha
void forwardLeft() {
  Serial.println("forward Left");

  // reduce velocidad del motor izquierdo para curvar suavemente a la izquierda
  uint8_t slow = max(40, SPEED * 60 / 100);  // 60% de SPEED, mínimo 40
  analogWrite(AIA, slow);
  analogWrite(AIB, 0);
  analogWrite(BIA, SPEED);
  analogWrite(BIB, 0);
}

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
void testMovement() {
  forward();
  delay(5000);
  softStop();
  delay(1000);
  backward();
  delay(5000);
  hardStop();
  delay(1000);
  left();
  delay(5000);
  right();
  delay(5000);
  softStop();
  delay(1000);
}

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