
const int AIA = 8;   // (pwm) pin 6 conectado a pin A-IA
const int AIB = 7;   // (pwm) pin 5 conectado a pin A-IB
const int BIA = 10;  // (pwm) pin 10 conectado a pin B-IA --> velodicdad
const int BIB = 9;   // (pwm) pin 9 conectado a pin B-IB  --> direccion

byte SPEED = 200;

void setup() {
  hBridge();
  serial();
}

void loop() {
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