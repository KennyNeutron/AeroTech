/*
  AeroTech_ActuatorClient.ino
  ESP32 client that reads actuator_state from Supabase REST and drives
  two relays (Pump, Fan). Includes two physical push buttons that toggle
  the corresponding relay when that actuator is in MANUAL mode.
  This version also WRITES BACK to Supabase when buttons toggle states.

  Tables used:
    actuator_state:
      device_id (uuid), pump_on (bool), fan_on (bool),
      mode_pump (text: "auto"|"manual"), mode_fan (text: "auto"|"manual")

  Notes:
  - Uses HTTPS with setInsecure() for simplicity. For production, pin a root cert.
  - RLS: a valid anon key will return 200 with [] for reads if no rows match.
    For writes, you need an update policy; see note below.
*/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// ===================== USER CONFIG FROM YOUR SKETCH =====================
// Keep these exactly as you have them in your file.
#define WIFI_SSID       "Kenny Walter-2.4G"
#define WIFI_PASSWORD   "PROFTECH"

// Supabase project ref (slug) and anon key
#define SUPABASE_REF    "njjxedmjqlkeoicckvwv"
const char* SUPABASE_ANON_KEY = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6Im5qanhlZG1qcWxrZW9pY2Nrdnd2Iiwicm9sZSI6ImFub24iLCJpYXQiOjE3NjA2Njk0MTIsImV4cCI6MjA3NjI0NTQxMn0.xZTAE29TDWj8O_CskPCL4qruAIIow_HM5eE6Dwwf-QM";
// The devices.id row for this hardware
const char* DEVICE_ID = "70d605e9-677f-4484-89bb-59cf7d98ed78";

// Relay pins
const int PUMP_PIN = 26;
const int FAN_PIN  = 27;

// Push buttons (active LOW with internal pull-ups)
const int PUMP_BTN = 14;
const int FAN_BTN  = 12;

// Relay logic: set to true if HIGH energizes relay, false if LOW energizes
const bool RELAY_ACTIVE_HIGH = false;

// Poll interval (ms) for pulling actuator_state
const uint32_t POLL_INTERVAL_MS = 5000;
// =======================================================================


// REST base once built from project ref
const String REST_BASE = String("https://") + SUPABASE_REF + ".supabase.co/rest/v1";

// Cloud state we mirror locally
struct ActuatorState {
  bool pump_on = false;
  bool fan_on  = false;
  String mode_pump = "auto"; // "auto"|"manual"
  String mode_fan  = "auto"; // "auto"|"manual"
} cloud;

// Debounce
unsigned long lastPumpBtnTime = 0;
unsigned long lastFanBtnTime  = 0;
const unsigned long DEBOUNCE_MS = 50;

// What we last drove to the pins, to avoid chatter
bool pumpApplied = false;
bool fanApplied  = false;


// ------------------------ Utilities ------------------------
inline void relayWrite(int pin, bool on) {
  if (RELAY_ACTIVE_HIGH) {
    digitalWrite(pin, on ? HIGH : LOW);
  } else {
    digitalWrite(pin, on ? LOW : HIGH);
  }
}

void setPump(bool on) { relayWrite(PUMP_PIN, on); pumpApplied = on; }
void setFan(bool on)  { relayWrite(FAN_PIN,  on); fanApplied  = on; }

void ensureWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.printf("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.printf("\nConnected! IP: %s\n", WiFi.localIP().toString().c_str());
}

// ------------------------ HTTP helpers ------------------------
bool httpGetJson(const String& pathAndQuery, DynamicJsonDocument& doc) {
  ensureWiFi();

  WiFiClientSecure tls;
  tls.setInsecure(); // pin a cert for production

  HTTPClient http;
  const String url = REST_BASE + pathAndQuery;
  if (!http.begin(tls, url)) {
    Serial.println("HTTP begin() failed");
    return false;
  }
  http.addHeader("apikey", SUPABASE_ANON_KEY);
  http.addHeader("Authorization", String("Bearer ") + SUPABASE_ANON_KEY);
  http.addHeader("Accept", "application/json");

  int code = http.GET();
  String body = http.getString();

  Serial.printf("HTTP GET %s -> %d\n", url.c_str(), code);
  if (code != 200) {
    Serial.println(body);
    http.end();
    return false;
  }

  auto err = deserializeJson(doc, body);
  http.end();

  if (err) {
    Serial.printf("JSON parse error: %s\n", err.c_str());
    // Serial.println(body);
    return false;
  }
  return true;
}

