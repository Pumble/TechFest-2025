#include <ESP32Servo.h>

#define SERVO_LEFT_PIN 2
#define SERVO_RIGHT_PIN 4
Servo leftServo;
Servo rightServo;
#define SERVO_DOWN 'M'  // 77
#define SERVO_UP 'N'    // 78
#define SERVO_INITIAL_POSITION 90
#define SERVO_DOWN_POSITION 15
#define SERVO_MIDDLE_POSITION 45
#define SERVO_HIGH_POSITION 90

void setup() {
  Serial.begin(115200);

  leftServo.attach(SERVO_LEFT_PIN);    // attaches the servo on pin SERVO_LEFT_PIN to the servo object
  rightServo.attach(SERVO_RIGHT_PIN);  // attaches the servo on pin SERVO_RIGHT_PIN to the servo object

  leftServo.write(SERVO_INITIAL_POSITION);
  rightServo.write(SERVO_INITIAL_POSITION);

  Serial.println("servos configured");
}

void loop() {
  if (Serial.available() > 0) {
    int in = Serial.read();  // lee un byte
    if (in == -1) return;
    char c = (char)in;

    // Ignora retorno de carro / salto de línea
    if (c == '\n' || c == '\r') return;

    Serial.print("Recibido char: ");
    Serial.println(c);

    processCommand(c);
  }
}

void processCommand(int input) {
  switch (input) {
    case SERVO_DOWN:
      leftServo.write(SERVO_DOWN_POSITION);
      rightServo.write(SERVO_DOWN_POSITION);
      break;
    case SERVO_UP:
      leftServo.write(SERVO_HIGH_POSITION);
      rightServo.write(SERVO_HIGH_POSITION);
      break;
  }
}