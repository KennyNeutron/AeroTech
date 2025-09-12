bool Screen_MainMenu_INIT = false;

// Example variables for sensor data
float phValue = 7.4;
float tdsValue = 774.0;
float waterLevel = 104.0;
float temperature = 30.4;

// Example time variables
uint8_t Time_HH = 10;
uint8_t Time_MM = 0;
uint8_t Time_SS = 0;

// Forward declaration of your custom create_label()
lv_obj_t* create_label(lv_obj_t* parent, const char* text, const lv_font_t* font, lv_color_t color);

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
  lv_obj_align(label_status, LV_ALIGN_BOTTOM_MID, 0, -3);

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

  // Smaller panels arranged in 2x2 grid with more spacing
  // pH Panel (top-left)
  create_data_panel(SCR_MainMenu, LV_SYMBOL_DOWNLOAD, "pH Level", "pH", phValue,
                    (phValue < 6.5 || phValue > 7.5) ? "Critical" : "Normal",
                    (phValue < 6.5 || phValue > 7.5) ? lv_color_hex(0xFF4444) : lv_color_hex(0x44AA44), 
                    20, 55);

  // TDS Panel (top-right)
  create_data_panel(SCR_MainMenu, LV_SYMBOL_CHARGE, "TDS", "ppm", tdsValue,
                    "Normal",
                    lv_color_hex(0x44AA44), 170, 55);

  // Water Level Panel (bottom-left)
  create_data_panel(SCR_MainMenu, LV_SYMBOL_BATTERY_3, "Water Level", "%", waterLevel,
                    (waterLevel < 20 || waterLevel > 100) ? "Critical" : "Normal",
                    (waterLevel < 20 || waterLevel > 100) ? lv_color_hex(0xFF4444) : lv_color_hex(0x4488FF), 
                    20, 145);

  // Temperature Panel (bottom-right)
  create_data_panel(SCR_MainMenu, LV_SYMBOL_EDIT, "Temperature", "°C", temperature,
                    (temperature > 35.0 || temperature < 18.0) ? "Critical" : "Normal",
                    (temperature > 35.0 || temperature < 18.0) ? lv_color_hex(0xFF4444) : lv_color_hex(0xFF8844), 
                    170, 145);
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