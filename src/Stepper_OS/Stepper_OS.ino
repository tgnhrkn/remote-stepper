#include <WiFi.h>
#include <WiFiClient.h>

// Eqn = 60000 (ms/min) / RPM / (200 (full step per rev) * microsteps per full step) / 2 (high edge triggers)
#define MS_DELAY 1.875

#define STEP_PIN 12
#define EN_PIN 13
#define DIR_PIN 14
#define LED_PIN 23
#define FAN_PIN 25

#define BUTTON_UP 15
#define BUTTON_DOWN 4
#define BUTTON_STOP 0

// Button state
int motorState = 0; // 0 = rest, 1 = up, 2 = down
bool halt = false;

// WiFi params
IPAddress ip(192, 168, 1, 70);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);
const char* ssid     = "TheWifi";
const char* password = "ThePassword";
WiFiServer server(7000);

// This will probably be useful later
// unsigned long timeout = millis();

void buttonUp() {
  if (digitalRead(BUTTON_UP) == LOW) {
    motorState = 1;
  } else {
    motorState = 0;
  }
}

void buttonDown() {
  if (digitalRead(BUTTON_DOWN) == LOW) {
    motorState = 2;
  } else {
    motorState = 0;
  }
}

void stopPressed() {
  digitalWrite(EN_PIN, HIGH);
  motorState = 0;
  halt = true;
}

void setupGPIO() {
  pinMode(STEP_PIN, OUTPUT);
  pinMode(EN_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);

  digitalWrite(EN_PIN, HIGH);
  digitalWrite(FAN_PIN, LOW);
}

void setupInterrupts() {
  pinMode(BUTTON_UP, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_UP), buttonUp, CHANGE);  // attach interrupt handler

  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_DOWN), buttonDown, CHANGE);  // attach interrupt handler

  pinMode(BUTTON_STOP, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_STOP), stopPressed, CHANGE);  // attach interrupt handler
}

void setupWiFi() {
  WiFi.useStaticBuffers(true);
  WiFi.setAutoReconnect(true);
  if (!WiFi.config(ip, gateway, subnet)) {
    errorLoop(500);
  }
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  server.begin();
}

void setup() {
  setupGPIO();
  setupInterrupts();
  setupWiFi();
}

void errorLoop(float ms) {
  while (1) {
    digitalWrite(LED_PIN, HIGH);
    delay(ms);
    digitalWrite(LED_PIN, LOW);
    delay(ms);
  }
}

void checkHalt() {
  if (halt) {
    errorLoop(1000);
  }
}

void runMotor() {
  checkHalt();

  bool go = false;
  if (motorState == 1) {
    digitalWrite(DIR_PIN, HIGH);
    go = true;
  } else if (motorState == 2) {
    digitalWrite(DIR_PIN, LOW);
    go = true;
  }

  if (go) {
    digitalWrite(EN_PIN, LOW);
    digitalWrite(FAN_PIN, HIGH);
    digitalWrite(STEP_PIN, HIGH);
    delay(MS_DELAY);
    digitalWrite(STEP_PIN, LOW);
    delay(MS_DELAY);
  } else {
    digitalWrite(FAN_PIN, LOW);
    digitalWrite(EN_PIN, HIGH);
  }
}

void handleClientChar(char c) {
  if (c == 'a') {
    motorState = 1;
  } else if (c == 'b') {
    motorState = 0;
  } else if (c == 'c') {
    motorState = 2;
  } else if (c == 'd') {
    stopPressed();
  }
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        handleClientChar(c);
      }
      runMotor();
    }
    client.stop();
    // if the client disconnects for some reason, stop the motor in case they disconnected in the middle of some commands
    motorState = 0;
  } else {
    runMotor();
  }
}
