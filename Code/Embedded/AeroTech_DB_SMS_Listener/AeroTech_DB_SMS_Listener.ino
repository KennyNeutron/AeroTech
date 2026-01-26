#include <HardwareSerial.h>

HardwareSerial sim900(2); // UART2

void sendCommand(String cmd, int delayTime = 1000) {
  sim900.println(cmd);
  delay(delayTime);

  while (sim900.available()) {
    Serial.write(sim900.read());
  }
}

void sendSMS(String number, String message) {
  Serial.println("sending SMS funct");
  sendCommand("AT");                     // Test AT startup
  sendCommand("AT+CMGF=1");              // Set SMS text mode
  sendCommand("AT+CMGS=\"" + number + "\""); // Set recipient
  delay(500);
  sim900.print(message);                // Message body
  delay(500);
  sim900.write(26);                     // Ctrl+Z to send SMS
  delay(3000);
}

void setup() {
  Serial.begin(115200);

  sim900.begin(9600, SERIAL_8N1, 17, 16); // RX=16, TX=17
  delay(3000);

  Serial.println("SIM900A SMS Test...");

  sendSMS("09602312239", "Hello from ESP32 + SIM900A!");
}

void loop() {
  // nothing here
}
