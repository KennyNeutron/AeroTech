bool Screen_MainMenu_INIT = false;



// Store references to UI elements for efficient updates
lv_obj_t* ph_value_label;
lv_obj_t* ph_status_label;
lv_obj_t* ph_panel;

lv_obj_t* tds_value_label;
lv_obj_t* tds_status_label;
lv_obj_t* tds_panel;

lv_obj_t* water_value_label;
lv_obj_t* water_status_label;
lv_obj_t* water_panel;

lv_obj_t* temp_value_label;
lv_obj_t* temp_status_label;
lv_obj_t* temp_panel;

lv_obj_t* time_label;
lv_obj_t* date_label;

lv_obj_t* Icon_WIFI_Label;
lv_color_t Icon_WIFI_Color;

lv_obj_t* PumpMode_Label;
lv_obj_t* FanMode_Label;

lv_obj_t* FanStatus_Label;
lv_obj_t* PumpStatus_Label;

lv_obj_t* SupabaseFetch_Label;
lv_color_t SupabaseFetch_Color;

// Forward declaration of your custom create_label()
lv_obj_t* create_label(lv_obj_t* parent, const char* text, const lv_font_t* font, lv_color_t color);

// Helper function to determine pH status and color
void getPhStatus(float ph, const char** status, lv_color_t* color) {
  if (ph > 6.5) {
    *status = "Too High";
    *color = lv_color_hex(0xFF4444);  // Red for alkaline
  } else if (ph >= 5.5 && ph <= 6.5) {
    *status = "Right Value";
    *color = lv_color_hex(0x44AA44);  // Green for ideal
  } else {
    *status = "Too Low";
    *color = lv_color_hex(0xFF4444);  // Red for acidic
  }
}

// Helper function to determine TDS status and color
void getTDSStatus(float tds, const char** status, lv_color_t* color) {
  if (tds < 560) {
    *status = "Too Low";
    *color = lv_color_hex(0xFF8844);  // Orange for weak nutrients
  } else if (tds >= 560 && tds <= 840) {
    *status = "Normal";
    *color = lv_color_hex(0x44AA44);  // Green for normal
  } else {
    *status = "Too High";
    *color = lv_color_hex(0xFF4444);  // Red for strong nutrients
  }
}

// Helper function to determine Water Level status and color
void getWaterLevelStatus(float level, const char** status, lv_color_t* color) {
  if (level == 4) {
    *status = "ERROR";
    *color = lv_color_hex(0xFF4444);  // Red for error
  } else if (level == 3) {
    *status = "FULL";
    *color = lv_color_hex(0x4488FF);  // Blue for full
  } else if (level == 2) {
    *status = "HIGH";
    *color = lv_color_hex(0x44AA44);  // Green for high
  } else if (level == 1) {
    *status = "MID";
    *color = lv_color_hex(0xFF8844);  // Orange for mid
  } else if (level == 0) {
    *status = "LOW";
    *color = lv_color_hex(0xFF4444);  // Red for low
  }
}

// Helper function to determine Temperature status and color based on day/night
void getTemperatureStatus(float temp, bool isDay, const char** status, lv_color_t* color) {
  if (isDay) {
    // Day conditions: 18-24°C ideal
    if (temp < 18) {
      *status = "Too Cold";
      *color = lv_color_hex(0x4488FF);  // Blue for cold
    } else if (temp >= 18 && temp <= 24) {
      *status = "Normal";
      *color = lv_color_hex(0x44AA44);  // Green for normal
    } else {
      *status = "Too Hot";
      *color = lv_color_hex(0xFF4444);  // Red for hot
    }
  } else {
    // Night conditions: 12-18°C ideal
    if (temp < 12) {
      *status = "Too Cold";
      *color = lv_color_hex(0x4488FF);  // Blue for cold
    } else if (temp >= 12 && temp <= 18) {
      *status = "Normal";
      *color = lv_color_hex(0x44AA44);  // Green for normal
    } else {
      *status = "Too Hot";
      *color = lv_color_hex(0xFF4444);  // Red for hot
    }
  }
}

