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

const char site[]="<html><body><h2>XMas</h2><a href='/wald'>Weihnachtsmann im Wald</a><br/><a href='/0'>Kranz 0 Kerzen</a><br/><a href='/1'>Kranz 1 Kerzen</a><br/><a href='/2'>Kranz 2 Kerzen</a><br/><a href='/3'>Kranz 3 Kerzen</a><br/><a href='/4'>Kranz 4 Kerzen</a><br/><a href='/5'>Kranz 5 Kerzen!!!</a></body></html>";

CRGB leds[NUM_LEDS];
CRGB screen[NUM_LEDS];
CRGB feuer[NUM_LEDS];
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
  server.on("/0", handle0);
  server.on("/1", handle1);
  server.on("/2", handle2);
  server.on("/3", handle3);
  server.on("/4", handle4);
  server.on("/5", handle5);
  server.on("/wald", handleWald);
  server.begin();

  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
}

void handleRoot() {
  server.send(200, "text/html", site);
}

int numberKerzen=0;
int scene=0;

void handle0() {
  scene=1;
  numberKerzen=0;
  server.send(200, "text/html", site);
}

void handle1() {
  scene=1;
  numberKerzen=1;
  server.send(200, "text/html", site);
}

void handle2() {
  scene=1;
  numberKerzen=2;
  server.send(200, "text/html", site);
}

void handle3() {
  scene=1;
  numberKerzen=3;
  server.send(200, "text/html", site);
}

void handle4() {
  scene=1;
  numberKerzen=4;
  server.send(200, "text/html", site);
}

void handle5() {
  scene=1;
  numberKerzen=5;
  server.send(200, "text/html", site);
}

void handleWald() {
  scene=0;
  server.send(200, "text/html", site);
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

void calcPictureWald(){
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


int lastZappel=0;
unsigned char *k[4];
void calcPictureKranz(){
  drawBackground(kranz_tif,0,30);

  if (millis()-lastZappel>100){
    for (int i=0;i<4;i++){
      switch(random(0,3)){
        case 0: k[i]=light1_tif;break;
        case 1: k[i]=light2_tif;break;
        case 2: k[i]=light3_tif;break;
        case 3: k[i]=light4_tif;break;
      }
    }
    lastZappel=millis();
  }

  if (numberKerzen>0) drawSprite(k[0],6,4,3,4);
  if (numberKerzen>1) drawSprite(k[1],11,1,3,4);
  if (numberKerzen>2) drawSprite(k[2],20,3,3,4);
  if (numberKerzen>3) drawSprite(k[3],15,6,3,4);
  if (numberKerzen>4){
    for (int y=0;y<19;y++){
      for (int x=1;x<29;x++){
        feuer[y*30+x]=CRGB(feuer[y*30+x+29].r/4+feuer[y*30+x+30].r/4+feuer[y*30+x+31].r/4,feuer[y*30+x+29].g/4+feuer[y*30+x+30].g/4+feuer[y*30+x+31].g/4,feuer[y*30+x+29].b/4+feuer[y*30+x+30].b/4+feuer[y*30+x+31].b/4);
        if (screen[y*30+x]!=CRGB(0,0,0))
          feuer[y*30+x]+=CRGB(random(screen[y*30+x].r,255),random(0,screen[y*30+x].g),0);
      }
    }
    for (int y=0;y<20;y++)
      for (int x=0;x<30;x++)
        screen[y*30+x]=screen[y*30+x]+feuer[y*30+x];
  }
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

  if (scene==0)  calcPictureWald();
  if (scene==1)  calcPictureKranz();

  transformPicture();
  FastLED.show();  
}
