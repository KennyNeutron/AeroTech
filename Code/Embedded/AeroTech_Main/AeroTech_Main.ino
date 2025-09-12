/*******************************************************
 * Project: AeroTech
 * Board: ESP32 (WROOM Series)
 *
 * Description:
 *   The AeroTech project runs on ESP32 with optimized 
 *   settings for high-performance applications. 
 *   Configuration prioritizes maximum CPU frequency 
 *   and stable flash operations to support demanding 
 *   embedded workloads. 
 *
 * Project Settings:
 *   - CPU Frequency:        240 MHz (WiFi/BT enabled)
 *   - Core Debug Level:     None
 *   - Erase All Flash:      Disabled
 *   - Events Run On:        Core 0
 *   - Flash Frequency:      80 MHz
 *   - Flash Mode:           QIO
 *   - Flash Size:           4MB (32Mb)
 *   - JTAG Adapter:         Disabled
 *   - Arduino Runs On:      Core 0
 *   - Partition Scheme:     Huge APP (3MB No OTA / 1MB SPIFFS)
 *   - PSRAM:                Disabled
 *   - Upload Speed:         921600 baud
 *   - Zigbee Mode:          Disabled
 *
 * Notes:
 *   - Designed under the AeroTech project folder.
 *   - Optimized for real-time performance and 
 *     resource efficiency on the ESP32 platform.
 *******************************************************/

#include <lvgl.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
// #include "font_Font90Icon_48_1bpp.c"

// External sensor variables (declared in Screen_MainMenu.ino)
extern float phValue;
extern float tdsValue;
extern float waterLevel;
extern float temperature;

lv_obj_t* create_label(lv_obj_t* parent, const char* text, const lv_font_t* font, lv_color_t color);

lv_obj_t* create_label(lv_obj_t* parent, const char* text, const lv_font_t* font, lv_color_t color) {
  lv_obj_t* label = lv_label_create(parent);
  lv_label_set_text(label, text);
  lv_obj_set_style_text_font(label, font, 0);
  lv_obj_set_style_text_color(label, color, 0);
  return label;
}

// Touchscreen pins
#define XPT2046_IRQ 36   // T_IRQ
#define XPT2046_MOSI 32  // T_DIN
#define XPT2046_MISO 39  // T_OUT
#define XPT2046_CLK 25   // T_CLK
#define XPT2046_CS 33    // T_CS

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320

// Touchscreen coordinates: (x, y) and pressure (z)
int x, y, z;

#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 6 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

// Randomizer variables
unsigned long lastUpdateTime = 0;
const unsigned long UPDATE_INTERVAL = 2000; // Update every 2 seconds

// If logging is enabled, it will inform the user about what is happening in the library
void log_print(lv_log_level_t level, const char* buf) {
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}

// Function to generate realistic random sensor values
void updateSensorValues() {
  // pH: Range 5.5 to 8.5 (realistic for hydroponic systems)
  phValue = random(550, 850) / 100.0;
  
  // TDS: Range 400 to 1200 ppm (typical for nutrient solutions)
  tdsValue = random(400, 1200);
  
  // Water Level: Range 15 to 120% (can go over 100% to simulate overflow)
  waterLevel = random(15, 120);
  
  // Temperature: Range 18 to 38°C (realistic for growing environments)
  temperature = random(180, 380) / 10.0;
  
  // Print values to Serial for debugging
  Serial.print("pH: "); Serial.print(phValue);
  Serial.print(" | TDS: "); Serial.print(tdsValue);
  Serial.print(" | Water: "); Serial.print(waterLevel);
  Serial.print("% | Temp: "); Serial.println(temperature);
}

// Get the Touchscreen data
void touchscreen_read(lv_indev_t* indev, lv_indev_data_t* data) {
  // Checks if Touchscreen was touched, and prints X, Y and Pressure (Z)
  if (touchscreen.tirqTouched() && touchscreen.touched()) {
    // Get Touchscreen points
    TS_Point p = touchscreen.getPoint();
    // Calibrate Touchscreen points with map function to the correct width and height
    x = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
    y = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);
    z = p.z;

    data->state = LV_INDEV_STATE_PRESSED;

    // Set the coordinates
    data->point.x = x;
    data->point.y = y;
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

//LV Screens
uint16_t CurrentScreenID = 0x0000;
lv_obj_t* SCR_CurrentScreen;
static lv_obj_t* SCR_MainMenu;

void setup() {
  String LVGL_Arduino = String("LVGL Library Version: ") + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  Serial.begin(115200);
  Serial.println(LVGL_Arduino);

  // Initialize random seed
  randomSeed(analogRead(0));

  // Start LVGL
  lv_init();
  // Register print function for debugging
  lv_log_register_print_cb(log_print);

  // Start the SPI for the touchscreen and init the touchscreen
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  // Set the Touchscreen rotation in landscape mode
  // Note: in some displays, the touchscreen might be upside down, so you might need to set the rotation to 0: touchscreen.setRotation(0);
  touchscreen.setRotation(2);

  // Create a display object
  lv_display_t* disp;
  // Initialize the TFT display using the TFT_eSPI library
  disp = lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, draw_buf, sizeof(draw_buf));
  lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_270);

  // Initialize an LVGL input device object (Touchscreen)
  lv_indev_t* indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  // Set the callback function to read Touchscreen input
  lv_indev_set_read_cb(indev, touchscreen_read);

  // Generate initial sensor values
  updateSensorValues();
  
  Serial.println("AeroTech System Initialized");
  Serial.println("Sensor values will update every 2 seconds");
}

void loop() {
  // Check if it's time to update sensor values
  unsigned long currentTime = millis();
  if (currentTime - lastUpdateTime >= UPDATE_INTERVAL) {
    updateSensorValues();
    lastUpdateTime = currentTime;
    
    // Force screen refresh to show new values
    // Reset the screen to update with new sensor values
    extern bool Screen_MainMenu_INIT;
    Screen_MainMenu_INIT = false;
  }
  
  Screen_MainMenu();
  lv_task_handler();  // let the GUI do its work
  lv_tick_inc(5);     // tell LVGL how much time has passed
  delay(5);           // let this time pass
}