/*
  PATCH helper:
  Sends JSON to /rest/v1/actuator_state?device_id=eq.<uuid>
  Example payload for pump: {"mode_pump":"manual","pump_on":true}
  Example payload for fan : {"mode_fan":"manual","fan_on":false}
*/
bool httpPatchJson(const String& pathAndQuery, const String& jsonBody, String* outBody = nullptr) {
  ensureWiFi();

  WiFiClientSecure tls;
  tls.setInsecure();

  HTTPClient http;
  const String url = REST_BASE + pathAndQuery;
  if (!http.begin(tls, url)) {
    Serial.println("HTTP begin() failed (PATCH)");
    return false;
  }

  http.addHeader("apikey", SUPABASE_ANON_KEY);
  http.addHeader("Authorization", String("Bearer ") + SUPABASE_ANON_KEY);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Prefer", "return=representation"); // return updated row

  int code = http.sendRequest("PATCH", (uint8_t*)jsonBody.c_str(), jsonBody.length());
  String body = http.getString();

  Serial.printf("HTTP PATCH %s -> %d\n", url.c_str(), code);
  if (outBody) *outBody = body;

  if (code < 200 || code >= 300) {
    Serial.println(body);
    http.end();
    return false;
  }

  http.end();
  return true;
}


// ------------------------ Supabase Pulls ------------------------
void fetchActuatorState() {
  String path = String("/actuator_state")
              + "?device_id=eq." + DEVICE_ID
              + "&select=pump_on,fan_on,mode_pump,mode_fan";

  DynamicJsonDocument doc(2048);
  if (!httpGetJson(path, doc)) {
    Serial.println("GET actuator_state failed");
    return;
  }

  if (!doc.is<JsonArray>()) {
    Serial.println("Unexpected JSON shape (not array).");
    return;
  }
  if (doc.as<JsonArray>().size() == 0) {
    Serial.println("No actuator_state row for this device.");
    return;
  }

  JsonObject row = doc.as<JsonArray>()[0];

  if (!row.isNull()) {
    if (!row["pump_on"].isNull())  cloud.pump_on  = row["pump_on"].as<bool>();
    if (!row["fan_on"].isNull())   cloud.fan_on   = row["fan_on"].as<bool>();
    if (!row["mode_pump"].isNull()) cloud.mode_pump = row["mode_pump"].as<const char*>();
    if (!row["mode_fan"].isNull())  cloud.mode_fan  = row["mode_fan"].as<const char*>();
  }

  Serial.printf("Fetched: pump_on=%d fan_on=%d mode_pump=%s mode_fan=%s\n",
    cloud.pump_on, cloud.fan_on, cloud.mode_pump.c_str(), cloud.mode_fan.c_str());
}


