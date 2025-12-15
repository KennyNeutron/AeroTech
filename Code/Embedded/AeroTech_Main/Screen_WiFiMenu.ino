bool Screen_WiFiMenu_INIT = false;
lv_obj_t* wifi_list;            // Container for the list of networks
lv_obj_t* wifi_status_label;    // Label to show "Scanning..." or "Done"
bool wifi_scan_running = false; // Track if async scan is in progress

// Password UI
lv_obj_t* wifi_passwd_ta;       // Password text area
lv_obj_t* wifi_keyboard;        // Keyboard for password
lv_obj_t* wifi_overlay;         // Overlay to obscure background
String target_ssid = "";        // Selected SSID
bool is_connecting = false;     // Track connection attempts

// Forward declarations
void startWiFiScan();
void populateWiFiList();
void Screen_WiFiMenu_POST();

// ------------------------ HANDLERS ------------------------

// Event handler for keyboard
static void wifi_keyboard_event_handler(lv_event_t* e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_READY) {
    // User pressed checkmark/enter
    const char* pass = lv_textarea_get_text(wifi_passwd_ta);
    
    // Hide UI
    lv_obj_add_flag(wifi_keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(wifi_passwd_ta, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(wifi_overlay, LV_OBJ_FLAG_HIDDEN);
    
    // Attempt connection
    lv_label_set_text(wifi_status_label, "Status: Connecting...");
    WiFi.begin(target_ssid.c_str(), pass);
    is_connecting = true;
  } else if (code == LV_EVENT_CANCEL) {
    // Hide UI
    lv_obj_add_flag(wifi_keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(wifi_passwd_ta, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(wifi_overlay, LV_OBJ_FLAG_HIDDEN);
    is_connecting = false;
    lv_label_set_text(wifi_status_label, "Status: Cancelled");
  }
}

// Event handler for network list buttons
static void wifi_list_btn_event_handler(lv_event_t* e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    // Get button and its label
    lv_obj_t* btn = (lv_obj_t*)lv_event_get_target(e);
    lv_obj_t* label = lv_obj_get_child(btn, 0);
    const char* text = lv_label_get_text(label);
    
    // Extract SSID (remove RSSI part)
    // String format: "SSID (-xx dBm)"
    String full_text = String(text);
    int p = full_text.lastIndexOf(" (");
    if (p > 0) {
      target_ssid = full_text.substring(0, p);
    } else {
      target_ssid = full_text;
    }

    // Show Password UI
    lv_textarea_set_text(wifi_passwd_ta, ""); // Clear previous
    lv_obj_clear_flag(wifi_overlay, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(wifi_passwd_ta, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(wifi_keyboard, LV_OBJ_FLAG_HIDDEN);
    
    // Focus keyboard
    lv_keyboard_set_textarea(wifi_keyboard, wifi_passwd_ta);
  }
}

// Event handler for the Scan button
static void wifi_scan_btn_event_handler(lv_event_t* e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    if (!wifi_scan_running && !is_connecting) {
      startWiFiScan();
    }
  }
}

// ------------------------ FUNCTIONS ------------------------

void startWiFiScan() {
  lv_label_set_text(wifi_status_label, "Status: Scanning...");
  lv_obj_clean(wifi_list);
  
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  
  WiFi.scanNetworks(true);
  wifi_scan_running = true;
}

void populateWiFiList() {
  int n = WiFi.scanComplete();
  if (n == -2) return; // Error
  if (n == -1) return; // Still running
  
  if (n == 0) {
    lv_label_set_text(wifi_status_label, "Status: No networks found");
  } else {
    char buf[64];
    snprintf(buf, sizeof(buf), "Status: Found %d networks", n);
    lv_label_set_text(wifi_status_label, buf);
    
    for (int i = 0; i < n; ++i) {
      lv_obj_t* btn = lv_btn_create(wifi_list);
      lv_obj_set_width(btn, lv_pct(100)); 
      lv_obj_set_height(btn, LV_SIZE_CONTENT);
      lv_obj_add_event_cb(btn, wifi_list_btn_event_handler, LV_EVENT_CLICKED, NULL);
      
      String ssid = WiFi.SSID(i);
      int32_t rssi = WiFi.RSSI(i);
      
      char item_text[64];
      snprintf(item_text, sizeof(item_text), "%s (%d dBm)", ssid.c_str(), rssi);
      
      lv_obj_t* label = lv_label_create(btn);
      lv_label_set_text(label, item_text);
      lv_obj_center(label);
    }
  }
  
  WiFi.scanDelete();
  wifi_scan_running = false;
}

void checkScanResult() {
  if (wifi_scan_running) {
    int n = WiFi.scanComplete();
    if (n >= 0) {
      populateWiFiList();
    } else if (n == -2) {
      lv_label_set_text(wifi_status_label, "Status: Scan failed");
      wifi_scan_running = false;
    }
  }
}

void checkConnection() {
  if (is_connecting) {
    if (WiFi.status() == WL_CONNECTED) {
      is_connecting = false;
      lv_label_set_text(wifi_status_label, "Status: Connected!");
      delay(500); // Brief pause to show success
      Screen_WiFiMenu_POST();
    } else if (WiFi.status() == WL_CONNECT_FAILED || WiFi.status() == WL_NO_SSID_AVAIL) {
        // Only fail on explicit error states, otherwise keep trying
        // Note: simple wait loop, might need timeout in real app
    }
  }
}

// ------------------------ SCREEN LIFECYCLE ------------------------

void Screen_WiFiMenu_PRE() {
  if (SCR_CurrentScreen != NULL) {
    lv_obj_del(SCR_CurrentScreen);
  }

  SCR_WiFiMenu = lv_obj_create(NULL);
  lv_scr_load(SCR_WiFiMenu);

  lv_obj_set_style_bg_color(SCR_WiFiMenu, lv_color_hex(0x000000), 0);
  lv_obj_set_style_bg_opa(SCR_WiFiMenu, LV_OPA_COVER, 0);

  SCR_CurrentScreen = SCR_WiFiMenu;

  // Title
  lv_obj_t* Screen_Title = create_label(SCR_WiFiMenu, "WiFi Settings",
                                        &lv_font_montserrat_14, lv_color_white());
  lv_obj_align(Screen_Title, LV_ALIGN_TOP_MID, 0, 0);
  
  // Status
  wifi_status_label = create_label(SCR_WiFiMenu, "Status: Idle", &lv_font_montserrat_12, lv_color_hex(0xAAAAAA));
  lv_obj_align(wifi_status_label, LV_ALIGN_TOP_LEFT, 10, 25);

  // Scan Button
  lv_obj_t* btn_scan = lv_btn_create(SCR_WiFiMenu);
  lv_obj_set_size(btn_scan, 80, 30);
  lv_obj_align(btn_scan, LV_ALIGN_TOP_RIGHT, -10, 20);
  lv_obj_add_event_cb(btn_scan, wifi_scan_btn_event_handler, LV_EVENT_CLICKED, NULL);
  
  lv_obj_t* label_scan = lv_label_create(btn_scan);
  lv_label_set_text(label_scan, "Scan");
  lv_obj_center(label_scan);

  // List
  wifi_list = lv_obj_create(SCR_WiFiMenu);
  lv_obj_set_size(wifi_list, 220, 240); 
  lv_obj_align(wifi_list, LV_ALIGN_TOP_MID, 0, 60);
  lv_obj_set_style_bg_color(wifi_list, lv_color_hex(0x222222), 0);
  lv_obj_set_flex_flow(wifi_list, LV_FLEX_FLOW_COLUMN);
  
  // --- PASSWORD OVERLAY ---
  
  // Semi-transparent overlay
  wifi_overlay = lv_obj_create(SCR_WiFiMenu);
  lv_obj_set_size(wifi_overlay, LV_PCT(100), LV_PCT(100));
  lv_obj_set_style_bg_color(wifi_overlay, lv_color_black(), 0);
  lv_obj_set_style_bg_opa(wifi_overlay, LV_OPA_80, 0);
  lv_obj_center(wifi_overlay);
  lv_obj_add_flag(wifi_overlay, LV_OBJ_FLAG_HIDDEN); // Hide initially

  // Password Input
  wifi_passwd_ta = lv_textarea_create(SCR_WiFiMenu);
  lv_textarea_set_one_line(wifi_passwd_ta, true);
  lv_textarea_set_password_mode(wifi_passwd_ta, true);
  lv_textarea_set_placeholder_text(wifi_passwd_ta, "Password");
  lv_obj_set_width(wifi_passwd_ta, 200);
  lv_obj_align(wifi_passwd_ta, LV_ALIGN_TOP_MID, 0, 80);
  lv_obj_add_flag(wifi_passwd_ta, LV_OBJ_FLAG_HIDDEN); // Hide initially

  // Keyboard
  wifi_keyboard = lv_keyboard_create(SCR_WiFiMenu);
  lv_obj_add_flag(wifi_keyboard, LV_OBJ_FLAG_HIDDEN); // Hide initially
  lv_obj_add_event_cb(wifi_keyboard, wifi_keyboard_event_handler, LV_EVENT_ALL, NULL);
  
  Screen_WiFiMenu_INIT = true;
}

void Screen_WiFiMenu_POST() {
  Screen_WiFiMenu_INIT = false;
  wifi_scan_running = false;
  is_connecting = false;
  CurrentScreenID = 0x0000; // Return to Main Menu
}

void Screen_WiFiMenu() {
  if (!Screen_WiFiMenu_INIT) {
    Screen_WiFiMenu_PRE();
  }
  
  checkScanResult();
  checkConnection();
}
