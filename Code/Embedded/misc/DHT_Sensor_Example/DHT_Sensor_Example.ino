#include "DHT.h"


#define DHTPIN 6
#define DHTTYPE DHT11


DHT dht(DHTPIN, DHTTYPE);

struct AeroTechData_Example {
  float Temperature;
  float Humidity; // Note: Original AeroTechData doesn't have Humidity, but DHT provides it.
};

AeroTechData_Example currentSensorData;


void setup() {
  Serial.begin(115200);
  Serial.println("AeroTech DHT Sensor Example Starting...");

  dht.begin();
  
  Serial.println("System Initialized");
}

void loop() {
  // Wait a few seconds between measurements.
  delay(2000);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  // Store data in our struct
  currentSensorData.Temperature = t;
  currentSensorData.Humidity = h;

  // Print to Serial
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));


}
