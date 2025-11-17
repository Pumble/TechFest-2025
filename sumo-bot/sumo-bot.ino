/**
 * Referencias:
 * https://randomnerdtutorials.com/esp32-dc-motor-l298n-motor-driver-control-speed-direction
 * https://randomnerdtutorials.com/esp32-pinout-reference-gpios
 */

/* ============================================================= INCLUDES ============================================================= */
#include "BluetoothSerial.h"

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

int speed = 255;
// Setting PWM properties
const int freq = 30000;
const int pwmChannel = 0;
const int resolution = 8;

L298N HB1 = { HB1_ENA, HB1_IN1, HB1_IN2, HB1_ENB, HB1_IN3, HB1_IN4 };

/* ============================================================= SETUP ============================================================= */
void setup() {
  Serial.begin(SERIAL_BAUDS);
  Serial.println("Starting setup...");

  Serial.println("==> L298N setup");
  HB1.setup(freq, resolution, pwmChannel);
  Serial.println("==> L298N setup completed");

  Serial.println("Setup completed...");
}

/* ============================================================= LOOP ============================================================= */
void loop() {
  stop();
  delay(1000);

  forward();
  delay(5000);

  stop();
  delay(1000);

  backwards();
  delay(5000);
}

/* ============================================================= MOVEMENT ============================================================= */
void forward() {
  Serial.println("Moving forward");

  HB1.forward(speed);
}

void backwards() {
  Serial.println("Moving backwards");

  HB1.backward(speed);
}

void stop() {
  Serial.println("stoping");

  // H-BRIDGE 1
  HB1.stop();
}

void left() {
  Serial.println("Moving left");

  HB1.backward(200);
}

void right() {
  Serial.println("Moving right");

  HB1.forward(200);
}