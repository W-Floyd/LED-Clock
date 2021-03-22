#include <Arduino.h>
#if defined(ESP8266)
/* ESP8266 Dependencies */
#include <ESP8266WiFi.h>
#elif defined(ESP32)
/* ESP32 Dependencies */
#include <WiFi.h>
#endif

#define FASTLED_ALLOW_INTERRUPTS 0
#include "FastLED.h"

#define NUM_LEDS 28
#define DATA_PIN D5

#if FASTLED_VERSION < 3001000
#error "Requires FastLED 3.1 or later; check github for latest code."
#endif

#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Timezone.h>

// US Central Time Zone (Chicago, Houston)
TimeChangeRule usCDT = {"CDT", Second, Sun, Mar, 2, -300};
TimeChangeRule usCST = {"CST", First, Sun, Nov, 2, -360};
Timezone myTZ(usCDT, usCST);

TimeChangeRule *tcr; // pointer to the time change rule, use to get TZ abbrev

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

time_t oldtime, newtime;

uint8_t max_bright = 255;

CRGB leds[NUM_LEDS];

//    1
//   ---
// 2|   |3
//   -4-
// 5|   |6
//   ---
//    7

// const uint8_t d1[7] = {18, 27, 17, 19, 26, 16, 20};
// const uint8_t d2[7] = {9, 10, 0, 8, 11, 1, 7};
// const uint8_t d3[7] = {21, 25, 15, 22, 24, 14, 23};
// const uint8_t d4[7] = {6, 12, 2, 5, 13, 3, 4};

const uint8_t d1[7] = {4, 3, 13, 5, 2, 12, 6};
const uint8_t d2[7] = {23, 14, 24, 22, 15, 25, 21};
const uint8_t d3[7] = {7, 1, 11, 8, 0, 10, 9};
const uint8_t d4[7] = {20, 16, 26, 19, 17, 27, 18};

const uint8_t segments = 7;

uint8_t hue = 0;
uint8_t sat = 255;
uint8_t val = 63;

void blankSegments(const uint8_t map[])
{
  for (int i = 0; i < segments; i++)
  {
    leds[map[i]] = CRGB::Black;
  }
}

void lightSegment(uint8_t led)
{
  leds[led].setHSV(hue, sat, val);
}

void setDigit(const uint8_t map[], uint8_t number)
{

  blankSegments(map);

  if (number == 0 || number == 2 || number == 3 || number == 5 || number == 6 || number == 7 || number == 8 || number == 9)
  { // Top
    lightSegment(map[0]);
  }

  if (number == 0 || number == 4 || number == 5 || number == 6 || number == 8 || number == 9)
  { // Top Left
    lightSegment(map[1]);
  }

  if (number == 0 || number == 1 || number == 2 || number == 3 || number == 4 || number == 7 || number == 8 || number == 9)
  { // Top Right
    lightSegment(map[2]);
  }

  if (number == 2 || number == 3 || number == 4 || number == 5 || number == 6 || number == 8 || number == 9)
  { // Center
    lightSegment(map[3]);
  }

  if (number == 0 || number == 2 || number == 6 || number == 8)
  { // Bottom Left
    lightSegment(map[4]);
  }

  if (number == 0 || number == 1 || number == 3 || number == 4 || number == 5 || number == 6 || number == 7 || number == 8 || number == 9)
  { // Bottom Right
    lightSegment(map[5]);
  }

  if (number == 0 || number == 2 || number == 3 || number == 5 || number == 6 || number == 8 || number == 9)
  { // Bottom
    lightSegment(map[6]);
  }
}

void chooseDigit(uint8_t digit, uint8_t number)
{

  switch (digit)
  {
  case 0:
    setDigit(d1, number);
    break;
  case 1:
    setDigit(d2, number);
    break;
  case 2:
    setDigit(d3, number);
    break;
  case 3:
    setDigit(d4, number);
    break;
  }
}

void showTime()
{

  int minutes = minute(newtime);
  int hours = hour(newtime);

  int minTop = minutes / 10;
  int minBot = minutes - minTop * 10;

  int hourTop = hours / 10;
  int hourBot = hours - hourTop * 10;

  chooseDigit(0, hourTop);
  chooseDigit(1, hourBot);
  chooseDigit(2, minTop);
  chooseDigit(3, minBot);
}

/* Your WiFi Credentials */
const char *ssid = "letnet-connect";     // SSID
const char *password = "letnet-connect"; // Password

void setup()
{
  Serial.begin(115200);

  /* Connect WiFi */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.printf("WiFi Failed!\n");
    return;
  }
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(max_bright);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 2400);

  blankSegments(d1);
  blankSegments(d2);
  blankSegments(d3);
  blankSegments(d4);
  FastLED.show();

  timeClient.begin();
  timeClient.forceUpdate();
}

void loop()
{
  timeClient.update();
  newtime = myTZ.toLocal(timeClient.getEpochTime(), &tcr);
  if (minute(newtime) != minute(oldtime))
  {
    oldtime = newtime;
    showTime();
    Serial.print(hour(newtime));
    Serial.print(":");
    Serial.println(minute(newtime));
  }
  FastLED.show();
}
