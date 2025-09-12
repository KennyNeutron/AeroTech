bool Screen_MainMenu_INIT = false;

// Example variables for sensor data
float phValue = 7.4;
float tdsValue = 774.0;
float waterLevel = 104.0;
float temperature = 30.4;

// Day/Night mode variable (you can set this based on time or user preference)
bool isDayTime = true; // Set to false for night mode

// Example time variables
uint8_t Time_HH = 10;
uint8_t Time_MM = 0;
uint8_t Time_SS = 0;

// Forward declaration of your custom create_label()
lv_obj_t* create_label(lv_obj_t* parent, const char* text, const lv_font_t* font, lv_color_t color);

// Helper function to determine pH status and color
void getPhStatus(float ph, const char** status, lv_color_t* color) {
  if (ph > 6.5) {
    *status = "Too High";
    *color = lv_color_hex(0xFF4444); // Red for alkaline
  } else if (ph >= 5.5 && ph <= 6.5) {
    *status = "Right Value";
    *color = lv_color_hex(0x44AA44); // Green for ideal
  } else {
    *status = "Too Low";
    *color = lv_color_hex(0xFF4444); // Red for acidic
  }
}

// Helper function to determine TDS status and color
void getTDSStatus(float tds, const char** status, lv_color_t* color) {
  if (tds < 560) {
    *status = "Too Low";
    *color = lv_color_hex(0xFF8844); // Orange for weak nutrients
  } else if (tds >= 560 && tds <= 840) {
    *status = "Normal";
    *color = lv_color_hex(0x44AA44); // Green for normal
  } else {
    *status = "Too High";
    *color = lv_color_hex(0xFF4444); // Red for strong nutrients
  }
}

// Helper function to determine Water Level status and color
void getWaterLevelStatus(float level, const char** status, lv_color_t* color) {
  if (level >= 70) {
    *status = "FULL";
    *color = lv_color_hex(0x4488FF); // Blue for full
  } else if (level >= 40 && level <= 69) {
    *status = "MID";
    *color = lv_color_hex(0x44AA44); // Green for mid
  } else {
    *status = "LOW";
    *color = lv_color_hex(0xFF4444); // Red for low
  }
}

// Helper function to determine Temperature status and color based on day/night
void getTemperatureStatus(float temp, bool isDay, const char** status, lv_color_t* color) {
  if (isDay) {
    // Day conditions: 18-24°C ideal
    if (temp < 18) {
      *status = "Too Cold";
      *color = lv_color_hex(0x4488FF); // Blue for cold
    } else if (temp >= 18 && temp <= 24) {
      *status = "Normal";
      *color = lv_color_hex(0x44AA44); // Green for normal
    } else {
      *status = "Too Hot";
      *color = lv_color_hex(0xFF4444); // Red for hot
    }
  } else {
    // Night conditions: 12-18°C ideal
    if (temp < 12) {
      *status = "Too Cold";
      *color = lv_color_hex(0x4488FF); // Blue for cold
    } else if (temp >= 12 && temp <= 18) {
      *status = "Normal";
      *color = lv_color_hex(0x44AA44); // Green for normal
    } else {
      *status = "Too Hot";
      *color = lv_color_hex(0xFF4444); // Red for hot
    }
  }
}

