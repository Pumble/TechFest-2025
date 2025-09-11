// PINES DE PUENTE H
const int AIA = 8;   // (pwm) pin 6 conectado a pin A-IA
const int AIB = 7;   // (pwm) pin 5 conectado a pin A-IB
const int BIA = 10;  // (pwm) pin 10 conectado a pin B-IA --> velodicdad
const int BIB = 9;   // (pwm) pin 9 conectado a pin B-IB  --> direccion
byte SPEED = 200;

// PINES DE MODULO DE SEGUIDOR DE LINEA
const int S1 = A7;
const int S2 = A5;
const int S3 = A4;
const int S4 = A3;
const int S5 = A2;
const int CLP = A1;
const int NEAR = A0;

void setup() {
  hBridge();
  serial();
}

void loop() {
  // forward();
  // delay(5000);
  // softStop();
  // delay(1000);
  // backward();
  // delay(5000);
  // hardStop();
  // delay(1000);
  // left();
  // delay(5000);
  // right();
  // delay(5000);

  Serial.print("S1: " + String(digitalRead(S1)));
  Serial.print(", S2: " + String(digitalRead(S2)));
  Serial.print(", S3: " + String(digitalRead(S3)));
  Serial.print(", S4: " + String(digitalRead(S4)));
  Serial.print(", S5: " + String(digitalRead(S5)));
  Serial.print(", CLP: " + String(digitalRead(CLP)));
  Serial.print(", NEAR: " + String(digitalRead(NEAR)));
  Serial.println();
  delay(1000);
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
  pinMode(NEAR, INPUT);
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