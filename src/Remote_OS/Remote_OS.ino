#include <WiFi.h>

#define BUTTON_UP 15
#define BUTTON_DOWN 4
#define BUTTON_STOP 0

#define LED_PIN 23

volatile char toSend = '\0';
WiFiClient client;

// WiFi params
IPAddress ip(192, 168, 1, 71);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);
const char* ssid     = "<wifi ssid>";
const char* password = "<password>";

// This will probably be useful later
unsigned long timeout;

void buttonUp() {
  if (digitalRead(BUTTON_UP) == LOW) {
    toSend = 'a';
  } else {
    toSend = 'b';
  }
}

void buttonDown() {
  if (digitalRead(BUTTON_DOWN) == LOW) {
    toSend = 'c';
  } else {
    toSend = 'b';
  }
}

void stopPressed() {
  toSend = 'd';
}

void setupGPIO() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
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

void loop()
{
  if (toSend != '\0') {
    if (!client.connected()) {
      timeout = millis();
      while (!client.connect("192.168.1.70", 7000)) {
        delay(1000);
        if (millis() - timeout > 60000) {
          errorLoop(1000);
        }
      }
    }

    client.print(toSend);
    toSend = '\0';
  }
}
