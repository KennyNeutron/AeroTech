bool Screen_MainMenu_INIT = false;

void Screen_MainMenu_PRE() {
  SCR_MainMenu = lv_obj_create(NULL);
  lv_scr_load(SCR_MainMenu);

  lv_obj_set_style_bg_color(SCR_MainMenu, lv_color_hex(0x000000), 0);
  lv_obj_set_style_bg_opa(SCR_MainMenu, LV_OPA_COVER, 0);

  lv_obj_t* Screen_Title = create_label(SCR_MainMenu, "AEROTECH- Aerophonics Control System", &lv_font_montserrat_14, lv_color_white());
  lv_obj_align(Screen_Title, LV_ALIGN_TOP_MID, 0, 0);

  uint8_t Time_HH = 12;
  uint8_t Time_MM = 34;
  uint8_t Time_SS = 56;

  // Format the time as string
  char STR_Time[15];  // "HH:MM:SS" + null terminator
  snprintf(STR_Time, sizeof(STR_Time), "TIME:%02d:%02d:%02d", Time_HH, Time_MM, Time_SS);

  // Time label
  lv_obj_t* Time_Label = create_label(SCR_MainMenu, STR_Time, &lv_font_montserrat_18, lv_color_white());
  lv_obj_align(Time_Label, LV_ALIGN_TOP_LEFT, 0, 25);
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