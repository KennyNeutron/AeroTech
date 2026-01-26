const char* ENDPOINT = "https://njjxedmjqlkeoicckvwv.functions.supabase.co/ingest-reading";
const char* ENDPOINT_HOST = "njjxedmjqlkeoicckvwv.functions.supabase.co";  // for DNS test

// const char* DEVICE_ID     = "70d605e9-677f-4484-89bb-59cf7d98ed78";
const char* DEVICE_SECRET = "aerotech-demo-key";
const char* SUPABASE_REST_URL = "https://njjxedmjqlkeoicckvwv.supabase.co/rest/v1/system_targets";

/* ====== TIMING ====== */
const unsigned long SEND_INTERVAL_MS = 10UL * 1000UL;   // 10 seconds
const unsigned long FETCH_INTERVAL_MS = 30UL * 1000UL;  // 30 seconds
unsigned long lastSend = 0;
unsigned long lastFetch = 0;

float randFloat(float minVal, float maxVal) {
  return minVal + (float)random(0, 10001) / 10000.0f * (maxVal - minVal);
}
int randWaterCode() {
  return random(0, 3);
}

/* --- Wait for NTP time (TLS often needs a sane clock) --- */
void syncTime() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov", "time.google.com");
  Serial.print("Syncing time");
  time_t now = time(nullptr);
  int tries = 0;
  while (now < 8 * 3600 * 2 && tries < 30) {  // ~Jan 1970 means not set
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
  // Safety check: ensure WiFi is connected
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("POST Reading: WiFi not connected, skipping");
    return false;
  }

  if (isNetworkBusy) return false;
  isNetworkBusy = true;

  WiFiClientSecure client;
  client.setInsecure();               // ⚠️ dev only; replace with setCACert for production
  client.setHandshakeTimeout(15000);  // 15s TLS handshake
  client.setTimeout(15000);           // socket timeout
  // client.setBufferSizes(4096, 1024); // uncomment on older cores if needed

  HTTPClient http;
  http.setConnectTimeout(15000);  // HTTP connect timeout

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
  isNetworkBusy = false;
  return (code == 200);
}

/* --- GET System Targets --- */
void fetchSystemTargets() {
  // Safety check: ensure WiFi is connected
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("FETCH Targets: WiFi not connected, skipping");
    return;
  }

  if (isNetworkBusy) return;
  isNetworkBusy = true;

  WiFiClientSecure client;
  client.setInsecure();
  client.setHandshakeTimeout(15000);
  client.setTimeout(15000);

  HTTPClient http;
  String url = String(SUPABASE_REST_URL) + "?device_id=eq." + DEVICE_ID + "&select=ph_min,ph_max,tds_min,tds_max,temp_min,temp_max";

  if (!http.begin(client, url)) {
    Serial.println("FETCH Targets: http.begin failed");
    isNetworkBusy = false;
    return;
  }

  http.addHeader("apikey", SUPABASE_ANON_KEY);
  http.addHeader("Authorization", String("Bearer ") + SUPABASE_ANON_KEY);
  http.addHeader("Accept", "application/json");

  int code = http.GET();
  if (code == 200) {
    String payload = http.getString();
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (!error && doc.is<JsonArray>() && doc.as<JsonArray>().size() > 0) {
      JsonObject obj = doc[0];
      phMin = obj["ph_min"] | 6.0;
      phMax = obj["ph_max"] | 7.0;
      tdsMin = obj["tds_min"] | 700.0;
      tdsMax = obj["tds_max"] | 900.0;
      tempMin = obj["temp_min"] | 22.0;
      tempMax = obj["temp_max"] | 26.0;

      Serial.println("Targets updated from Supabase:");
      Serial.printf("  pH: %.2f - %.2f\n", phMin, phMax);
      Serial.printf("  TDS: %.2f - %.2f\n", tdsMin, tdsMax);
      Serial.printf("  Temp: %.2f - %.2f\n", tempMin, tempMax);
    } else {
      Serial.println("FETCH Targets: Invalid JSON or empty array");
    }
  } else {
    Serial.printf("FETCH Targets: HTTP %d\n", code);
  }

  http.end();
  client.stop();
  isNetworkBusy = false;
}

void Supabase_setup() {
  //   Serial.begin(115200);
  //   delay(200);

  //   randomSeed(esp_random());

  //   Serial.printf("Connecting to WiFi \"%s\"", WIFI_SSID);
  //   WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  //   while (WiFi.status() != WL_CONNECTED) {
  //     delay(400);
  //     Serial.print(".");
  //   }
  //   Serial.println(" connected!");
  //   Serial.print("IP: "); Serial.println(WiFi.localIP());

  // Quick DNS resolution check
  IPAddress ip;
  if (WiFi.hostByName(ENDPOINT_HOST, ip)) {
    Serial.print("Resolved host to: ");
    Serial.println(ip);
  } else {
    Serial.println("DNS resolution FAILED");
  }

  // Get time for TLS
  syncTime();
}

bool Supabase_INIT = false;
void Supabase_loop() {
  if (!Supabase_INIT) {
    Supabase_INIT = true;
    Supabase_setup();
  }
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

  // Polling for Targets (delayed start to allow system to stabilize)
  if (now - lastFetch >= FETCH_INTERVAL_MS || (lastFetch == 0 && now > 20000)) {
    lastFetch = now;
    Serial.println("Polling Supabase Targets...");
    fetchSystemTargets();
  }

  // Pushing Readings
  if (now - lastSend >= SEND_INTERVAL_MS) {
    lastSend = now;
    // float ph    = randFloat(5.5, 7.5);
    // float tds   = randFloat(600, 900);
    // float tempC = randFloat(20.0, 28.0);
    // int water   = randWaterCode();
    postReading(phValue, tdsValue, temperature, waterLevel);
  }

  delay(50);
}
