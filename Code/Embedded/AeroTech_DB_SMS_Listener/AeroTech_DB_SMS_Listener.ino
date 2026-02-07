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

#include <time.h>

// ==========================================
// CONFIGURATION
// ==========================================

// WiFi Credentials
const char* WIFI_SSID = "Innocore2.4G";
const char* WIFI_PASSWORD = "one2nine";

// Supabase Configuration
const char* DEVICE_ID = "70d605e9-677f-4484-89bb-59cf7d98ed78";
const char* SUPABASE_URL_TARGETS = "https://njjxedmjqlkeoicckvwv.supabase.co/rest/v1/system_targets";
const char* SUPABASE_URL_READINGS = "https://njjxedmjqlkeoicckvwv.supabase.co/rest/v1/sensor_readings";
const char* SUPABASE_ANON_KEY = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6Im5qanhlZG1qcWxrZW9pY2Nrdnd2Iiwicm9sZSI6ImFub24iLCJpYXQiOjE3NjA2Njk0MTIsImV4cCI6MjA3NjI0NTQxMn0.xZTAE29TDWj8O_CskPCL4qruAIIow_HM5eE6Dwwf-QM";

// SIM900 Configuration
HardwareSerial sim900(2); // UART2: RX=16, TX=17

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

String lastAlertedTimestamp = ""; // Timestamp of last alert to prevent duplicates

// NTP Server
const char* ntpServer = "pool.ntp.org";

// ==========================================
// FUNCTIONS
// ==========================================

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

void setupTime() {
  Serial.print("Syncing time with NTP");
  configTime(0, 0, ntpServer); // Configure NTP for UTC
  
  time_t now = time(nullptr);
  int retry = 0;
  while (now < 8 * 3600 * 2 && retry < 20) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
    retry++;
  }
  Serial.println(now > 8 * 3600 * 2 ? " Done!" : " Failed (Check Network)");
  
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current UTC Time: ");
  Serial.println(asctime(&timeinfo));
}

time_t parseISODate(String str) {
  // Parses Supabase ISO8601 timestamp (YYYY-MM-DDTHH:MM:SS) to time_t
  struct tm tm = {0};
  
  if (str.length() < 19) return 0;
  
  tm.tm_year = str.substring(0, 4).toInt() - 1900;
  tm.tm_mon  = str.substring(5, 7).toInt() - 1;
  tm.tm_mday = str.substring(8, 10).toInt();
  tm.tm_hour = str.substring(11, 13).toInt();
  tm.tm_min  = str.substring(14, 16).toInt();
  tm.tm_sec  = str.substring(17, 19).toInt();
  
  return mktime(&tm);
}

void sendCommand(String cmd, int delayTime = 1000) {
  sim900.println(cmd);
  delay(delayTime);
  while (sim900.available()) {
    Serial.write(sim900.read());
  }
}



void checkAlerts() {
  if (currentTargets.smsRecipientNumber.length() < 10) return;

  // 1. Duplicate Check
  if (latestReadings.recordedAt == lastAlertedTimestamp) {
    return;
  }

  // 2. Stale Data Check
  time_t now = time(nullptr);
  time_t readingTime = parseISODate(latestReadings.recordedAt);

  // Validate timestamps
  if (now > 1000000000 && readingTime > 1000000000) {
    if ((now - readingTime) > 300) { // Check if reading is > 300 seconds (5 minutes) old
      return;
    }
  } else {
    Serial.println("Time not synced yet, skipping stale check.");
    return;
  }

  String alerts = "";

  // pH Check
  if (latestReadings.ph < currentTargets.phMin || latestReadings.ph > currentTargets.phMax) {
    alerts += "pH Alert: " + String(latestReadings.ph, 1) + " (Limit: " + String(currentTargets.phMin, 1) + "-" + String(currentTargets.phMax, 1) + ")\n";
  }

  // TDS Check
  if (latestReadings.tds < currentTargets.tdsMin || latestReadings.tds > currentTargets.tdsMax) {
    alerts += "TDS Alert: " + String(latestReadings.tds, 0) + " (Limit: " + String(currentTargets.tdsMin, 0) + "-" + String(currentTargets.tdsMax, 0) + ")\n";
  }

  // Temp Check
  if (latestReadings.temp < currentTargets.tempMin || latestReadings.temp > currentTargets.tempMax) {
    alerts += "Temp Alert: " + String(latestReadings.temp, 1) + " (Limit: " + String(currentTargets.tempMin, 1) + "-" + String(currentTargets.tempMax, 1) + ")\n";
  }

  // Water Level Check
  if (latestReadings.waterLevel < currentTargets.waterLevelTarget) {
     alerts += "Water Low: Level " + String(latestReadings.waterLevel) + " (Target: " + String(currentTargets.waterLevelTarget) + ")\n";
  }

  if (alerts.length() > 0) {
    Serial.println("!!! ALERTS DETECTED !!!");
    Serial.println(alerts);
    
    sendSMS(currentTargets.smsRecipientNumber, "AeroTech Alert:\n" + alerts);
    
    // Update last alerted timestamp to prevent duplicate notifications
    lastAlertedTimestamp = latestReadings.recordedAt;
  }
}



// ==========================================
// MAIN SETUP & LOOP
// ==========================================

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\nAeroTech SMS Listener Starting...");

  // Init SIM900
  sim900.begin(9600, SERIAL_8N1, 16, 17); // UART2: RX=16, TX=17
  delay(3000);
  Serial.println("SIM900 Initialized (Wait for network...)");

  // Init WiFi
  setupWiFi();

  // Init Time
  setupTime();

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
    
    checkAlerts();
  }
  
  // Pass through serial for manual AT command debugging
  if (Serial.available()) {
    sim900.write(Serial.read());
  }
  if (sim900.available()) {
    Serial.write(sim900.read());
  }
}
