#include <SoftwareSerial.h>

// RX, TX (Arduino side)
SoftwareSerial sim900(7, 8);

void sendSMS() {
  sim900.println("AT");
  delay(1000);

  sim900.println("AT+CMGF=1");   // Set SMS text mode
  delay(1000);

  sim900.println("AT+CMGS=\"+639602312239\""); // <-- replace with your number
  delay(1000);

  sim900.print("Hello World from SIM900A!");
  delay(500);

  sim900.write(26); // CTRL+Z to send SMS
  delay(5000);
}

void setup() {
  sim900.begin(9600);
  delay(5000);   // Let SIM900A boot and register to network

  sendSMS();     // Send SMS once on startup
}

void loop() {
  // nothing here
}
