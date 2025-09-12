struct __attribute__((packed)) AeroTechData {
    uint8_t Header = 0x00;           // Header byte for packet validation
    uint8_t AD_Time_HH = 0x00;       // Hours (0-23)
    uint8_t AD_Time_MM = 0x00;       // Minutes (0-59)
    uint8_t AD_Time_SS = 0x00;       // Seconds (0-59)
    float AD_pH = 0.0;               // pH value (typically 0.0-14.0)
    float AD_TDS = 0.0;              // TDS in ppm
    float AD_WaterLevel = 0.0;       // Water level in liters
    float AD_Temperature = 0.0;      // Temperature in Celsius
    uint8_t Footer = 0x00;           // Footer byte for packet validation
};

AeroTechData Data_AeroTech;