#include <Wire.h>
#include "RTClib.h"

RTC_DS3231 rtc;

void setup() {
  Serial.begin(9600);
  delay(1000);

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting the time...");

    // Set the date and time (Year, Month, Day, Hour, Minute, Second)
    rtc.adjust(DateTime(2025, 9, 12, 11, 45, 0));  
    // Example: 2025-09-12 11:45:00
  }
}

void loop() {
  DateTime now = rtc.now();

  // Extract time
  uint8_t hour   = now.hour();
  uint8_t minute = now.minute();
  uint8_t second = now.second();

  // Extract date
  uint8_t dow    = now.dayOfTheWeek();  // 0 = Sunday, 6 = Saturday
  uint8_t day    = now.day();
  uint8_t month  = now.month();
  uint16_t year  = now.year();

  // Temperature from DS3231
  float temperature = rtc.getTemperature();

  // Print results
  Serial.print("Time: ");
  Serial.print(hour); Serial.print(":");
  Serial.print(minute); Serial.print(":");
  Serial.println(second);

  Serial.print("Date: ");
  Serial.print(year); Serial.print("/");
  Serial.print(month); Serial.print("/");
  Serial.println(day);

  Serial.print("Day of Week: ");
  Serial.println(dow);

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");

  Serial.println("---------------------");
  delay(1000);
}
