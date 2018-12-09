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
#include "sleigh.h"

#define DATA_PIN    1
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    20*30
#define BRIGHTNESS          120
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

void drawBackground(unsigned char *bitmap, int pos, int width){
  int p;
  for (int y=0;y<20;y++)
    for (int x=0; x<30; x++){
      p=(y*width+((x+pos)%width))*3+8;
      screen[y*30+x]=CRGB(bitmap[p],bitmap[p+1],bitmap[p+2]);
    }
}

void drawBackgroundMasked(unsigned char *bitmap, int pos, int ypos, int height, int width){
  int p;
  CRGB c;
  for (int y=0;y<height;y++)
    for (int x=0; x<30; x++){
      p=(y*width+((x+pos)%width))*3+8;
      c=CRGB(bitmap[p],bitmap[p+1],bitmap[p+2]);
      if (c!=CRGB(0,255,255))
        screen[(y+ypos)*30+x]=c;
    }
}

void drawSprite(unsigned char *bitmap, int x, int y, int width, int height){
  int pos=8;
  CRGB c;
  for (int yy=y;yy<y+height;yy++)
    for (int xx=x;xx<x+width;xx++){
      if ((xx>=0)&&(xx<30)&&(yy>=0)&&(yy<20)){
        c=CRGB(bitmap[pos],bitmap[pos+1],bitmap[pos+2]);
        if (c!=CRGB(0,255,255))
          screen[yy*30+xx]=c;
      }
      pos+=3;
    }
}

void calcPicture(){
  drawBackground(mountains_tif, millis()/400,95);
  drawBackgroundMasked(backdrop_tif, millis()/200, 0, 20, 93);
  drawBackgroundMasked(snow_tif, millis()/100, 15, 5, 95);
 
  if ((millis()%500)>250)
    drawSprite(sleigh_tif,0,5,30,15);
  else
    drawSprite(sleigh2_tif,0,5,30,15);

  drawBackgroundMasked(snow_tif, millis()/70, 18, 5, 95);
  drawSprite(tree1_tif,50-((millis()/50)%100),3,13,17);
  drawSprite(tree2_tif,50-((millis()/30)%110),3,13,17);
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
