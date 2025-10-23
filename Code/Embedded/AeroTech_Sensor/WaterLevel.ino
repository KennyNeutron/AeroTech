
void WaterLevelSensor_setup() {
    Serial.println("Water Level Sensor Initializing...");
    pinMode(WaterLevel_Low, INPUT_PULLUP);
    pinMode(WaterLevel_Mid, INPUT_PULLUP);
    pinMode(WaterLevel_High, INPUT_PULLUP);
}


/*
    Water Level Sensor
    LOW is when low= 0, mid =0, high=0  Return 0
    MID is when low=1, mid=0, high=0    Return 1
    HIGH is when low=1, mid=1, high=0   Return 2
    FULL is when low=1, mid=1, high=1   Return 3
    ERROR is any other combination      Return 4
*/
uint8_t WaterLevelSensor_Level(){
    bool levelLow = digitalRead(WaterLevel_Low);
    bool levelMid = digitalRead(WaterLevel_Mid);
    bool levelHigh = digitalRead(WaterLevel_High);

    // Serial.print("Floater Low:" + String(levelLow));
    // Serial.print(" Mid:" + String(levelMid));
    // Serial.println(" High:" + String(levelHigh));

    if(!levelLow && !levelMid && !levelHigh) {
        return 0; // LOW
    } else if(levelLow && !levelMid && !levelHigh) {
        return 1; // MID
    } else if(levelLow && levelMid && !levelHigh) {
        return 2; // HIGH
    } else if(levelLow && levelMid && levelHigh) {
        return 3; // FULL
    }else{
        return 4; // ERROR
    }
}