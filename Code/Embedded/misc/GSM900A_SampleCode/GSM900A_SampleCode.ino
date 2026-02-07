#include <HardwareSerial.h>

HardwareSerial mySerial(0); // UART0 (TX0, RX0)

void setup() {
  Serial.begin(115200);       // Serial Monitor
  mySerial.begin(9600);       // SIM900A default baud rate

  delay(1000);
  
  Serial.println("Initializing GSM...");
  
  // Check communication with SIM900A
  mySerial.println("AT");     
  delay(1000);
  if (mySerial.available()) {
    Serial.println("GSM Ready");
    while(mySerial.available()){
      Serial.write(mySerial.read());
    }
  }

  // Set SMS text mode
  mySerial.println("AT+CMGF=1");
  delay(1000);

  // Send SMS
  mySerial.println("AT+CMGS=\"+639629339856\""); // replace with target number
  delay(1000);
  mySerial.print("Hello from ESP32!"); // message content
  delay(500);
  mySerial.write(26); // CTRL+Z to send
  Serial.println("SMS Sent!");
}

void loop() {
  // Read GSM responses
  if (mySerial.available()) {
    Serial.write(mySerial.read());
  }
}