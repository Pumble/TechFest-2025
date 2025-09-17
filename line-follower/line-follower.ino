// PINES DE PUENTE H
const int AIA = 8;   // (pwm) pin 6 conectado a pin A-IA
const int AIB = 7;   // (pwm) pin 5 conectado a pin A-IB
const int BIA = 10;  // (pwm) pin 10 conectado a pin B-IA --> velodicdad
const int BIB = 9;   // (pwm) pin 9 conectado a pin B-IB  --> direccion
byte SPEED = 200;

// PINES DE MODULO DE SEGUIDOR DE LINEA
// Cuando hay una deteccion, cambia a LOW, default: HIGH
const int S1 = A0;   // Extremo izquierdo
const int S2 = A5;   // Izquierda
const int S3 = A4;   // Centro
const int S4 = A3;   // Derecha
const int S5 = A2;   // Extremo derecho
const int CLP = A1;  // Choque, no se usara
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

  if (s3 == LOW && s1 == HIGH && s2 == HIGH && s4 == HIGH && s5 == HIGH) {
    forward();  // Ir derecho
  } else if (s1 == LOW || s2 == LOW) {
    // Ajustar el tiempo que gira hacia la izquierda
    // Girar hasta que el centro detecte la linea
    left();
  } else if (s4 == LOW || s5 == LOW) {
    // Ajustar el tiempo que gira hacia la derecha
    // Girar hasta que el centro detecte la linea
    right();
  } else {
    /* Hay que buscar, girar en alguna direccion hasta 
     * que el centro detecte la linea
     */
    searching();
  }

  delay(50);
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
  pinMode(CLP, INPUT);
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

  int movingTime = 100;
  bool izquierda = true;

  while (digitalRead(S3) == HIGH && movingTime <= 1000) {
    if (izquierda) {
      left();
      delay(movingTime);
    } else {
      right();
      delay(movingTime);
    }
    izquierda = !izquierda;  // Cambia de dirección
    movingTime += 100;
  }
  forward();
  delay(100);
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
}

void testLineFollowerSensor() {
  Serial.print("S1: " + String(digitalRead(S1)));
  Serial.print(", S2: " + String(digitalRead(S2)));
  Serial.print(", S3: " + String(digitalRead(S3)));
  Serial.print(", S4: " + String(digitalRead(S4)));
  Serial.print(", S5: " + String(digitalRead(S5)));
  Serial.print(", CLP: " + String(digitalRead(CLP)));
  // Serial.print(", NEAR: " + String(digitalRead(NEAR)));
  Serial.println();
  delay(1000);
}