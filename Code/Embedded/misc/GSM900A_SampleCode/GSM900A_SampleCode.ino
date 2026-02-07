#include <SoftwareSerial.h>

SoftwareSerial mySerial(3, 1); // RX, TX (UART0 pins)

void setup() {
  Serial.begin(115200);
  mySerial.begin(9600);

  delay(1000);
  
  Serial.println("Initializing GSM...");
  
  mySerial.println("AT");     
  delay(1000);

  if (mySerial.available()) {
    Serial.println("GSM Ready");
    while(mySerial.available()){
      Serial.write(mySerial.read());
    }
  }

  mySerial.println("AT+CMGF=1");
  delay(1000);

  mySerial.println("AT+CMGS=\"+639629339856\"");
  delay(1000);

  mySerial.print("Hello from ESP32!");
  delay(500);

  mySerial.write(26);
  Serial.println("SMS Sent!");
}

void loop() {
  if (mySerial.available()) {
    Serial.write(mySerial.read());
  }
}