// ------------------------ Write-back on manual buttons ------------------------
bool syncManualChange(const char* actuator, bool on) {
  // Build the JSON body and query filter
  String path = String("/actuator_state") + "?device_id=eq." + DEVICE_ID;
  String body;

  if (strcmp(actuator, "pump") == 0) {
    // Manual mode should already be enforced by the button handler
    StaticJsonDocument<256> doc;
    doc["mode_pump"] = "manual";
    doc["pump_on"] = on;
    serializeJson(doc, body);
  } else if (strcmp(actuator, "fan") == 0) {
    StaticJsonDocument<256> doc;
    doc["mode_fan"] = "manual";
    doc["fan_on"] = on;
    serializeJson(doc, body);
  } else {
    return false;
  }

  String resp;
  bool ok = httpPatchJson(path, body, &resp);
  if (!ok) {
    Serial.println("PATCH failed");
    return false;
  }

  // Optionally parse the returned row and refresh local cloud state
  StaticJsonDocument<1024> out;
  if (deserializeJson(out, resp) == DeserializationError::Ok && out.is<JsonArray>() && out.size() > 0) {
    JsonObject r = out[0];
    if (!r["pump_on"].isNull())   cloud.pump_on = r["pump_on"].as<bool>();
    if (!r["fan_on"].isNull())    cloud.fan_on  = r["fan_on"].as<bool>();
    if (!r["mode_pump"].isNull()) cloud.mode_pump = r["mode_pump"].as<const char*>();
    if (!r["mode_fan"].isNull())  cloud.mode_fan  = r["mode_fan"].as<const char*>();
  }

  Serial.printf("Synced -> pump_on=%d fan_on=%d mode_pump=%s mode_fan=%s\n",
    cloud.pump_on, cloud.fan_on, cloud.mode_pump.c_str(), cloud.mode_fan.c_str());

  return true;
}


// ------------------------ Push Buttons ------------------------
void checkButtons() {
  const unsigned long now = millis();

  // Buttons are wired to GND, INPUT_PULLUP means pressed == LOW
  int pumpBtn = digitalRead(PUMP_BTN);
  if (pumpBtn == LOW && now - lastPumpBtnTime > DEBOUNCE_MS) {
    lastPumpBtnTime = now;
    if (cloud.mode_pump == "manual") {
      cloud.pump_on = !cloud.pump_on;
      setPump(cloud.pump_on);
      Serial.printf("Manual Pump toggled -> %s\n", cloud.pump_on ? "ON" : "OFF");

      // Write back to Supabase for consistency with the dashboard
      syncManualChange("pump", cloud.pump_on);
    } else {
      Serial.println("Pump button ignored (AUTO mode)");
    }
  }

  int fanBtn = digitalRead(FAN_BTN);
  if (fanBtn == LOW && now - lastFanBtnTime > DEBOUNCE_MS) {
    lastFanBtnTime = now;
    if (cloud.mode_fan == "manual") {
      cloud.fan_on = !cloud.fan_on;
      setFan(cloud.fan_on);
      Serial.printf("Manual Fan toggled -> %s\n", cloud.fan_on ? "ON" : "OFF");

      // Write back to Supabase for consistency with the dashboard
      syncManualChange("fan", cloud.fan_on);
    } else {
      Serial.println("Fan button ignored (AUTO mode)");
    }
  }
}


// ------------------------ Output Application ------------------------
void applyOutputs() {
  // Manual modes: mirror cloud booleans directly
  if (cloud.mode_pump == "manual") {
    if (pumpApplied != cloud.pump_on) setPump(cloud.pump_on);
  }
  if (cloud.mode_fan == "manual") {
    if (fanApplied != cloud.fan_on) setFan(cloud.fan_on);
  }

  // Auto modes can be handled here if you want local autonomous behavior.
}


// ------------------------ Arduino Lifecycle ------------------------
void setup() {
  Serial.begin(115200);

  pinMode(PUMP_PIN, OUTPUT);
  pinMode(FAN_PIN,  OUTPUT);
  pinMode(PUMP_BTN, INPUT_PULLUP);
  pinMode(FAN_BTN,  INPUT_PULLUP);

  // Default both OFF
  setPump(false);
  setFan(false);

  Serial.printf("REF: %s\n", SUPABASE_REF);
  Serial.printf("ANON prefix: %.12s...\n", SUPABASE_ANON_KEY);
  Serial.printf("DEVICE_ID: %s\n", DEVICE_ID);

  ensureWiFi();
  fetchActuatorState(); // initial pull
}

void loop() {
  static uint32_t lastPoll = 0;
  const uint32_t now = millis();

  checkButtons();
  applyOutputs();

  if (now - lastPoll >= POLL_INTERVAL_MS) {
    lastPoll = now;
    fetchActuatorState();
  }

  if (WiFi.status() != WL_CONNECTED) {
    ensureWiFi();
  }

  delay(10);
}