// Helper function to create a data panel and store references to updatable elements
void create_data_panel_with_refs(lv_obj_t* parent, const char* symbol, const char* title,
                                 const char* unit, float value, const char* status,
                                 lv_color_t accent_color, int x, int y,
                                 lv_obj_t** panel_ref, lv_obj_t** value_label_ref, lv_obj_t** status_label_ref) {
  // Panel container - smaller size
  lv_obj_t* panel = lv_obj_create(parent);
  *panel_ref = panel;
  lv_obj_set_size(panel, 130, 80);

  // Dark theme styling
  lv_obj_set_style_bg_color(panel, lv_color_hex(0x1a1a1a), 0);  // Dark background
  lv_obj_set_style_border_color(panel, accent_color, 0);
  lv_obj_set_style_border_width(panel, 1, 0);  // Thinner border
  lv_obj_set_style_radius(panel, 8, 0);        // Rounded corners
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
  *value_label_ref = label_value;

  // Unit
  lv_obj_t* label_unit = create_label(panel, unit, &lv_font_montserrat_10, lv_color_hex(0xaaaaaa));
  lv_obj_align(label_unit, LV_ALIGN_CENTER, 0, 10);

  // Status - smaller font
  lv_obj_t* label_status = create_label(panel, status, &lv_font_montserrat_10, accent_color);
  lv_obj_align(label_status, LV_ALIGN_BOTTOM_MID, 0, -2);
  *status_label_ref = label_status;
}

// Function to update sensor values efficiently without recreating the screen
void updateSensorDisplays() {
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

  // Update pH panel
  char ph_buf[16];
  snprintf(ph_buf, sizeof(ph_buf), "%.1f", phValue);
  lv_label_set_text(ph_value_label, ph_buf);
  lv_obj_set_style_text_color(ph_value_label, phColor, 0);
  lv_label_set_text(ph_status_label, phStatus);
  lv_obj_set_style_text_color(ph_status_label, phColor, 0);
  lv_obj_set_style_border_color(ph_panel, phColor, 0);

  // Update TDS panel
  char tds_buf[16];
  snprintf(tds_buf, sizeof(tds_buf), "%.1f", tdsValue);
  lv_label_set_text(tds_value_label, tds_buf);
  lv_obj_set_style_text_color(tds_value_label, tdsColor, 0);
  lv_label_set_text(tds_status_label, tdsStatus);
  lv_obj_set_style_text_color(tds_status_label, tdsColor, 0);
  lv_obj_set_style_border_color(tds_panel, tdsColor, 0);

  // Update Water Level panel
  char water_buf[16];
  float water_level_float = static_cast<float>(waterLevel);
  snprintf(water_buf, sizeof(water_buf), "%.1f", water_level_float);
  lv_label_set_text(water_value_label, water_buf);
  lv_obj_set_style_text_color(water_value_label, waterColor, 0);
  lv_label_set_text(water_status_label, waterStatus);
  lv_obj_set_style_text_color(water_status_label, waterColor, 0);
  lv_obj_set_style_border_color(water_panel, waterColor, 0);

  // Update Temperature panel
  char temp_buf[16];
  snprintf(temp_buf, sizeof(temp_buf), "%.1f", temperature);
  lv_label_set_text(temp_value_label, temp_buf);
  lv_obj_set_style_text_color(temp_value_label, tempColor, 0);
  lv_label_set_text(temp_status_label, tempStatus);
  lv_obj_set_style_text_color(temp_status_label, tempColor, 0);
  lv_obj_set_style_border_color(temp_panel, tempColor, 0);


  // Update time display
  char STR_Time[20];
  snprintf(STR_Time, sizeof(STR_Time), "TIME: %02d:%02d", Time_HH, Time_MM);
  lv_label_set_text(time_label, STR_Time);

  // Update date display
  char STR_Date[30];
  snprintf(STR_Date, sizeof(STR_Date), "DATE: %02d/%02d/%04d", Date_Month, Date_Day, Date_Year);
  lv_label_set_text(date_label, STR_Date);


  // Update Pump Mode display
  char STR_PumpMode[20];
  snprintf(STR_PumpMode, sizeof(STR_PumpMode), "PumpM: %s", AeroTech_PumpMode ? "A" : "M");
  lv_label_set_text(PumpMode_Label, STR_PumpMode);

  // Update Fan Mode display
  char STR_FanMode[20];
  snprintf(STR_FanMode, sizeof(STR_FanMode), "FanM: %s", AeroTech_FanMode ? "A" : "M");
  lv_label_set_text(FanMode_Label, STR_FanMode);

  // Update Pump Status display
  char STR_PumpStatus[20];
  snprintf(STR_PumpStatus, sizeof(STR_PumpStatus), "Pump: %s", AeroTech_PumpStatus ? "ON" : "OFF");
  lv_label_set_text(PumpStatus_Label, STR_PumpStatus);

  // Update Fan Status display
  char STR_FanStatus[20];
  snprintf(STR_FanStatus, sizeof(STR_FanStatus), "Fan: %s", AeroTech_FanStatus ? "ON" : "OFF");
  lv_label_set_text(FanStatus_Label, STR_FanStatus);

  // Update WiFi Icon Color
  lv_obj_set_style_text_color(Icon_WIFI_Label, AeroTech_WifiStatus ? lv_color_hex(0x00FF00) : lv_color_hex(0xFF0000), LV_PART_MAIN);

  // Update SupabaseFetch_Color
  lv_obj_set_style_text_color(SupabaseFetch_Label, AeroTech_SupabaseStatus ? lv_color_hex(0x00FF00) : lv_color_hex(0xFF0000), LV_PART_MAIN);
}

