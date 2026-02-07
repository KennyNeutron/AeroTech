void fetchSystemTargets() {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  WiFiClientSecure client;
  client.setInsecure(); // Skip cert validation for demo

  // URL: Select all columns for this device
  String url = String(SUPABASE_URL_TARGETS) + "?device_id=eq." + DEVICE_ID + "&select=*";
  
  if (!http.begin(client, url)) {
    Serial.println("Connect failed (Targets)");
    return;
  }

  http.addHeader("apikey", SUPABASE_ANON_KEY);
  http.addHeader("Authorization", String("Bearer ") + SUPABASE_ANON_KEY);
  
  int httpCode = http.GET();
  if (httpCode == 200) {
    String payload = http.getString();
    // Serial.println(payload); // Debug

    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (!error && doc.is<JsonArray>() && doc.size() > 0) {
      JsonObject obj = doc[0];
      currentTargets.phMin = obj["ph_min"] | 6.0;
      currentTargets.phMax = obj["ph_max"] | 7.0;
      currentTargets.tdsMin = obj["tds_min"] | 700.0;
      currentTargets.tdsMax = obj["tds_max"] | 900.0;
      currentTargets.tempMin = obj["temp_min"] | 22.0;
      currentTargets.tempMax = obj["temp_max"] | 26.0;
      currentTargets.waterLevelTarget = obj["water_level_target"] | 1;
      currentTargets.smsRecipientNumber = obj["smsRecipientNumber"].as<String>();

      Serial.println("--- System Targets Updated ---");
      Serial.printf("pH: %.1f - %.1f\n", currentTargets.phMin, currentTargets.phMax);
      Serial.printf("TDS: %.0f - %.0f\n", currentTargets.tdsMin, currentTargets.tdsMax);
      Serial.printf("Temp: %.1f - %.1f\n", currentTargets.tempMin, currentTargets.tempMax);
      Serial.println("SMS Number: " + currentTargets.smsRecipientNumber);
      Serial.println("------------------------------");
    } else {
      Serial.println("Error parsing Targets JSON");
    }
  } else {
    Serial.printf("HTTP Error (Targets): %d\n", httpCode);
  }
  http.end();
}