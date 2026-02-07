void fetchLatestReadings() {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  WiFiClientSecure client;
  client.setInsecure();

  // URL: Get latest reading
  String url = String(SUPABASE_URL_READINGS) + "?device_id=eq." + DEVICE_ID + "&order=recorded_at.desc&limit=1";

  if (!http.begin(client, url)) {
    Serial.println("Connect failed (Readings)");
    return;
  }

  http.addHeader("apikey", SUPABASE_ANON_KEY);
  http.addHeader("Authorization", String("Bearer ") + SUPABASE_ANON_KEY);

  int httpCode = http.GET();
  if (httpCode == 200) {
    String payload = http.getString();
    
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (!error && doc.is<JsonArray>() && doc.size() > 0) {
      JsonObject obj = doc[0];
      latestReadings.ph = obj["ph"];
      latestReadings.tds = obj["tds"];
      latestReadings.temp = obj["temp_c"];
      latestReadings.waterLevel = obj["water_level_code"];
      latestReadings.recordedAt = obj["recorded_at"].as<String>();

      Serial.println("--- Latest Sensor Readings ---");
      Serial.printf("Time: %s\n", latestReadings.recordedAt.c_str());
      Serial.printf("pH: %.2f\n", latestReadings.ph);
      Serial.printf("TDS: %.0f\n", latestReadings.tds);
      Serial.printf("Temp: %.1f\n", latestReadings.temp);
      Serial.printf("Water Level Code: %d\n", latestReadings.waterLevel);
      Serial.println("------------------------------");
      
    } else {
      Serial.println("Error parsing Readings JSON");
    }
  } else {
    Serial.printf("HTTP Error (Readings): %d\n", httpCode);
  }
  http.end();
}