// Library to control RGBled
#include <Adafruit_NeoPixel.h>
#include "pitches.h"
#include <MKRWAN.h>

// Define Pins
#define PIN_LED 0
#define DETECTION_SENSOR 6
#define TEMPERATURE A0
#define HUMIDITY A1
#define BUZZER_PIN 9
#define NUM_LED 1 // Number of LEDs in a strip

// Melody for the buzzer
int melody[] = {
  NOTE_A4, REST, NOTE_B4, REST, NOTE_C5, REST, NOTE_A4, REST,
  NOTE_D5, REST, NOTE_E5, REST, NOTE_D5, REST,

  NOTE_G4, NOTE_A4, NOTE_C5, NOTE_A4, NOTE_E5, NOTE_E5, REST,
  NOTE_D5, REST,

  NOTE_G4, NOTE_A4, NOTE_C5, NOTE_A4, NOTE_D5, NOTE_D5, REST,
  NOTE_C5, REST, NOTE_B4, NOTE_A4, REST,

  NOTE_G4, NOTE_A4, NOTE_C5, NOTE_A4, NOTE_C5, NOTE_D5, REST,
  NOTE_B4, NOTE_A4, NOTE_G4, REST, NOTE_G4, REST, NOTE_D5, REST, NOTE_C5, REST
};

int durations[] = {
  8, 8, 8, 8, 8, 8, 8, 4,
  8, 8, 8, 8, 2, 2,

  8, 8, 8, 8, 2, 8, 8,
  2, 8,

  8, 8, 8, 8, 2, 8, 8,
  4, 8, 8, 8, 8,

  8, 8, 8, 8, 2, 8, 8,
  2, 8, 4, 8, 8, 8, 8, 8, 1, 4
};


// Library to control RGBled
Adafruit_NeoPixel RGB_Strip = Adafruit_NeoPixel(NUM_LED, PIN_LED, NEO_GRB + NEO_KHZ800); // initaliser LED

// Library to control LoRa
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

  while (!connected)
  { // Reconnexion si echec
    Serial.println("Retrying...");
    connected = modem.joinOTAA(appEui, appKey);
  }
  Serial.println("Connected.");
}

void setup()
{
  // Init serial
  Serial.begin(115200);

  connect_TTN();

  // Init LED
  RGB_Strip.begin();
  RGB_Strip.show();
  RGB_Strip.setBrightness(20); // Set brightness, 0-255 (darkest - brightest)

  // Init Sensor
  pinMode(DETECTION_SENSOR, INPUT);
  // Init Temp
  pinMode(TEMPERATURE, INPUT);
  // Init Humidity
  pinMode(HUMIDITY, INPUT);
  // Init Buzzer
  pinMode(BUZZER_PIN, OUTPUT);
}

double temperature;
int temp;
int humidity;
bool detection;

int  compteur = 0;
bool envoi = false; // Envoi state
bool warning = false; // Warning state
bool alarm = false;   // Alarm state
bool led_on = false; // LED state
bool done = false;

byte payload[5] = {0};

void loop()
{
  // **************************** //
  // *            led           * //
  if (alarm == true ) {
    static unsigned long previousMillis2 = 0;
    if (millis() - previousMillis2 > 500) {
      previousMillis2 = millis();
      if (led_on == false) {
        colorWipe(RGB_Strip.Color(255, 0, 0), 1000);
        Serial.println("Led color : red On");
        led_on = true;
      } else {
        colorWipe(RGB_Strip.Color(0, 0, 0), 1000);
        Serial.println("Led color : red Off");
        led_on = false;
      }
    }
  } else if (warning == true ) {
    colorWipe(RGB_Strip.Color(255, 165, 0), 1000);
    Serial.println("Led color : Orange");
  } else {
    colorWipe(RGB_Strip.Color(0, 255, 0), 1000);
    Serial.println("Led color : Green");
  }
  // **************************** //

  // **************************** //
  // *         buzzer           * //
  if (alarm == true) {
    if (done == false) {
      done = true;
      Serial.println("Doing musics...");
      int size = sizeof(durations) / sizeof(int);

      for (int note = 0; note < size; note++) {
        int duration = 1000 / durations[note];
        tone(BUZZER_PIN, melody[note], duration);
        int pauseBetweenNotes = duration * 1.30;
        delay(pauseBetweenNotes);
        noTone(BUZZER_PIN);
      }
    } else {
      Serial.println("Stop music...");
      noTone(BUZZER_PIN);
    }
  } else {
    done = false;
    led_on = false;
  }
  // **************************** //
  
  // **************************** //
  // *Send data every 10 seconds* //
  static unsigned long previousMillis = 0;
  if (millis() - previousMillis > 20000) {
    previousMillis = millis();
    detection = digitalRead(DETECTION_SENSOR);
    Serial.print("Detection Sensor Value:");
    Serial.println(detection);

    humidity = analogRead(HUMIDITY); // Lecture humidité
    Serial.print("Moisture Sensor Value:");
    Serial.println(humidity);
    delay(100);

    temperature = analogRead(TEMPERATURE);   // Lecture temperature
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
    if (err > 0)
    {
      Serial.println("Message sent successfully");
      envoi = true;
      compteur = 0;
    }
    else
    {
      Serial.println("Error sending message");
      envoi = false;
    }
  }
  // **************************** //

  // **************************** //
  // *   Retroaction   * //
  static unsigned long previousMillis3 = 0;
  if (millis() - previousMillis3 > 1000) {
    previousMillis3 = millis();
    if (envoi) {
      Serial.print("Duree  depuis  le  dernier  envoi  de  donnees : ");
      compteur ++;
      Serial.println(compteur);
      if (!modem.available()) {
        return;
      } else {
        char rcv[64];
        int i = 0;

        while (modem.available()) {
          rcv[i++] = (char)modem.read();
        }

        Serial.print("Received: ");
        for (unsigned int j = 0; j < i; j++) {
          Serial.print(rcv[j] >> 4, HEX);
          Serial.print(rcv[j] & 0xF, HEX);
          Serial.print(" ");
        }
        Serial.println();

        // Check if first received character is '1' or '0' and control LED
        Serial.print("Received 1 : ");
        Serial.println(rcv[1], HEX);
        Serial.println("Hexa de 1 :");
        Serial.println(0x01, HEX);
        if (i > 0) {  // Make sure we received some data
          if (rcv[1] == 0x01) {
            Serial.println("Warning !");
            warning = true;
            alarm = false;
          } else if (rcv[1] == 0x00) {
            Serial.println("No problem");
            warning = false;
            alarm = false;
          } else if (rcv[1] == 0x02) {
            Serial.println("Alert !!!");
            warning = false;
            alarm = true;
          } else {
            Serial.println("error data");
          }
        }
      }
    } else {
      Serial.println("Echec  de l'envoi. Pas de  message  en  attente  de  reception.");
      return;
    }
  }
  // **************************** //

  // **************************** //
  // *    modem poll            * //
  modem.poll();
  // **************************** //
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint16_t wait)
{
  for (uint16_t i = 0; i < RGB_Strip.numPixels(); i++)
  {
    RGB_Strip.setPixelColor(i, c);
    RGB_Strip.show();
    delay(wait);
  }
}
