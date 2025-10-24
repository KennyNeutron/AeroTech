// Pin where TDS sensor is connected
#define TdsSensorPin A0

// Constants
const float VREF = 5.0;       // Reference voltage (Arduino Nano = 5V)
const int SCOUNT = 30;        // Sample count for averaging

int analogBuffer[SCOUNT];      // Buffer to store samples
int analogBufferIndex = 0;
float averageVoltage = 0;
float tdsValue = 0;

void setup() {
  Serial.begin(9600);
  pinMode(TdsSensorPin, INPUT);
}

void loop() {
  // Take multiple samples for stable reading
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
             + 857.39 * averageVoltage) * 0.5;  // Compensation factor ~0.5

  // Print results
  Serial.print("ADC: "); Serial.print(averageADC);
  Serial.print(" | Voltage: "); Serial.print(averageVoltage, 2); Serial.print(" V");
  Serial.print(" | TDS: "); Serial.print(tdsValue, 0); Serial.println(" ppm");

  delay(1000);
}
