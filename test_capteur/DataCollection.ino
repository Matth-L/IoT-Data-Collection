// If something is detected by the detection sensor, it turns the led red
// Elsewhere, it turns it back blue

// Library to control RGBled
#include <Adafruit_NeoPixel.h>

// Define Pins
#define PIN_LED 0
#define DETECTION_SENSOR 6
#define BUZZER 8
#define TEMPERATURE A0
#define HUMIDITY A1

#define NUM_LED 1     // Number of LEDs in a strip

Adafruit_NeoPixel RGB_Strip = Adafruit_NeoPixel(NUM_LED, PIN_LED, NEO_GRB + NEO_KHZ800); // initaliser LED

void setup() {
  // Init serial
  Serial.begin(9600);

  // Init LED
  RGB_Strip.begin();
  RGB_Strip.show();
  RGB_Strip.setBrightness(20);    // Set brightness, 0-255 (darkest - brightest)

  // Init Sensor
  pinMode(DETECTION_SENSOR, INPUT);
  // Init Temp
  pinMode(TEMPERATURE, INPUT);
  // Init Humidity
  pinMode(HUMIDITY, INPUT);
}


double temperature;
int temp;

int humidity;

void loop()
{
    // If something is detected, it turns the led red
    // Elsewhere, it turns it back blue
    if (digitalRead(DETECTION_SENSOR) == HIGH)
    {
      Serial.println("On");
      colorWipe(RGB_Strip.Color(255, 0, 0), 1000);  // Red
    }
    else {
      Serial.println("Off");
      colorWipe(RGB_Strip.Color(0, 0, 255), 1000);  // Blue
    }

  Serial.print("Moisture Sensor Value:");
  Serial.println(analogRead(HUMIDITY));
  delay(100);

  temperature = analogRead(TEMPERATURE); // Lecture temperature
  temperature = temperature * 3.3 / 10.24; // Conversion temperature
  temp = (int)temperature;
  Serial.print("Temperature Sensor Value:");
  Serial.println(temp);
  delay(100);


}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint16_t wait) {
  for (uint16_t i = 0; i < RGB_Strip.numPixels(); i++) {
    RGB_Strip.setPixelColor(i, c);
    RGB_Strip.show();
    delay(wait);
  }
}