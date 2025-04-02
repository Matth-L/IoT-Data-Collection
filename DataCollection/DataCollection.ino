// Library to control RGBled
#include <Adafruit_NeoPixel.h>

#include <MKRWAN.h>

// Define Pins
#define PIN_LED 0
#define DETECTION_SENSOR 6
#define BUZZER 8
#define TEMPERATURE A0
#define HUMIDITY A1


#define NUM_LED 1     // Number of LEDs in a strip

Adafruit_NeoPixel RGB_Strip = Adafruit_NeoPixel(NUM_LED, PIN_LED, NEO_GRB + NEO_KHZ800); // initaliser LED

LoRaModem modem(Serial1);

String appEui;
String appKey;
String devAddr;
String nwkSKey;
String appSKey;

void connect_TTN()
{
  modem.begin(US915); // Bande de frequence pour la region Amerique du Nord

  int connected;

  appEui = "0000000000000000";
  appKey = "59C16C6E3D695D7196F88BDFB9707678";
  appKey.trim();
  appEui.trim();

  connected = modem.joinOTAA(appEui, appKey); // Connexion en mode OTAA

  while (!connected) { // Reconnexion si echec
    Serial.println("Retrying...");
    connected = modem.joinOTAA(appEui, appKey);
  }
  Serial.println("Connected.");
}



void setup() {
  // Init serial
  Serial.begin(115200);

  connect_TTN();

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
bool detection;

byte payload[5] = {0};

void loop()
{
    // If something is detected, it turns the led red
    // Elsewhere, it turns it back blue
    detection = digitalRead(DETECTION_SENSOR);
    Serial.print("Detection Sensor Value:");
    Serial.println(detection);
    if (detection)
    {
      Serial.println("On");
      colorWipe(RGB_Strip.Color(255, 0, 0), 1000);  // Red
    }
    else {
      Serial.println("Off");
      colorWipe(RGB_Strip.Color(0, 0, 255), 1000);  // Blue
    }

  humidity = analogRead(HUMIDITY); // Lecture humidité
  Serial.print("Moisture Sensor Value:");
  Serial.println(humidity);
  delay(100);

  temperature = analogRead(TEMPERATURE); // Lecture temperature
  temperature = temperature * 3.3 / 10.24; // Conversion temperature
  temp = (int)temperature;
  Serial.print("Temperature Sensor Value:");
  Serial.println(temp);

  payload[0] = highByte(temp);
  payload[1] = lowByte(temp);
  payload[2] = highByte(humidity);
  payload[3] = lowByte(humidity);
  payload[4] = detection;

  delay(100);

  modem.beginPacket();
  modem.write(payload, sizeof(payload)); 
  int err = modem.endPacket(true); 
  if (err > 0) {
    Serial.println("Message sent successfully");
  } else {
    Serial.println("Error sending message");
  }

  modem.poll();
  delay(10000);


}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint16_t wait) {
  for (uint16_t i = 0; i < RGB_Strip.numPixels(); i++) {
    RGB_Strip.setPixelColor(i, c);
    RGB_Strip.show();
    delay(wait);
  }
}
