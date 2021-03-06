#include <Arduino.h>
#include "../include/config.h"
#include <SPI.h>
#include "../lib/TFT_eSPI/TFT_eSPI.h"
#include "../lib/Adafruit_TouchScreen/TouchScreen.h"


TFT_eSPI tft = TFT_eSPI();       // Invoke custom library

#ifdef ILI9486_DRIVER
const int XP = TFT_D6, XM = TFT_DC, YP = TFT_WR, YM = TFT_D7;
#else 
const int XP = TFT_D0, XM = TFT_DC, YP = TFT_CS, YM = TFT_D1;
#endif
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 30);

#define MINPRESSURE 2
#define MAXPRESSURE 1000

// some colours to play with
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
 
// Either run TouchScreen_Calibr_native.ino and apply results to the arrays below
// or just use trial and error from drawing on screen
// ESP32 coordinates at default 12 bit resolution have range 0 - 4095
// however the ADC cannot read voltages below 150mv and tops out around 3.15V
// so the actual coordinates will not be at the extremes
// each library and driver may have different coordination and rotation sequence
const int coords[] = {3800, 500, 300, 3800}; // portrait - left, right, top, bottom

boolean Touch_getXY(uint16_t *x, uint16_t *y, boolean showTouch);

void setup(void) {
  Serial.begin(USB_SERIAL_BAUDRATE);
  tft.begin();
  tft.setRotation(SCREEN_ROTATION);
  tft.fillScreen(TFT_BLACK);
  tft.drawRect(10, 50, 20, 100, TFT_WHITE);
  tft.fillRectVGradient(10, 50, 20, 100, TFT_BLUE, TFT_RED);
}

void loop() {
    // display touched point with colored dot
    uint16_t pixel_x, pixel_y;    
    boolean pressed = Touch_getXY(&pixel_x, &pixel_y, true);
    Serial.println(pressed);
}

boolean Touch_getXY(uint16_t *x, uint16_t *y, boolean showTouch) {
    TSPoint p;
    p = ts.getPoint();
    pinMode(YP, OUTPUT);      //restore shared pins
    pinMode(XM, OUTPUT);
    digitalWrite(YP, HIGH);   //because TFT control pins
    digitalWrite(XM, HIGH);
    bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE);
    if (pressed) {
      switch (SCREEN_ROTATION) {
        case 0: // portrait
          *x = map(p.x, coords[0], coords[1], 0, tft.width()); 
          *y = map(p.y, coords[2], coords[3], 0, tft.height());
        break;
        case 1: // landscape
          *x = map(p.y, coords[1], coords[0], 0, tft.width()); 
          *y = map(p.x, coords[2], coords[3], 0, tft.height());
        break;
        case 2: // portrait inverted
          *x = map(p.x, coords[1], coords[0], 0, tft.width()); 
          *y = map(p.y, coords[3], coords[2], 0, tft.height());
        break;
        case 3: // landscape inverted
          *x = map(p.y, coords[0], coords[1], 0, tft.width()); 
          *y = map(p.x, coords[3], coords[2], 0, tft.height());
        break;
      }      
      if (showTouch) tft.fillCircle(*x, *y, 2, YELLOW);
    }
    return pressed;
}
