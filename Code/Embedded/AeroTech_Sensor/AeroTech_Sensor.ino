#include <Wire.h>
#include "RTClib.h"
#include "DataStructure.h"

RTC_DS3231 RTC;

//TDS
#define TdsSensorPin A0
// Constants
const float VREF = 5.0;    // Reference voltage (Arduino Nano = 5V)
const int SCOUNT = 30;     // Sample count for averaging
int analogBuffer[SCOUNT];  // Buffer to store samples
int analogBufferIndex = 0;
float averageVoltage = 0;
float tdsValue = 0;
bool TDS_INIT = false;

void setup() {
  Serial.begin(115200);

  if (!RTC.begin()) {
    Serial.println("Couldn't find RTC");
    while (1)
      ;
  }

  if (RTC.lostPower()) {
    Serial.println("RTC lost power, setting the time...");

    // Set the date and time (Year, Month, Day, Hour, Minute, Second)
    RTC.adjust(DateTime(2025, 9, 12, 11, 45, 0));
    // Example: 2025-09-12 11:45:00
  }


  Serial.println("AeroTech Dummy Data Sender Ready...");

  // Fill struct with dummy values
  Data_AeroTech.Header = 0x55;  // Example header
  Data_AeroTech.AD_Time_HH = 0;
  Data_AeroTech.AD_Time_MM = 0;
  Data_AeroTech.AD_Time_SS = 0;
  Data_AeroTech.AD_pH = 6.8;
  Data_AeroTech.AD_TDS = 750.5;
  Data_AeroTech.AD_WaterLevel = 42.7;
  Data_AeroTech.AD_Temperature = 0.0;
  Data_AeroTech.Footer = 0xAA;  // Example footer
}

void loop() {

  DateTime RTC_Now = RTC.now();
  TDS_loop();

  // Extract time
  Data_AeroTech.AD_Time_HH = RTC_Now.hour();
  Data_AeroTech.AD_Time_MM = RTC_Now.minute();
  Data_AeroTech.AD_Time_SS = RTC_Now.second();

  // Extract date
  Data_AeroTech.AD_DayOfWeek = RTC_Now.dayOfTheWeek();  // 0 = Sunday, 6 = Saturday
  Data_AeroTech.AD_Date_Day = RTC_Now.day();
  Data_AeroTech.AD_Date_Month = RTC_Now.month();
  Data_AeroTech.AD_Date_Year = RTC_Now.year();

  // Temperature from DS3231
  Data_AeroTech.AD_Temperature = RTC.getTemperature();

  //TDS Value from TDS Sensor
  Data_AeroTech.AD_TDS = tdsValue;


  // Send marker + struct
  Serial.write('A');  // Start marker
  Serial.write((uint8_t*)&Data_AeroTech, sizeof(Data_AeroTech));

  // Debug print to Serial Monitor
  Serial.print("[Packet Sent] A ");
  Serial.print("Time: ");
  Serial.print(Data_AeroTech.AD_Time_HH);
  Serial.print(":");
  Serial.print(Data_AeroTech.AD_Time_MM);
  Serial.print(":");
  Serial.print(Data_AeroTech.AD_Time_SS);
  Serial.print(" | Date: ");
  Serial.print(Data_AeroTech.AD_Date_Year);
  Serial.print("/");
  Serial.print(Data_AeroTech.AD_Date_Month);
  Serial.print("/");
  Serial.print(Data_AeroTech.AD_Date_Day);
  Serial.print(" | Day: ");
  Serial.print(Data_AeroTech.AD_DayOfWeek);

  Serial.print(" | pH: ");
  Serial.print(Data_AeroTech.AD_pH);
  Serial.print(" | TDS: ");
  Serial.print(Data_AeroTech.AD_TDS);
  Serial.print(" | Level: ");
  Serial.print(Data_AeroTech.AD_WaterLevel);
  Serial.print(" | Temp: ");
  Serial.println(Data_AeroTech.AD_Temperature);
  delay(3);
}
