bool Screen_WiFiMenu_INIT = false;
lv_obj_t* wifi_list;            // Container for the list of networks
lv_obj_t* wifi_status_label;    // Label to show "Scanning..." or "Done"
bool wifi_scan_running = false; // Track if async scan is in progress

// Forward declarations
void startWiFiScan();
void populateWiFiList();

// Event handler for the Scan button
static void wifi_scan_btn_event_handler(lv_event_t* e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    if (!wifi_scan_running) {
      startWiFiScan();
    }
  }
}

void startWiFiScan() {
  // Update status
  lv_label_set_text(wifi_status_label, "Status: Scanning...");
  
  // Clear existing list
  lv_obj_clean(wifi_list);
  
  // Set WiFi to Station mode before scanning
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  
  // Start async scan
  WiFi.scanNetworks(true);
  wifi_scan_running = true;
}

void populateWiFiList() {
  int n = WiFi.scanComplete();
  if (n == -2) {
    // Check failed or still running (shouldn't happen here if called after completion)
    return;
  } else if (n == -1) {
     // Still running
     return;
  } else if (n == 0) {
    lv_label_set_text(wifi_status_label, "Status: No networks found");
  } else {
    // Networks found
    char buf[64];
    snprintf(buf, sizeof(buf), "Status: Found %d networks", n);
    lv_label_set_text(wifi_status_label, buf);
    
    for (int i = 0; i < n; ++i) {
      // Create a button for each network
      lv_obj_t* btn = lv_btn_create(wifi_list);
      lv_obj_set_width(btn, lv_pct(100)); // Full width
      lv_obj_set_height(btn, LV_SIZE_CONTENT);
      
      // Add label to button with SSID and RSSI
      String ssid = WiFi.SSID(i);
      int32_t rssi = WiFi.RSSI(i);
      
      char item_text[64];
      snprintf(item_text, sizeof(item_text), "%s (%d dBm)", ssid.c_str(), rssi);
      
      lv_obj_t* label = lv_label_create(btn);
      lv_label_set_text(label, item_text);
      lv_obj_center(label);
    }
  }
  
  // Clean up scan results to free memory
  WiFi.scanDelete();
  wifi_scan_running = false;
}

void checkScanResult() {
  if (wifi_scan_running) {
    int n = WiFi.scanComplete();
    if (n >= 0) {
      // Scan finished
      populateWiFiList();
    } else if (n == -2) {
      // Error
      lv_label_set_text(wifi_status_label, "Status: Scan failed");
      wifi_scan_running = false;
    }
    // if n == -1, it's still scanning, do nothing
  }
}

void Screen_WiFiMenu_PRE() {
  if (SCR_CurrentScreen != NULL) {
    lv_obj_del(SCR_CurrentScreen);
  }

  SCR_WiFiMenu = lv_obj_create(NULL);
  lv_scr_load(SCR_WiFiMenu);

  // Keep dark background
  lv_obj_set_style_bg_color(SCR_WiFiMenu, lv_color_hex(0x000000), 0);
  lv_obj_set_style_bg_opa(SCR_WiFiMenu, LV_OPA_COVER, 0);

  SCR_CurrentScreen = SCR_WiFiMenu;

  // Screen title with symbol
  lv_obj_t* Screen_Title = create_label(SCR_WiFiMenu, "WiFi Settings",
                                        &lv_font_montserrat_14, lv_color_white());
  lv_obj_align(Screen_Title, LV_ALIGN_TOP_MID, 0, 0);
  
  // Status Label
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

  // WiFi List Container
  wifi_list = lv_obj_create(SCR_WiFiMenu);
  lv_obj_set_size(wifi_list, 220, 240); // Adjust size to fit screen
  lv_obj_align(wifi_list, LV_ALIGN_TOP_MID, 0, 60);
  lv_obj_set_style_bg_color(wifi_list, lv_color_hex(0x222222), 0);
  lv_obj_set_flex_flow(wifi_list, LV_FLEX_FLOW_COLUMN); // List layout
  
  Screen_WiFiMenu_INIT = true;
}

void Screen_WiFiMenu_POST() {
  Screen_WiFiMenu_INIT = false;
  wifi_scan_running = false;
}

void Screen_WiFiMenu() {
  if (!Screen_WiFiMenu_INIT) {
    Screen_WiFiMenu_PRE();
  }
  
  // Check for async scan results
  checkScanResult();
}
