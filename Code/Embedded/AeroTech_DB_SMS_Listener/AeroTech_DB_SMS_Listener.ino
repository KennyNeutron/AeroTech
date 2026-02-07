/*
 * Board Settings:
 * Board: "ESP32 Dev Module"
 * CPU Frequency: "240MHz (WiFi/BT)"
 * Core Debug Level: "None"
 * Erase All Flash Before Sketch Upload: "Disabled"
 * Events Run On: "Core 1"
 * Flash Frequency: "80MHz"
 * Flash Mode: "QIO"
 * Flash Size: "4MB (32Mb)"
 * JTAG Adapter: "Disabled"
 * Arduino Runs On: "Core 1"
 * Partition Scheme: "Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)"
 * PSRAM: "Disabled"
 * Upload Speed: "921600"
 * Zigbee Mode: "Disabled"
 */

#include <HardwareSerial.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

// ==========================================
// CONFIGURATION
// ==========================================

// WiFi Credentials (PLACEHOLDERS - REPLACE BEFORE FLASHING)
const char* WIFI_SSID = "Innocore2.4G";
const char* WIFI_PASSWORD = "one2nine";

// Supabase Configuration
const char* DEVICE_ID = "70d605e9-677f-4484-89bb-59cf7d98ed78";
const char* SUPABASE_URL_TARGETS = "https://njjxedmjqlkeoicckvwv.supabase.co/rest/v1/system_targets";
const char* SUPABASE_URL_READINGS = "https://njjxedmjqlkeoicckvwv.supabase.co/rest/v1/sensor_readings";
const char* SUPABASE_ANON_KEY = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6Im5qanhlZG1qcWxrZW9pY2Nrdnd2Iiwicm9sZSI6ImFub24iLCJpYXQiOjE3NjA2Njk0MTIsImV4cCI6MjA3NjI0NTQxMn0.xZTAE29TDWj8O_CskPCL4qruAIIow_HM5eE6Dwwf-QM";

// SIM900 Configuration (Removed for now)
// HardwareSerial sim900(2); // UART2: RX=16, TX=17

// Global State
struct SystemTargets {
  float phMin = 6.0;
  float phMax = 7.0;
  float tdsMin = 700.0;
  float tdsMax = 900.0;
  float tempMin = 22.0;
  float tempMax = 26.0;
  int waterLevelTarget = 1; 
  String smsRecipientNumber = "";
} currentTargets;

struct SensorReadings {
  float ph = 0.0;
  float tds = 0.0;
  float temp = 0.0;
  int waterLevel = 0;
  String recordedAt = "";
} latestReadings;

unsigned long lastFetchTime = 0;
const unsigned long FETCH_INTERVAL = 30000; // 30 seconds

// ==========================================
// FUNCTIONS
// ==========================================

/* 
// GSM Functions removed for now
void sendCommand(String cmd, int delayTime = 1000) {
  sim900.println(cmd);
  delay(delayTime);
  while (sim900.available()) {
    Serial.write(sim900.read());
  }
}

void sendSMS(String number, String message) {
  // ...
}
*/

void setupWiFi() {
  Serial.printf("Connecting to WiFi: %s", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}





// ==========================================
// MAIN SETUP & LOOP
// ==========================================

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\nAeroTech SMS Listener Starting...");

  // Init SIM900 (Removed for now)
  // sim900.begin(9600, SERIAL_8N1, 17, 16); 
  // delay(3000);
  // Serial.println("SIM900 Initialized (Wait for network...)");

  // Init WiFi
  setupWiFi();

  // Initial Fetch
  fetchSystemTargets();
  fetchLatestReadings();
}

void loop() {
  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi lost, trying to reconnect...");
    WiFi.reconnect();
    delay(5000);
    return;
  }

  unsigned long now = millis();
  if (now - lastFetchTime >= FETCH_INTERVAL) {
    lastFetchTime = now;
    Serial.println("\n[Periodic Fetch]");
    fetchSystemTargets();
    fetchLatestReadings();
    
    // Future: Add logic here to compare Readings vs Targets and call sendSMS()
  }
  
  // Pass through serial for debugging (Removed)
  /*
  if (Serial.available()) {
    sim900.write(Serial.read());
  }
  if (sim900.available()) {
    Serial.write(sim900.read());
  }
  */
}
