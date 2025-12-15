bool Screen_WiFiMenu_INIT = false;

void Screen_WiFiMenu_PRE() {
  lv_obj_del(SCR_CurrentScreen);

  SCR_WiFiMenu = lv_obj_create(NULL);
  lv_scr_load(SCR_WiFiMenu);

  // Keep dark background
  lv_obj_set_style_bg_color(SCR_MainMenu, lv_color_hex(0x000000), 0);
  lv_obj_set_style_bg_opa(SCR_MainMenu, LV_OPA_COVER, 0);

  SCR_CurrentScreen = SCR_WiFiMenu;

  // Screen title with symbol
  lv_obj_t* Screen_Title = create_label(SCR_MainMenu, "WiFi Settings",
                                        &lv_font_montserrat_14, lv_color_white());
  lv_obj_align(Screen_Title, LV_ALIGN_TOP_MID, 0, 0);

  Screen_WiFiMenu_INIT = true;
}

void Screen_WiFiMenu_POST() {
  Screen_WiFiMenu_INIT = false;
}

void Screen_WiFiMenu() {
  if (Screen_WiFiMenu_INIT) {
    return;
  }

  Screen_WiFiMenu_PRE();
}