// Helper function to create a smaller data panel with dark theme and symbols
lv_obj_t* create_data_panel(lv_obj_t* parent, const char* symbol, const char* title, 
                            const char* unit, float value, const char* status, 
                            lv_color_t accent_color, int x, int y) {
  // Panel container - smaller size
  lv_obj_t* panel = lv_obj_create(parent);
  lv_obj_set_size(panel, 130, 80);  
  
  // Dark theme styling
  lv_obj_set_style_bg_color(panel, lv_color_hex(0x1a1a1a), 0);  // Dark background
  lv_obj_set_style_border_color(panel, accent_color, 0);
  lv_obj_set_style_border_width(panel, 1, 0);  // Thinner border
  lv_obj_set_style_radius(panel, 8, 0);  // Rounded corners
  lv_obj_align(panel, LV_ALIGN_TOP_LEFT, x, y);

  // Create title with symbol
  char title_with_symbol[64];
  snprintf(title_with_symbol, sizeof(title_with_symbol), "%s %s", symbol, title);
  
  lv_obj_t* label_title = create_label(panel, title_with_symbol, &lv_font_montserrat_12, lv_color_white());
  lv_obj_align(label_title, LV_ALIGN_TOP_MID, 0, 2);

  // Value - smaller font to fit
  char buf[16];
  snprintf(buf, sizeof(buf), "%.1f", value);
  lv_obj_t* label_value = create_label(panel, buf, &lv_font_montserrat_16, accent_color);
  lv_obj_align(label_value, LV_ALIGN_CENTER, 0, -3);

  // Unit
  lv_obj_t* label_unit = create_label(panel, unit, &lv_font_montserrat_10, lv_color_hex(0xaaaaaa));
  lv_obj_align(label_unit, LV_ALIGN_CENTER, 0, 10);

  // Status - smaller font
  lv_obj_t* label_status = create_label(panel, status, &lv_font_montserrat_10, accent_color);
  lv_obj_align(label_status, LV_ALIGN_BOTTOM_MID, 0, -2);

  return panel;
}

void Screen_MainMenu_PRE() {
  SCR_MainMenu = lv_obj_create(NULL);
  lv_scr_load(SCR_MainMenu);

  // Keep dark background
  lv_obj_set_style_bg_color(SCR_MainMenu, lv_color_hex(0x000000), 0);
  lv_obj_set_style_bg_opa(SCR_MainMenu, LV_OPA_COVER, 0);

  // Screen title with symbol
  lv_obj_t* Screen_Title = create_label(SCR_MainMenu, LV_SYMBOL_SETTINGS " AEROTECH - Aerophonics Control", 
                                       &lv_font_montserrat_14, lv_color_white());
  lv_obj_align(Screen_Title, LV_ALIGN_TOP_MID, 0, 0);

  uint8_t Time_HH = 12;
  uint8_t Time_MM = 34;
  uint8_t Time_SS = 56;

  // Format the time as string with clock symbol
  char STR_Time[20];
  snprintf(STR_Time, sizeof(STR_Time), "TIME: %02d:%02d:%02d", Time_HH, Time_MM, Time_SS);

  // Time label
  lv_obj_t* Time_Label = create_label(SCR_MainMenu, STR_Time, &lv_font_montserrat_18, lv_color_white());
  lv_obj_align(Time_Label, LV_ALIGN_TOP_LEFT, 0, 25);

  // Get status and colors for each sensor using the new conditions
  const char* phStatus;
  lv_color_t phColor;
  getPhStatus(phValue, &phStatus, &phColor);

  const char* tdsStatus;
  lv_color_t tdsColor;
  getTDSStatus(tdsValue, &tdsStatus, &tdsColor);

  const char* waterStatus;
  lv_color_t waterColor;
  getWaterLevelStatus(waterLevel, &waterStatus, &waterColor);

  const char* tempStatus;
  lv_color_t tempColor;
  getTemperatureStatus(temperature, isDayTime, &tempStatus, &tempColor);

  // Create panels with updated conditions
  // pH Panel (top-left)
  create_data_panel(SCR_MainMenu, LV_SYMBOL_DOWNLOAD, "pH Level", "pH", phValue,
                    phStatus, phColor, 20, 55);

  // TDS Panel (top-right)
  create_data_panel(SCR_MainMenu, LV_SYMBOL_CHARGE, "TDS", "ppm", tdsValue,
                    tdsStatus, tdsColor, 170, 55);

  // Water Level Panel (bottom-left)
  create_data_panel(SCR_MainMenu, LV_SYMBOL_BATTERY_3, "Water Level", "L", waterLevel,
                    waterStatus, waterColor, 20, 145);

  // Temperature Panel (bottom-right)
  create_data_panel(SCR_MainMenu, LV_SYMBOL_EDIT, "Temperature", "°C", temperature,
                    tempStatus, tempColor, 170, 145);
}

void Screen_MainMenu() {
  if (!Screen_MainMenu_INIT) {
    Screen_MainMenu_INIT = true;
    Screen_MainMenu_PRE();
  }
}

void Screen_MainMenu_POST() {
  Screen_MainMenu_INIT = false;
}