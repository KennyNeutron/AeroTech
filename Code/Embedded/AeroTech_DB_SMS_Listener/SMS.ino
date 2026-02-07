void sendSMS(String number, String message) {
  Serial.print("Sending SMS to ");
  Serial.println(number);
  
  if (number.length() < 10) {
    Serial.println("Error: Invalid mobile number");
    return;
  }

  // Check communication
  sim900.println("AT");     
  delay(1000);
  while(sim900.available()) Serial.write(sim900.read());

  // Set SMS text mode
  sim900.println("AT+CMGF=1");
  delay(1000);
  while(sim900.available()) Serial.write(sim900.read());

  // Send SMS command
  sim900.print("AT+CMGS=\"");
  sim900.print(number);
  sim900.println("\"");
  delay(1000);
  while(sim900.available()) Serial.write(sim900.read());

  // Message body
  sim900.print(message);                
  delay(500);

  // Ctrl+Z to send
  sim900.write(26);                     
  delay(3000);
  while(sim900.available()) Serial.write(sim900.read());
  
  Serial.println("SMS Sent!");
}