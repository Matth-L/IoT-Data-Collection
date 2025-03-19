// Default code to control the led
// Create a rainbow effect on the LED

#include <Adafruit_NeoPixel.h>

#define PIN_LED 0     // Control signal, connect to DI of the LED
#define NUM_LED 1     // Number of LEDs in a strip

// Custom colour1: Yellow
#define RED_VAL_1       255
#define GREEN_VAL_1     255
#define BLUE_VAL_1      0

// Custom colour2: Purple
#define RED_VAL_2       255
#define GREEN_VAL_2     0
#define BLUE_VAL_2      255

// Custom colour3: Cyan
#define RED_VAL_3       0
#define GREEN_VAL_3     255
#define BLUE_VAL_3      255

// Custom colour4: White
#define RED_VAL_4       255
#define GREEN_VAL_4     255
#define BLUE_VAL_4      255

Adafruit_NeoPixel RGB_Strip = Adafruit_NeoPixel(NUM_LED, PIN_LED, NEO_GRB + NEO_KHZ800);

void setup() {
  RGB_Strip.begin();
  RGB_Strip.show();
  RGB_Strip.setBrightness(128);    // Set brightness, 0-255 (darkest - brightest)
}

void loop() {

  colorWipe(RGB_Strip.Color(255, 0, 0), 1000);  // Red
  colorWipe(RGB_Strip.Color(0, 255, 0), 1000);  // Green
  colorWipe(RGB_Strip.Color(0, 0, 255), 1000);  // Blue

  colorWipe(RGB_Strip.Color(RED_VAL_1, GREEN_VAL_1, BLUE_VAL_1), 1000);   // Custom colour1: Yellow
  colorWipe(RGB_Strip.Color(RED_VAL_2, GREEN_VAL_2, BLUE_VAL_2), 1000);   // Custom colour2: Purple
  colorWipe(RGB_Strip.Color(RED_VAL_3, GREEN_VAL_3, BLUE_VAL_3), 1000);   // Custom colour3: Cyan
  colorWipe(RGB_Strip.Color(RED_VAL_4, GREEN_VAL_4, BLUE_VAL_4), 1000);   // Custom colour4: White

  rainbow(20);  // Rainbow
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint16_t wait) {
  for (uint16_t i = 0; i < RGB_Strip.numPixels(); i++) {
    RGB_Strip.setPixelColor(i, c);
    RGB_Strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256; j++) {
    for (i = 0; i < RGB_Strip.numPixels(); i++) {
      RGB_Strip.setPixelColor(i, Wheel((i + j) & 255));
    }
    RGB_Strip.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if (WheelPos < 85) {
    return RGB_Strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return RGB_Strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
    WheelPos -= 170;
    return RGB_Strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}