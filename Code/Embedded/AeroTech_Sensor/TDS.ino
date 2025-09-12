void TDS_setup() {
  pinMode(TdsSensorPin, INPUT);
}

void TDS_loop() {
  if (!TDS_INIT) {
    TDS_setup();
    TDS_INIT = true;
  }
  for (int i = 0; i < SCOUNT; i++) {
    analogBuffer[i] = analogRead(TdsSensorPin);
    delay(40);
  }

  // Compute average
  long sum = 0;
  for (int i = 0; i < SCOUNT; i++) {
    sum += analogBuffer[i];
  }
  int averageADC = sum / SCOUNT;

  // Convert ADC to voltage
  averageVoltage = averageADC * (VREF / 1024.0);

  // Convert voltage to TDS (ppm) using DFRobot’s formula
  tdsValue = (133.42 * averageVoltage * averageVoltage * averageVoltage
              - 255.86 * averageVoltage * averageVoltage
              + 857.39 * averageVoltage)
             * 0.5;  // Compensation factor ~0.5

  //   // Print results
  //   Serial.print("ADC: "); Serial.print(averageADC);
  //   Serial.print(" | Voltage: "); Serial.print(averageVoltage, 2); Serial.print(" V");
  //   Serial.print(" | TDS: "); Serial.print(tdsValue, 0); Serial.println(" ppm");

}
