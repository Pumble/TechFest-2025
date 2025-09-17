/* Referencias:
 * Codigo Funduino: https://codebender.cc/sketch:146219#Funduino%20Joystick%20Shield%20Example.ino
 * Tienda Funduino: https://www.crcibernetica.com/funduino-joystick-shield/
 *
 * BLUETOOTH HC-05
 * https://www.whizzbizz.com/en/bluetooth.joystick.shield
 *
 */

// ============================================================= INCLUDES =============================================================
#include <SoftwareSerial.h>

// ============================================================= VARS =============================================================

// CONTROL VARS
int TRIANGLE_BUTTON = 2;
int CROSS_BUTTON = 4;
int SQUARE_BUTTON = 5;
int CIRCLE_BUTTON = 3;
int START_BUTTON = 6;
int SELECT_BUTTON = 7;
int JOYSTICK_BUTTON = 8;
int JOYSTICK_AXIS_X = A0;  // MIN: 0, CENTER: 523, MAX: 1023
int JOYSTICK_AXIS_Y = A1;  // MIN: 0, CENTER: 506, MAX: 1023
int BUTTONS[] = { TRIANGLE_BUTTON, CROSS_BUTTON, SQUARE_BUTTON, CIRCLE_BUTTON, START_BUTTON, SELECT_BUTTON, JOYSTICK_BUTTON };

// SERIAL VARS
int BAUDS = 9600;

// BLUETOOTH VARS
#define BT_RX 10
#define BT_TX 11
String BT_DEVICE_NAME = "CTRL_FUNDUINO_JUAN";
String BT_DEVICE_PIN = "8462";
SoftwareSerial BT(BT_RX, BT_TX);  // Definimos los pines RX y TX del Arduino conectados al Bluetooth

// ============================================================= SETUP =============================================================
void setup() {
  buttons_setup();
  bluetooth_setup();
  Serial.begin(BAUDS);
  Serial.println("Starting funduino controller!");
}

void buttons_setup() {
  for (int i; i < 7; i++) {
    pinMode(BUTTONS[i], INPUT);
    digitalWrite(BUTTONS[i], HIGH);
  }
}

void bluetooth_setup() {
  BT.begin(38400);  // Inicializamos el puerto serie BT (Para Modo AT 2)

  BT.print("AT");       // Inicializa comando AT
  BT.print("AT+ORGL");  // Restaura el dispositivo de fabrica
  delay(1000);
  BT.print("AT+RESET");  // Reset al dispositivo
  delay(1000);

  BT.println("AT+ROLE=1");  // Configura el módulo como maestro
  delay(1000);
  BT.print("AT+NAME=" + BT_DEVICE_NAME);  // Cambia el nombre del dispositivo
  delay(1000);
  BT.print("AT+PSWD=" + BT_DEVICE_PIN);  // Cambia el pin del dispositivo

  // BT.println("AT+CMODE=0"); // Conectar a una dirección específica
  // BT.println("AT+BIND=0011,22,334455"); // Dirección MAC del dispositivo esclavo

  // getBluetoothData();
}

// ============================================================= LOOP =============================================================
void loop() {
  int X = map(analogRead(JOYSTICK_AXIS_X), 0, 1000, -1, 1);
  int Y = map(analogRead(JOYSTICK_AXIS_Y), 0, 1000, -1, 1);
  int K = digitalRead(JOYSTICK_BUTTON);

  int START = digitalRead(START_BUTTON);
  int SELECT = digitalRead(SELECT_BUTTON);

  int SQUARE = digitalRead(SQUARE_BUTTON);
  int TRIANGLE = digitalRead(TRIANGLE_BUTTON);
  int CIRCLE = digitalRead(CIRCLE_BUTTON);
  int CROSS = digitalRead(CROSS_BUTTON);

  BT.print(
    String(X) + "," + String(Y) + "," + String(K) + "," +

    String(START) + "," + String(SELECT) + "," +

    String(SQUARE) + "," + String(TRIANGLE) + "," + String(CIRCLE) + "," + String(CROSS));
  Serial.println(
    String(X) + "," + String(Y) + "," + String(K) + "," +

    String(START) + "," + String(SELECT) + "," +

    String(SQUARE) + "," + String(TRIANGLE) + "," + String(CIRCLE) + "," + String(CROSS));
  delay(1000);
}

// ============================================================= PRUEBAS =============================================================

void bluetooth() {
  Serial.println();
  String commands[] = { "AT+VERSION", "AT+STATE", "AT+ROLE", "AT+NAME", "AT+PSWD" };
  for (int i = 0; i < 5; i++) {
    BT.print(commands[i]);
    delay(200);

    Serial.print(commands[i]);
    Serial.print(": ");
    while (BT.available()) {
      Serial.write(BT.read());
    }
    Serial.println();
    delay(300);
  }
  delay(3000);

  // Serial.println();
  // Serial.print("VERSION: ");
  // Serial.println(BT.print("AT+VERSION"));
  // Serial.print("STATE: ");
  // Serial.println(BT.print("AT+STATE"));
  // Serial.print("ROLE: ");
  // Serial.println(BT.print("AT+ROLE"));
  // Serial.print("NAME: ");
  // Serial.println(BT.print("AT+NAME"));
  // Serial.print("PASSWORD: ");
  // Serial.println(BT.print("AT+PSWD"));
  // delay(1000);  // espera 1 segundo
}

void buttons() {
  // Al presionar un boton, pasa a 0, por defecto, todos son 1
  // Los ejes varian entre -1 y 1
  Serial.print("X: "), Serial.print(map(analogRead(JOYSTICK_AXIS_X), 0, 1000, -1, 1));
  Serial.print(", Y: "), Serial.print(map(analogRead(JOYSTICK_AXIS_Y), 0, 1000, -1, 1));
  Serial.print(", K: "), Serial.print(digitalRead(JOYSTICK_BUTTON));

  Serial.print(", START: "), Serial.print(digitalRead(START_BUTTON));
  Serial.print(", SELECT: "), Serial.print(digitalRead(SELECT_BUTTON));

  Serial.print(", \xE2\x96\xA1: "), Serial.print(digitalRead(SQUARE_BUTTON));
  Serial.print(", \xE2\x96\xB3: "), Serial.print(digitalRead(TRIANGLE_BUTTON));
  Serial.print(",	\xE2\x97\x8B: "), Serial.print(digitalRead(CIRCLE_BUTTON));
  Serial.print(", x: "), Serial.print(digitalRead(CROSS_BUTTON));
  Serial.println();
  delay(500);
}
