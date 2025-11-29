/**
 * Referencias:
 * https://randomnerdtutorials.com/esp32-dc-motor-l298n-motor-driver-control-speed-direction
 * https://randomnerdtutorials.com/esp32-pinout-reference-gpios
 */

/* ============================================================= INCLUDES ============================================================= */
#include "BluetoothSerial.h"
#include <ESP32Servo.h>

/* ============================================================= CHECKS ============================================================= */
// Check if Bluetooth is available
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

/* ============================================================= STRUCTS ============================================================= */
struct L298N {
  int ENA;
  int IN1;
  int IN2;
  int ENB;
  int IN3;
  int IN4;

  void setup(int freq, int resolution, int pwmChannel) {
    pinMode(ENA, OUTPUT);
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(ENB, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);

    // configure LEDC PWM
    ledcAttachChannel(ENA, freq, resolution, pwmChannel);
    ledcAttachChannel(ENB, freq, resolution, pwmChannel);
  }

  void forward(int speed) {
    ledcWrite(ENA, speed);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    ledcWrite(ENB, speed);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
  }

  void backward(int speed) {
    ledcWrite(ENA, speed);
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    ledcWrite(ENB, speed);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
  }

  void stop() {
    digitalWrite(ENA, LOW);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(ENB, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
  }
};

/* ============================================================= VARS ============================================================= */
// ================================== Bluetooth VARS
BluetoothSerial BT;
#define BL_NAME "PUMBLEBOT"  // Bluetooth device name
char incoming = 'S';

// ================================== Serial VARS
#define SERIAL_BAUDS 115200

// ================================== MOVEMENT VARS
#define MOVEMENT_FORWARD 'F'   // 70
#define MOVEMENT_BACKWARD 'B'  // 66
#define MOVEMENT_LEFT 'L'      // 76
#define MOVEMENT_RIGHT 'R'     // 82
#define MOVEMENT_STOP 'S'      // 83
#define SERVO_DOWN 'M'         // 77
#define SERVO_UP 'N'           // 78

// ================================== L298N VARS
// H-BRIDGE 1 PINS
#define HB1_ENA 23
#define HB1_IN1 22
#define HB1_IN2 21
#define HB1_ENB 5
#define HB1_IN3 19
#define HB1_IN4 18

// H-BRIDGE 2 PINS
#define HB2_ENA 13
#define HB2_IN1 12
#define HB2_IN2 14
#define HB2_ENB 25
#define HB2_IN3 27
#define HB2_IN4 26

// SERVO PINS
#define SERVO_PIN_A 32 // Lo comecte al mismo pin pero con alimentacion aparte
#define SERVO_PIN_B 33 // FUNCIONA
#define SERVO_INITIAL_POSITION 90
#define SERVO_DOWN_POSITION 15
#define SERVO_MIDDLE_POSITION 45
#define SERVO_HIGH_POSITION 90

int speed = 255;
// Setting PWM properties
const int freq = 30000;
const int pwmChannel = 0;
const int resolution = 8;

L298N HB1 = { HB1_ENA, HB1_IN1, HB1_IN2, HB1_ENB, HB1_IN3, HB1_IN4 };
L298N HB2 = { HB2_ENA, HB2_IN1, HB2_IN2, HB2_ENB, HB2_IN3, HB2_IN4 };
Servo ServoA;
Servo ServoB;

/* ============================================================= SETUP ============================================================= */
void setup() {
  Serial.begin(SERIAL_BAUDS);
  Serial.println("Starting setup...");

  Serial.println("==> L298N setup");
  HB1.setup(freq, resolution, pwmChannel);
  HB2.setup(freq, resolution, pwmChannel);
  Serial.println("==> L298N setup completed");

  Serial.println("==> Servo setup");
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);

  ServoA.attach(SERVO_PIN_A, 500, 2400);
  ServoB.attach(SERVO_PIN_B, 500, 2400);

  ServoA.write(SERVO_INITIAL_POSITION);
  ServoB.write(SERVO_INITIAL_POSITION);
  Serial.println("==> Servo setup completed");

  Serial.println("Setup completed...");
}

/* ============================================================= LOOP ============================================================= */
void loop() {
  servoTest();
}

/* ============================================================= MOVEMENT ============================================================= */
void forward() {
  Serial.println("Moving forward");

  HB1.forward(speed);
  HB2.forward(speed);
}

void backwards() {
  Serial.println("Moving backwards");

  HB1.backward(speed);
  HB2.backward(speed);
}

void stop() {
  Serial.println("stoping");

  HB1.stop();
  HB2.stop();
}

void left() {
  Serial.println("Moving left");

  HB1.backward(200);
}

void right() {
  Serial.println("Moving right");

  HB1.forward(200);
}

/* ============================================================= SERVOS ============================================================= */


/* ============================================================= TESTING ============================================================= */
void servoTest() {
  int servo_delay = 5000;

  ServoA.write(SERVO_MIDDLE_POSITION);
  ServoB.write(SERVO_MIDDLE_POSITION);
  Serial.println("SERVO_MIDDLE_POSITION");
  delay(servo_delay);

  ServoA.write(SERVO_HIGH_POSITION);
  ServoB.write(SERVO_HIGH_POSITION);
  Serial.println("SERVO_HIGH_POSITION");
  delay(servo_delay);

  // ServoA.write(SERVO_DOWN_POSITION);
  // ServoB.write(SERVO_DOWN_POSITION);
  // Serial.println("SERVO_DOWN_POSITION");
  // delay(servo_delay);
}