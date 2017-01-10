#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include "RTClib.h"



//Pin for the neopixel data 
#define PIN 6

//Number of leds in the ring
#define NUMLEDS 24

//How many seconds in a full cycle
#define SECONDSPERCYCLE 3600

//Maximum brightness in daytime
#define MAXBRIGHTDAY 195

//Maximum brightness in nighttime
#define MAXBRIGHTNIGHT 20



class ClockHandle {
  public:
    uint32_t led_index;
    uint16_t remainder;
    ClockHandle();
    void update (uint32_t timestamp);
};

ClockHandle::ClockHandle() {
  led_index = remainder = 0;
}

//Using the unix timpestamp, return the current ked and the seconds remaining for thr
//next one to light up
void ClockHandle::update(uint32_t timestamp) {
  led_index = (map (timestamp % SECONDSPERCYCLE, 0, SECONDSPERCYCLE, 0, NUMLEDS) + NUMLEDS / 2) % NUMLEDS;
  remainder = timestamp % (SECONDSPERCYCLE / NUMLEDS);
}



RTC_DS1307 RTC;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMLEDS, PIN, NEO_GRB + NEO_KHZ800);
ClockHandle handle;


void setup() {

  //Use A2 as ground and A3 as Vcc to power the RTC
  pinMode(A2, OUTPUT);
  digitalWrite(A2, LOW);
  pinMode(A3, OUTPUT);
  digitalWrite(A3, HIGH);

  Serial.begin(57600);
  Wire.begin();

  RTC.begin();
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    //RTC.adjust(DateTime(__DATE__, __TIME__));
  }

  handle = ClockHandle();

  strip.begin();
  strip.show();

}



void loop() {

  DateTime now = RTC.now();
  //  Serial.print(now.year(), DEC);
  //  Serial.print('/');
  //  Serial.print(now.month(), DEC);
  //  Serial.print('/');
  //  Serial.print(now.day(), DEC);
  //  Serial.print(' ');
  //  Serial.print(now.hour(), DEC);
  //  Serial.print(':');
  //  Serial.print(now.minute(), DEC);
  //  Serial.print(':');
  //  Serial.print(now.second(), DEC);
  //  Serial.println();

  int totalMinutes = now.hour() * 60 + now.minute();

  uint8_t maxBrightness;

  if (totalMinutes <= 7 * 60 + 30) {
    maxBrightness = MAXBRIGHTNIGHT;
  } else if (totalMinutes <= 8 * 60) {
    maxBrightness = map(totalMinutes, 7 * 60 + 31, 8 * 60, MAXBRIGHTNIGHT, MAXBRIGHTDAY);
  } else if (totalMinutes <= 19 * 60 + 30) {
    maxBrightness = MAXBRIGHTDAY;
  } else if (totalMinutes <= 20 * 60) {
    maxBrightness = map(totalMinutes, 19 * 60 + 31, 20 * 60, MAXBRIGHTDAY, MAXBRIGHTNIGHT);
  } else {
    maxBrightness = MAXBRIGHTNIGHT;
  }

  handle.update(now.unixtime());

  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor (i, maxBrightness / 2, 0, 0);
  }

  int nextLed = handle.led_index + 1;
  if (nextLed >= strip.numPixels()) nextLed = 0;

  int prevLed = handle.led_index - 1;
  if (prevLed < 0) prevLed = strip.numPixels() - 1;

  int remainderBrightness = map (handle.remainder, 0, SECONDSPERCYCLE/NUMLEDS, 0, maxBrightness);

  strip.setPixelColor (handle.led_index, 0, 0, maxBrightness);
  strip.setPixelColor (nextLed, (maxBrightness - remainderBrightness) / 2, 0, remainderBrightness);
  strip.setPixelColor (prevLed, remainderBrightness / 2, 0, maxBrightness - remainderBrightness);
  strip.show();

  delay(1000);

}






