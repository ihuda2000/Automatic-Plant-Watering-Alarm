#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

#include "secrets.h" // create from secrets.h.sample

// -------- Pins --------
const uint8_t PIN_BUZZER = D5;   // GPIO14
const uint8_t PIN_LED    = LED_BUILTIN; // D4 / GPIO2 (active LOW on many boards)
const uint8_t PIN_MOIST  = A0;

// -------- Config (EEPROM) --------
uint16_t moistureThreshold = 650; // higher = drier (typical for capacitive sensors)
uint8_t  alarmEnabled      = 1;

const size_t EEPROM_SIZE = 16;

void loadConfig() {
  EEPROM.begin(EEPROM_SIZE);
  uint16_t th;
  uint8_t en;
  EEPROM.get(0, th);
  EEPROM.get(2, en);

  // first-run detection (uninitialized EEPROM often 0xFF)
  if (th == 0xFFFF || th == 0) th = 650;
  if (en != 0 && en != 1)      en = 1;

  moistureThreshold = th;
  alarmEnabled = en;
}

void saveConfig() {
  EEPROM.put(0, moistureThreshold);
  EEPROM.put(2, alarmEnabled);
  EEPROM.commit();
}

// -------- WiFi / HTTP --------
ESP8266WebServer server(80);

String htmlPage(uint16_t value, uint16_t threshold, bool enabled) {
  String s;
  s += F("<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width,initial-scale=1'>");
  s += F("<title>Plant Watering Alarm</title>");
  s += F("<style>body{font-family:system-ui;margin:20px;max-width:680px} .card{border:1px solid #ddd;padding:16px;border-radius:8px;margin:10px 0} input[type=number]{width:120px}</style>");
  s += F("</head><body><h2>Automatic Plant Watering Alarm</h2>");

  s += F("<div class='card'><h3>Status</h3>");
  s += F("<p><b>Moisture (A0): </b>");
  s += String(value);
  s += F("</p><p><b>Threshold: </b>");
  s += String(threshold);
  s += F("</p><p><b>Alarm: </b>");
  s += (enabled ? "Enabled" : "Disabled");
  s += F("</p></div>");

  s += F("<div class='card'><h3>Configure</h3>"
         "<form action='/set' method='POST'>"
         "Threshold (0-1023): "
         "<input type='number' name='threshold' min='0' max='1023' value='");
  s += String(threshold);
  s += F("'>"
         "<br><label><input type='checkbox' name='enabled' ");
  if (enabled) s += F("checked");
  s += F("> Enable alarm</label>"
         "<br><button type='submit'>Save</button></form>"
         "<form action='/beep' method='POST' style='margin-top:10px'>"
         "<button>Beep Test</button></form>"
         "</div>");

  s += F("<p style='color:#666'>Tip: Many capacitive sensors read lower when wet and higher when dry. "
         "Raise the threshold if you want the alarm to trigger earlier (drier soil).</p>");

  s += F("</body></html>");
  return s;
}

void handleRoot() {
  uint16_t val = analogRead(PIN_MOIST);
  server.send(200, "text/html", htmlPage(val, moistureThreshold, alarmEnabled));
}

void handleSet() {
  if (server.method() != HTTP_POST) { server.send(405, "text/plain", "Method Not Allowed"); return; }
  if (server.hasArg("threshold")) {
    long th = server.arg("threshold").toInt();
    th = constrain(th, 0, 1023);
    moistureThreshold = (uint16_t)th;
  }
  alarmEnabled = server.hasArg("enabled") ? 1 : 0;
  saveConfig();
  server.sendHeader("Location", "/");
  server.send(303); // redirect
}

void beep(uint16_t freq, uint16_t ms) {
  tone(PIN_BUZZER, freq, ms);
  // LED indicator
  digitalWrite(PIN_LED, LOW);
  delay(ms);
  digitalWrite(PIN_LED, HIGH);
}

void handleBeep() {
  if (server.method() != HTTP_POST) { server.send(405, "text/plain", "Method Not Allowed"); return; }
  beep(2000, 250);
  server.sendHeader("Location", "/");
  server.send(303);
}

void setupWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Connected: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi connect failed; starting AP mode");
    WiFi.mode(WIFI_AP);
    WiFi.softAP("PlantAlarm-Setup");
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH);

  loadConfig();
  setupWiFi();

  server.on("/", HTTP_GET, handleRoot);
  server.on("/set", HTTP_POST, handleSet);
  server.on("/beep", HTTP_POST, handleBeep);
  server.begin();
  Serial.println("HTTP server started");
}

uint32_t lastRead = 0;
const uint32_t READ_INTERVAL_MS = 2000;

void loop() {
  server.handleClient();

  if (millis() - lastRead >= READ_INTERVAL_MS) {
    lastRead = millis();
    uint16_t val = analogRead(PIN_MOIST);
    Serial.printf("Moisture=%u  (threshold=%u, alarm=%u)\n", val, moistureThreshold, alarmEnabled);

    // Alarm if dry enough
    if (alarmEnabled && val >= moistureThreshold) {
      // chirp pattern
      for (int i = 0; i < 2; ++i) { beep(1800, 120); delay(120); }
    }
  }
}

