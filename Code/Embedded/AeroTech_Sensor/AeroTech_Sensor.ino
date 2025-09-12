#include "DataStructure.h"

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("AeroTech Dummy Data Sender Ready...");
}

void loop() {
  // Fill struct with dummy values
  Data_AeroTech.Header        = 0xAA;     // Example header
  Data_AeroTech.AD_Time_HH    = 12;
  Data_AeroTech.AD_Time_MM    = 34;
  Data_AeroTech.AD_Time_SS    = 56;
  Data_AeroTech.AD_pH         = 6.8;
  Data_AeroTech.AD_TDS        = 750.5;
  Data_AeroTech.AD_WaterLevel = 42.7;
  Data_AeroTech.AD_Temperature= 28.3;
  Data_AeroTech.Footer        = 0x55;     // Example footer

  // Send marker + struct
  Serial.write('A');  // Start marker
  Serial.write((uint8_t*)&Data_AeroTech, sizeof(Data_AeroTech));

  // Debug print to Serial Monitor
  Serial.print("[Packet Sent] A ");
  Serial.print("Time: ");
  Serial.print(Data_AeroTech.AD_Time_HH); Serial.print(":");
  Serial.print(Data_AeroTech.AD_Time_MM); Serial.print(":");
  Serial.print(Data_AeroTech.AD_Time_SS);
  Serial.print(" | pH: "); Serial.print(Data_AeroTech.AD_pH);
  Serial.print(" | TDS: "); Serial.print(Data_AeroTech.AD_TDS);
  Serial.print(" | Level: "); Serial.print(Data_AeroTech.AD_WaterLevel);
  Serial.print(" | Temp: "); Serial.println(Data_AeroTech.AD_Temperature);

  delay(1000); // send every second
}
