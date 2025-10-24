#define WaterLevel_Low 3
#define WaterLevel_Mid 4
#define WaterLevel_High 5

void setup(){
    Serial.begin(115200);
    pinMode(WaterLevel_Low, INPUT_PULLUP);
    pinMode(WaterLevel_Mid, INPUT_PULLUP);
    pinMode(WaterLevel_High, INPUT_PULLUP);
}


void loop(){
    int levelLow = digitalRead(WaterLevel_Low);
    int levelMid = digitalRead(WaterLevel_Mid);
    int levelHigh = digitalRead(WaterLevel_High);

    Serial.print("Water Level - Low: ");
    Serial.print(levelLow == LOW ? "Detected" : "Not Detected");
    Serial.print(", Mid: ");
    Serial.print(levelMid == LOW ? "Detected" : "Not Detected");
    Serial.print(", High: ");
    Serial.println(levelHigh == LOW ? "Detected" : "Not Detected");

    delay(1000);
}