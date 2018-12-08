#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED.h>
//FASTLED_USING_NAMESPACE
#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    1
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    20*30
#define BRIGHTNESS          96
#define FRAMES_PER_SECOND  30

CRGB leds[NUM_LEDS];
CRGB screen[NUM_LEDS];
ESP8266WebServer server(80);

void setup() {
  WiFi.mode(WIFI_STA);
  WiFi.begin("TSQ", "ficken9000!#");
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
    delay(5000);
  WiFi.config(IPAddress(192,168,178,66), IPAddress(192,168,178,1), IPAddress(255,255,255,0));  

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }
  });
  ArduinoOTA.onEnd([]() {
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
  });
  ArduinoOTA.onError([](ota_error_t error) {
  });
  ArduinoOTA.begin();

  server.on("/", handleRoot);
  server.begin();

  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
}

void handleRoot() {
  server.send(200, "text/plain", "XMas ESP Bild");
}

void drawTree(uint8_t pos, uint8_t darken){
  
}

int pos=0;
void calcPicture(){
  if (screen[pos]==CRGB(255,255,255))
    screen[pos]=CRGB::Blue;
  else
    screen[pos]=CRGB::White;
  pos++;
  if (pos==600)
    pos=0;
}

void transformPicture(){
  for (uint8_t y=0;y<20;y+=2)
    for (uint8_t x=0;x<30;x++){
      leds[y*30+x]=screen[y*30+x];
      leds[y*30+30+x]=screen[y*30+59-x];
    }
}

void loop() {
  ArduinoOTA.handle();

  server.handleClient();

  calcPicture();
  transformPicture();
  FastLED.show();  
}