void Screen_MainMenu_PRE() {

  if (SCR_CurrentScreen != NULL) {
    lv_obj_del(SCR_CurrentScreen);
  }

  SCR_MainMenu = lv_obj_create(NULL);
  lv_scr_load(SCR_MainMenu);

  // Keep dark background
  lv_obj_set_style_bg_color(SCR_MainMenu, lv_color_hex(0x000000), 0);
  lv_obj_set_style_bg_opa(SCR_MainMenu, LV_OPA_COVER, 0);

  SCR_CurrentScreen = SCR_MainMenu;

  // Screen title with symbol
  lv_obj_t* Screen_Title = create_label(SCR_MainMenu, LV_SYMBOL_SETTINGS " AEROTECH - Aerophonics Control",
                                        &lv_font_montserrat_14, lv_color_white());
  lv_obj_align(Screen_Title, LV_ALIGN_TOP_MID, 0, 0);

  Icon_WIFI_Color = AeroTech_WifiStatus ? lv_color_hex(0x00FF00) : lv_color_hex(0xFF0000);
  Icon_WIFI_Label = create_label(SCR_MainMenu, LV_SYMBOL_WIFI, &lv_font_montserrat_12, Icon_WIFI_Color);
  lv_obj_align(Icon_WIFI_Label, LV_ALIGN_TOP_RIGHT, -3, 0);

  SupabaseFetch_Color = AeroTech_SupabaseStatus ? lv_color_hex(0x00FF00) : lv_color_hex(0xFF0000);
  SupabaseFetch_Label = create_label(SCR_MainMenu, LV_SYMBOL_DOWNLOAD, &lv_font_montserrat_12, SupabaseFetch_Color);
  lv_obj_align(SupabaseFetch_Label, LV_ALIGN_TOP_LEFT, 0, 0);

  // Format the time as string with clock symbol
  char STR_Time[20];
  snprintf(STR_Time, sizeof(STR_Time), "TIME: %02d:%02d", Time_HH, Time_MM);
  // Time label - store reference for updates
  time_label = create_label(SCR_MainMenu, STR_Time, &lv_font_montserrat_14, lv_color_white());
  lv_obj_align(time_label, LV_ALIGN_TOP_LEFT, 0, 20);

  char STR_Date[30];
  snprintf(STR_Date, sizeof(STR_Date), "DATE: %02d/%02d/%04d", Date_Month, Date_Day, Date_Year);
  // Date label - store reference for updates
  date_label = create_label(SCR_MainMenu, STR_Date, &lv_font_montserrat_14, lv_color_white());
  lv_obj_align(date_label, LV_ALIGN_TOP_LEFT, 0, 35);


  char STR_PumpMode[20];
  snprintf(STR_PumpMode, sizeof(STR_PumpMode), "PumpM: %s", AeroTech_PumpMode ? "A" : "M");
  PumpMode_Label = create_label(SCR_MainMenu, STR_PumpMode, &lv_font_montserrat_14, lv_color_white());
  lv_obj_align(PumpMode_Label, LV_ALIGN_TOP_LEFT, 150, 20);

  char STR_FanMode[20];
  snprintf(STR_FanMode, sizeof(STR_FanMode), "FanM: %s", AeroTech_FanMode ? "A" : "M");
  FanMode_Label = create_label(SCR_MainMenu, STR_FanMode, &lv_font_montserrat_14, lv_color_white());
  lv_obj_align(FanMode_Label, LV_ALIGN_TOP_LEFT, 250, 20);

  char STR_PumpStatus[20];
  snprintf(STR_PumpStatus, sizeof(STR_PumpStatus), "Pump: %s", AeroTech_PumpStatus ? "ON" : "OFF");
  PumpStatus_Label = create_label(SCR_MainMenu, STR_PumpStatus, &lv_font_montserrat_14, lv_color_white());
  lv_obj_align(PumpStatus_Label, LV_ALIGN_TOP_LEFT, 150, 35);

  char STR_FanStatus[20];
  snprintf(STR_FanStatus, sizeof(STR_FanStatus), "Fan: %s", AeroTech_FanStatus ? "ON" : "OFF");
  FanStatus_Label = create_label(SCR_MainMenu, STR_FanStatus, &lv_font_montserrat_14, lv_color_white());
  lv_obj_align(FanStatus_Label, LV_ALIGN_TOP_LEFT, 250, 35);


  // Get initial status and colors for each sensor
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

  // Create panels with references stored for efficient updates
  // pH Panel (top-left)
  create_data_panel_with_refs(SCR_MainMenu, LV_SYMBOL_DOWNLOAD, "pH Level", "pH", phValue,
                              phStatus, phColor, 20, 55, &ph_panel, &ph_value_label, &ph_status_label);

  // TDS Panel (top-right)
  create_data_panel_with_refs(SCR_MainMenu, LV_SYMBOL_CHARGE, "TDS", "ppm", tdsValue,
                              tdsStatus, tdsColor, 170, 55, &tds_panel, &tds_value_label, &tds_status_label);

  // Water Level Panel (bottom-left)
  create_data_panel_with_refs(SCR_MainMenu, LV_SYMBOL_BATTERY_3, "Water Level", " ", waterLevel,
                              waterStatus, waterColor, 20, 145, &water_panel, &water_value_label, &water_status_label);

  // Temperature Panel (bottom-right)
  create_data_panel_with_refs(SCR_MainMenu, LV_SYMBOL_EDIT, "Temperature", "°C", temperature,
                              tempStatus, tempColor, 170, 145, &temp_panel, &temp_value_label, &temp_status_label);
}

void Screen_MainMenu() {
  // Serial.println("Screen_MainMenu called");
  if (!Screen_MainMenu_INIT) {
    Screen_MainMenu_INIT = true;
    Screen_MainMenu_PRE();
  } else {
    // If screen is already initialized, just update the values efficiently
    updateSensorDisplays();
  }

  ActuatorClient_loop();
  Supabase_loop();
}

void Screen_MainMenu_POST() {
  Screen_MainMenu_INIT = false;
}