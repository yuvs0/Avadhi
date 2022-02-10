#include <Arduino_GFX_Library.h>
#define TFT_BL 23
Arduino_DataBus *bus = new Arduino_ESP32SPI(27 /* DC */, 5 /* CS */, 18 /* SCK */, 23 /* MOSI */, -1 /* MISO */, VSPI /* spi_num */);
Arduino_GFX *gfx = new Arduino_GC9A01(bus, 7 /* RST */, 0 /* rotation */, true /* IPS */);

#include "NewPing.h"
#define TRIGGER_PIN 14
#define ECHO_PIN 12
#define MAX_DISTANCE 400

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
unsigned int fps = 20;
bool timerRunning = false;
bool timerCancelled = false;

void dispTime(int seconds){
  int ss, mm;
  gfx->fillRect(45,95,150,50, 0x0000);
  gfx->setCursor(50,100);
  ss = seconds % 60;
  mm = (seconds - ss) /60;
  if (mm < 10) gfx->print("0");
  gfx->print(mm);
  gfx->print(":");
  if (ss < 10) gfx->print("0");
  gfx->print(ss);
}

struct GraphicTimer {
  
  uint16_t inittot, tot, sec, min;
  uint16_t prevtot;
  uint16_t tarinc;
  double sta, ang, end;
  unsigned long tar;
  
  void init (int seconds){
    inittot = seconds;
    gfx->fillScreen(0x0000);
    gfx->drawArc(120,120,116,103,0.01,359.9,0x1048);
    gfx->drawArc(120,120,115,104,0.01,359.9,0x3094);
    gfx->fillArc(120,120,114,105,0.01,359.9,0x599D);
    tot = seconds+1;
    sta = static_cast<float>(tot);
    if (seconds > 600) fps = 5;
    ang = (float) 360/(fps*seconds);
    end = 269.999;
    sec = tot % 60;
    min = (tot - sec) /60;
    tarinc = 1000/fps;
    tar = millis() + tarinc;
    gfx->setCursor(50,100);
    gfx->setTextSize(5,5,1);
    if (min < 10) gfx->print("0");
    gfx->print(min);
    gfx->print(":");
    if (sec < 10) gfx->print("0");
    gfx->print(sec);
    timerRunning = true;
  }
  
  void inc (){
    tar += tarinc;
    if (sta > 0.001)
      {sta -= (float) 1.00/fps;
      end -= ang;
     }
    else timerRunning = false;
    tot = static_cast<int>(sta);
    if ((tot < inittot - 3) && (sonar.ping() < 17.5)){
        timerRunning = false;
        timerCancelled = true;
    }
  }

  void graphicsUpdate (){
    if (prevtot != tot){
      dispTime(tot);
      end = 269.99 - ((inittot - tot) * (360.0 / inittot));
      Serial.println(millis());
    }
    gfx->fillArc(120,120,116,103,end,270,0x0000);
    prevtot = tot;
  }
  
};

GraphicTimer dt;

void setTimer() {
  
  bool slapVel = false;
  float  prev2dist, prevdist, duration, distance, irmtot;
  uint16_t prev4tot, prev3tot, prev2tot, prevtot, targtot;
  signed int vel1, vel2;
  
  prev3tot = 0;
  prev2tot = 0;
  prevtot = 0;
  targtot = 0;

  gfx->fillScreen(BLACK);
  gfx->setTextSize(5,5,1);

  gfx->drawArc(120,120,116,103,0.01,359.9,0x1048);
  gfx->drawArc(120,120,115,104,0.01,359.9,0x3094);
  gfx->fillArc(120,120,114,105,0.01,359.9,0x599D);

  while(!slapVel){

    if (targtot > 3600) targtot = 3600;
    prev4tot = prev3tot;
    prev3tot = prev2tot;
    prev2tot = prevtot;
    prevtot = targtot;
    prev2dist = prevdist;
    prevdist = distance;
    
    duration = sonar.ping_median(5);
    distance = roundf(((duration / 2) * 0.0343) * 100) / 100 ;
    if (distance < 100.01 && distance > 0.001){
      
      vel1 = static_cast<int>(distance - prevdist);
      vel2 = static_cast<int>(prevdist - prev2dist);

      if (vel1 > -30 && vel1 < -4){
        if (vel2 > -12 && vel2 < 0) {
          slapVel = true;
        }
      }
      
      Serial.println(vel1);
  
      if (12 <= distance && distance < 25) {
        irmtot = (6.00/13.00) * (distance - 12.00);
        targtot = static_cast<int>(irmtot);
        targtot *= 10; 
      }
      else if (25 <= distance && distance < 55) {
        irmtot = (19.00/30.00) * (distance - 25.00);
        targtot = static_cast<int>(irmtot);
        targtot *= 60;
      }
      else if (55 <= distance && distance < 56) ;
      else if (56 <= distance && distance < 59) targtot = 20 * 60;
      else if (59 <= distance && distance < 61) targtot = 25 * 60;
      else if (62 <= distance && distance < 64) targtot = 30 * 60;
      else if (65 <= distance && distance < 67) targtot = 35 * 60;
      else if (68 <= distance && distance < 70) targtot = 40 * 60;
      else if (71 <= distance && distance < 73) targtot = 45 * 60;
      else if (74 <= distance && distance < 76) targtot = 50 * 60;
      else if (77 <= distance && distance < 79) targtot = 55 * 60;
      else if (80 <= distance && distance < 99) targtot = 60 * 60;
      else targtot = 0;
  
      if (prev3tot != prev4tot) {
        dispTime(prev3tot);
      }
      delay(24);
    }
  }
  Serial.println("TimerINIT");
  dt.init(prev3tot);
  Serial.println(timerRunning);
}

void setup() {
  Serial.begin(115200);
  gfx->begin();
}

void loop() {
  setTimer();
  while(timerRunning) {
    unsigned long cur_millis = millis();
    if (cur_millis >= dt.tar)
    { dt.inc();
      dt.graphicsUpdate();}
  }
  if (!timerCancelled){
    delay(1500);
  }
}
