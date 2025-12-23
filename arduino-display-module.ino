#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

// Pin definitions
#define PIN_TFT_CS   10
#define PIN_TFT_DC   9
#define PIN_TFT_RST  8
#define PIN_TFT_LED  6

// Screen dimensions
#define DISPLAY_WIDTH   160
#define DISPLAY_HEIGHT  128

// UI layout constants
#define STATUSBAR_HEIGHT 18
#define FOOTER_HEIGHT    28
#define CONTENT_Y        STATUSBAR_HEIGHT
#define CONTENT_HEIGHT   (DISPLAY_HEIGHT - STATUSBAR_HEIGHT - FOOTER_HEIGHT)

// Color definitions
#define COLOR_BACKGROUND   ST77XX_BLACK
#define COLOR_FOREGROUND   ST77XX_WHITE
#define COLOR_ACCENT       ST77XX_CYAN
#define COLOR_STATUSBAR    ST77XX_BLUE
#define COLOR_FOOTER       ST77XX_BLACK

#define DISPLAY_TIMEOUT_MS 30000   // 30 seconds

Adafruit_ST7735 display = Adafruit_ST7735(PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_RST);
String serialInput = "";
unsigned long lastUserActivity = 0;
bool isDisplayOn = true;
bool needsRedraw;

void drawStatusBar(bool wifiConnected, bool rtcAvailable, const char* shortTime) {
  display.fillRect(0, 0, DISPLAY_WIDTH, STATUSBAR_HEIGHT, COLOR_STATUSBAR);
  display.setTextColor(ST77XX_WHITE);
  display.setTextSize(1);

  display.setCursor(6, 5);
  display.print("WiFi:");
  display.print(wifiConnected ? " OK" : " NA");

  display.setCursor(66, 5);
  display.print("RTC:");
  display.print(rtcAvailable ? " OK" : " NA");

  display.setCursor(120, 5);
  display.print(shortTime);   // e.g. "12:45"
}

void drawMainContent(String mainText, String label) {
  display.fillRect(0, CONTENT_Y, DISPLAY_WIDTH, CONTENT_HEIGHT, COLOR_BACKGROUND);

  display.setTextColor(COLOR_ACCENT);
  display.setTextSize(1);
  drawCenteredText(label, CONTENT_Y + 8);

  display.setTextSize(2);   // big, readable
  drawCenteredText(mainText, CONTENT_Y + 28);   // "12:45:08"
}

void drawFooter() {
  display.fillRect(0, DISPLAY_HEIGHT - FOOTER_HEIGHT, DISPLAY_WIDTH, FOOTER_HEIGHT, COLOR_FOOTER);
  display.setTextColor(COLOR_FOREGROUND);
  display.setTextSize(1);

  display.setCursor(10, DISPLAY_HEIGHT - FOOTER_HEIGHT + 8);
  display.print("> Sync Time");

  display.setCursor(100, DISPLAY_HEIGHT - FOOTER_HEIGHT + 8);
  display.print("< Menu");
}

void setup() {
  Serial.begin(9600);
  display.initR(INITR_BLACKTAB);
  pinMode(PIN_TFT_LED, OUTPUT);
  analogWrite(PIN_TFT_LED, 31);
  lastUserActivity = millis();
  display.setRotation(3);   // LANDSCAPE
  display.fillScreen(COLOR_BACKGROUND);

  drawStatusBar(false, false, "13:28");
  drawMainContent("13:28:32", "TIME");
  drawFooter();
}

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    wakeDisplay();           // 👈 wake on activity
    drawMainContent(cmd, "MESSAGE FROM SERIAL");
  }
  if (isDisplayOn && needsRedraw) {
    needsRedraw = false;
  }

  checkDisplayTimeout();
}

void wakeDisplay() {
  if (!isDisplayOn) {
    analogWrite(PIN_TFT_LED, 31);   // backlight ON
    isDisplayOn = true;
    needsRedraw = true;
  }
  lastUserActivity = millis();
}

void screenTimeout() {
  if (isDisplayOn) {
    analogWrite(PIN_TFT_LED, 0);    // backlight OFF
    isDisplayOn = false;
  }
}

void checkDisplayTimeout() {
  if (isDisplayOn && (millis() - lastUserActivity > DISPLAY_TIMEOUT_MS)) {
    screenTimeout();
  }
}

void drawCenteredText(String text, int16_t y) {
  int16_t x1, y1;
  uint16_t w, h;

  display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);

  int16_t x = (DISPLAY_WIDTH - w) / 2;

  display.setCursor(x, y);
  display.print(text);
}
