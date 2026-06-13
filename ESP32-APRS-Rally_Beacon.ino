#include <WiFi.h>

// =====================
// Wi-Fi Configuration
// =====================
const char* WIFI_SSID     = "BananaGuard1";
const char* WIFI_PASSWORD = "F1nnthehuman";

// =====================
// APRS Configuration
// =====================
const char* APRS_SERVER = "euro.aprs2.net";
const int   APRS_PORT   = 14580;

const char* CALLSIGN = "GB4NR";
const char* PASSCODE = "21230";

// APRS position format
// Latitude:  DDMM.mmN
// Longitude: DDDMM.mmW
const char* LATITUDE  = "5127.57N";
const char* LONGITUDE = "00118.33W";

// I corrected the URL format from https.www... to https://www...
const char* COMMENT =
  "Location of NADARS Radio Rally 23rd June - Visit https://www.nadars.org/rally.asp";

// 1 hour
const unsigned long BEACON_INTERVAL_MS = 3600UL * 1000UL;

unsigned long lastBeaconTime = 0;


// =====================
// Connect Wi-Fi
// =====================
bool connectWiFi()
{
  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }

  Serial.print("Connecting to Wi-Fi: ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long startAttempt = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 20000) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Wi-Fi connected. IP address: ");
    Serial.println(WiFi.localIP());
    return true;
  }

  Serial.println("Wi-Fi connection failed.");
  return false;
}


// =====================
// Send APRS Beacon
// =====================
bool sendAprsBeacon()
{
  if (!connectWiFi()) {
    return false;
  }

  WiFiClient client;
  client.setNoDelay(true);

  Serial.print("Connecting to APRS-IS server: ");
  Serial.print(APRS_SERVER);
  Serial.print(":");
  Serial.println(APRS_PORT);

  if (!client.connect(APRS_SERVER, APRS_PORT)) {
    Serial.println("APRS-IS connection failed.");
    return false;
  }

  // Give the server a moment to send its greeting.
  delay(1000);

  while (client.available()) {
    String line = client.readStringUntil('\n');
    line.trim();
    if (line.length() > 0) {
      Serial.print("Server: ");
      Serial.println(line);
    }
  }

  // APRS-IS login line
  String loginLine = "user ";
  loginLine += CALLSIGN;
  loginLine += " pass ";
  loginLine += PASSCODE;
  loginLine += " vers ESP32Beacon 1.0\r\n";

  Serial.print("Sending login: ");
  Serial.print(loginLine);

  client.print(loginLine);

  delay(1000);

  while (client.available()) {
    String line = client.readStringUntil('\n');
    line.trim();
    if (line.length() > 0) {
      Serial.print("Server: ");
      Serial.println(line);
    }
  }

  // APRS position packet, same idea as your Python:
  // GB4NR>APRS,TCPIP*:=5127.57N/00118.33W- Comment
  String packet = "";
  packet += CALLSIGN;
  packet += ">APRS,TCPIP*:=";
  packet += LATITUDE;
  packet += "/";
  packet += LONGITUDE;
  packet += "- ";
  packet += COMMENT;
  packet += "\r\n";

  Serial.print("Sending packet: ");
  Serial.print(packet);

  client.print(packet);
  delay(1000);

  client.stop();

  Serial.println("Beacon sent and connection closed.");
  return true;
}


// =====================
// Setup
// =====================
void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("ESP32 APRS-IS Beacon Starting");

  sendAprsBeacon();
  lastBeaconTime = millis();
}


// =====================
// Main Loop
// =====================
void loop()
{
  if (millis() - lastBeaconTime >= BEACON_INTERVAL_MS) {
    sendAprsBeacon();
    lastBeaconTime = millis();
  }

  delay(1000);
}
