#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

/* ====== CONFIG ====== */
const char* WIFI_SSID     = "Kenny Walter-2.4G";
const char* WIFI_PASSWORD = "PROFTECH";

const char* ENDPOINT = "https://njjxedmjqlkeoicckvwv.functions.supabase.co/ingest-reading";
const char* ENDPOINT_HOST = "njjxedmjqlkeoicckvwv.functions.supabase.co"; // for DNS test

const char* DEVICE_ID     = "70d605e9-677f-4484-89bb-59cf7d98ed78";
const char* DEVICE_SECRET = "aerotech-demo-key";

/* ====== TIMING ====== */
const unsigned long SEND_INTERVAL_MS = 10UL * 1000UL; // 10 seconds
unsigned long lastSend = 0;

float randFloat(float minVal, float maxVal) {
  return minVal + (float)random(0, 10001) / 10000.0f * (maxVal - minVal);
}
int randWaterCode() { return random(0, 3); }

/* --- Wait for NTP time (TLS often needs a sane clock) --- */
void syncTime() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov", "time.google.com");
  Serial.print("Syncing time");
  time_t now = time(nullptr);
  int tries = 0;
  while (now < 8 * 3600 * 2 && tries < 30) { // ~Jan 1970 means not set
    delay(500);
    Serial.print(".");
    now = time(nullptr);
    tries++;
  }
  Serial.println(now >= 8 * 3600 * 2 ? " done!" : " failed (will still try HTTPS)");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.printf("UTC time: %s", asctime(&timeinfo));
}

/* --- Debug helper for TLS last error --- */
void printTlsLastError(WiFiClientSecure& client) {
  char buf[128];
  int err = client.lastError(buf, sizeof(buf));
  if (err) {
    Serial.printf("TLS lastError(%d): %s\n", err, buf);
  } else {
    Serial.println("TLS lastError: (none reported)");
  }
}

/* --- POST helper --- */
bool postReading(float ph, float tds, float tempC, int waterCode) {
  WiFiClientSecure client;
  client.setInsecure();                 // ⚠️ dev only; replace with setCACert for production
  client.setHandshakeTimeout(15000);    // 15s TLS handshake
  client.setTimeout(15000);             // socket timeout
  // client.setBufferSizes(4096, 1024); // uncomment on older cores if needed

  HTTPClient http;
  http.setConnectTimeout(15000);        // HTTP connect timeout

  if (!http.begin(client, ENDPOINT)) {
    Serial.println("HTTP begin() failed");
    printTlsLastError(client);
    return false;
  }

  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", String("Bearer ") + DEVICE_SECRET);

  StaticJsonDocument<256> doc;
  doc["device_id"] = DEVICE_ID;
  doc["ph"] = ph;
  doc["tds"] = tds;
  doc["temp_c"] = tempC;
  doc["water_level_code"] = waterCode;

  String payload;
  serializeJson(doc, payload);

  Serial.printf("POST %s\nPayload: %s\n", ENDPOINT, payload.c_str());

  int code = http.POST(payload);
  String resp = http.getString();

  Serial.printf("HTTP %d: %s\n", code, resp.c_str());
  if (code <= 0) {
    Serial.printf("HTTP error: %s\n", http.errorToString(code).c_str());
    printTlsLastError(client);
  }

  http.end();
  client.stop();
  return (code == 200);
}

void setup() {
  Serial.begin(115200);
  delay(200);

  randomSeed(esp_random());

  Serial.printf("Connecting to WiFi \"%s\"", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print(".");
  }
  Serial.println(" connected!");
  Serial.print("IP: "); Serial.println(WiFi.localIP());

  // Quick DNS resolution check
  IPAddress ip;
  if (WiFi.hostByName(ENDPOINT_HOST, ip)) {
    Serial.print("Resolved host to: "); Serial.println(ip);
  } else {
    Serial.println("DNS resolution FAILED");
  }

  // Get time for TLS
  syncTime();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi dropped, reconnecting...");
    WiFi.reconnect();
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
      delay(300);
      Serial.print(".");
    }
    Serial.println(WiFi.status() == WL_CONNECTED ? "reconnected!" : "reconnect failed.");
  }

  unsigned long now = millis();
  if (now - lastSend >= SEND_INTERVAL_MS) {
    lastSend = now;
    float ph    = randFloat(5.5, 7.5);
    float tds   = randFloat(600, 900);
    float tempC = randFloat(20.0, 28.0);
    int water   = randWaterCode();
    postReading(ph, tds, tempC, water);
  }

  delay(50);
}
