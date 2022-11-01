#include <Elegoo_GFX.h>     // Core graphics library
#include <Elegoo_TFTLCD.h>  // Hardware-specific library
#include <TouchScreen.h>
#include <Servo.h>
#include <Adafruit_NeoPixel.h>
#include "Rectangle.h"

#if defined(__SAM3X8E__)
#undef __FlashStringHelper::F(string_literal)
#define F(string_literal) string_literal
#endif

//Define Colors (16-bit colors)
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF
#define GRAY 0x94B2
#define ORANGE 0xFD20

/* Pin Out
  5V
  GND
  LCD_RD	A0
  LCD_WR	A1
  LCD_RS	A2
  LCD_CS	A3
  LCD_RST	A4
  LCD_D2	D2
  LCD_D3	D3
  LCD_D4	D4
  LCD_D5	D5
  LCD_D6	D6
  LCD_D7	D7
  LCD_D0	D8
  LCD_D1	D9
*/

//Define Pins
#define LIGHT A5       //Using A5 for the light.
#define YP A3          //must be an analog pin, use "An" notation!
#define XM A2          //must be an analog pin, use "An" notation!
#define YM 9           //can be a digital pin
#define XP 8           //can be a digital pin
#define LCD_CS A3      //
#define LCD_CD A2      //
#define LCD_WR A1      //
#define LCD_RD A0      //
#define LCD_RESET A4   //
#define SVf 10         //Servo pin for Fan
#define SVc 11         //Servo pin for Candy
#define TNK 12         //Pin for the Tanks
#define JET 13         //Pin for JET Lights
#define nLEDt 49       //Number of LEDs for Tanks
#define nLEDj 41       //Number of LEDs for Jets
#define SRVDLY 105     //Detatch time delay
#define SRVFANDLY 150  //Detatch time delay

//X and & range from  testing of Touch Screen
#define Xmin 909      //Emperically determined min X val
#define Xmax 114      //Emperically determined max X val
#define Ymin 80       //Emperically determined min Y val
#define Ymax 915      //Emperically determined max Y val
#define Zmin 512      //Emperically determined min Z val
#define Zmax 315      //Emperically determined max Z val
#define MINPRESS 30   //Minimum pressure
#define MAXPRESS 200  //Maximum pressure (really weeds out in valid touches.)
#define COOLDOWN 200  //Controls 'multiple touches' within this time frame (ms)
#define PRESSDLY 50   //This controls the delay to wait while touchscreen is pressed.
#define LEDDIS 150    //Discharge rate
#define LEDCHG 120    //Charge rate
#define JETCNG 1      //Change rate of the Jets LEDs
#define LEDMAX 200    //Max brightness of LEDs
#define FANMAX 1900   //Max Fan Speed
#define FANMIN 1000   //Min Fan Speed

//Variables
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);             //300 is pressure resistance
Elegoo_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);  //0x9341 LCD driver

Servo svCandy;
Servo svFan;

Adafruit_NeoPixel Tanks(49, TNK, NEO_GRB + NEO_KHZ800);      //24 LEDs before bend 25 LEDs after.
Adafruit_NeoPixel JetLights(49, JET, NEO_GRB + NEO_KHZ800);  //41 LEDs.

uint32_t cRed = Tanks.Color(255, 0, 0);
uint32_t cGold = Tanks.Color(255, 215, 0);
uint32_t cOrange = Tanks.Color(255, 165, 0);
uint32_t cWhite = Tanks.Color(255, 255, 255);
uint32_t cOFF = Tanks.Color(0, 0, 0);

int iJetsCurr = 0;  //Controls flicker of Jets lights.
int iJetsPrev = 0;

uint16_t identifier = 0x9341;      //Pulled from example tftpaint.ino
bool bServoButtonPressed = false;  //
TSPoint p;                         //
long lCoolDown = 0.00;             //This will handle touch screen cooldown and prevent back to back input
long lJetDelay = 0.00;
long lLEDDelay = 0.00;

//int iLH2Pos = 0;
//int iLOXPos = 0;
//int iLEDPos = 0;

bool bJetsPushed = false;  //
bool bCandyPushed = false;
bool bLightsPushed = false;

//Define Buttons
Rectangle bntJetsInn(29, 224, 52, 82);   //Inner Jets button
Rectangle bntJetsOut(4, 214, 103, 103);  //Outter Jets button
Rectangle btnCandy(110, 214, 72, 103);   //Candy button
Rectangle btnLights(186, 214, 50, 103);  //Lights button

//Define Meters
Rectangle mtrOXY(42, 8, 190, 12, 0.60, 0.008, true);
Rectangle mtrCO2(42, 28, 190, 12, 0.20, 0.003, true);
Rectangle mtrRAD(42, 48, 190, 12, 0.80, 0.01, true);
Rectangle mtrH2O(42, 68, 190, 12, 0.75, 0.003, true);
Rectangle mtrLH2(42, 170, 190, 12, 1.00, 0.005, true);
Rectangle mtrLOX(42, 190, 190, 12, 1.00, 0.005, true);

//Functions
TSPoint formatPoint() {
  //This will take a reference to a point and format its x and y vals.
  TSPoint temp = ts.getPoint();
  temp.x = map(temp.x, Xmin, Xmax, 0, 240);
  temp.y = map(temp.y, Ymin, Ymax, 0, 320);
  if (temp.z > 0) {
    temp.z = map(temp.z, Zmin, Zmax, 0, 100);
  } else {
    temp.z = 0;
  }
  //Need to set pinMode after getting points for some reason.
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  return temp;
}

//Draw functions

//
void DrawMeters() {
  tft.setTextColor(WHITE);
  tft.setTextSize(2);

  if (mtrOXY.bChanged) {
    if (mtrOXY.dCurrPer < mtrOXY.dPrevPer) {
      tft.fillRect((mtrOXY.x + (mtrOXY.w * mtrOXY.dPrevPer) - (mtrOXY.w * (mtrOXY.dPrevPer - mtrOXY.dCurrPer))), mtrOXY.y, (mtrOXY.w * (mtrOXY.dPrevPer - mtrOXY.dCurrPer)) + 1, mtrOXY.h, BLACK);
    }
    if (mtrOXY.dCurrPer > mtrOXY.dPrevPer) {
      tft.fillRect(mtrOXY.x + (mtrOXY.w * mtrOXY.dPrevPer), mtrOXY.y, (mtrOXY.w * (mtrOXY.dCurrPer - mtrOXY.dPrevPer)) + 1, mtrOXY.h, RED);
    }
    if (mtrOXY.dCurrPer == mtrOXY.dPrevPer) {
      tft.fillRect(mtrOXY.x, mtrOXY.y, mtrOXY.w * mtrOXY.dCurrPer, mtrOXY.h, RED);
    }
    tft.setCursor(4, 7);
    tft.print("OXY");
    mtrOXY.bChanged = false;
  }

  if (mtrCO2.bChanged) {
    if (mtrCO2.dCurrPer < mtrCO2.dPrevPer) {
      tft.fillRect((mtrCO2.x + (mtrCO2.w * mtrCO2.dPrevPer) - (mtrCO2.w * (mtrCO2.dPrevPer - mtrCO2.dCurrPer))), mtrCO2.y, (mtrCO2.w * (mtrCO2.dPrevPer - mtrCO2.dCurrPer)) + 1, mtrCO2.h, BLACK);
    }
    if (mtrCO2.dCurrPer > mtrCO2.dPrevPer) {
      tft.fillRect(mtrCO2.x + (mtrCO2.w * mtrCO2.dPrevPer), mtrCO2.y, (mtrCO2.w * (mtrCO2.dCurrPer - mtrCO2.dPrevPer)) + 1, mtrCO2.h, GRAY);
    }
    if (mtrCO2.dCurrPer == mtrCO2.dPrevPer) {
      tft.fillRect(mtrCO2.x, mtrCO2.y, mtrCO2.w * mtrCO2.dCurrPer, mtrCO2.h, GRAY);
    }
    tft.setCursor(4, 27);
    tft.print("CO2");
    mtrCO2.bChanged = false;
  }

  if (mtrRAD.bChanged) {
    if (mtrRAD.dCurrPer < mtrRAD.dPrevPer) {
      tft.fillRect((mtrRAD.x + (mtrRAD.w * mtrRAD.dPrevPer) - (mtrRAD.w * (mtrRAD.dPrevPer - mtrRAD.dCurrPer))), mtrRAD.y, (mtrRAD.w * (mtrRAD.dPrevPer - mtrRAD.dCurrPer)) + 1, mtrRAD.h, BLACK);
    }
    if (mtrRAD.dCurrPer > mtrRAD.dPrevPer) {
      tft.fillRect(mtrRAD.x + (mtrRAD.w * mtrRAD.dPrevPer), mtrRAD.y, (mtrRAD.w * (mtrRAD.dCurrPer - mtrRAD.dPrevPer)) + 1, mtrRAD.h, GREEN);
    }
    if (mtrRAD.dCurrPer == mtrRAD.dPrevPer) {
      tft.fillRect(mtrRAD.x, mtrRAD.y, mtrRAD.w * mtrRAD.dCurrPer, mtrRAD.h, GREEN);
    }
    tft.setCursor(4, 47);
    tft.print("RAD");
    mtrRAD.bChanged = false;
  }

  if (mtrH2O.bChanged) {
    if (mtrH2O.dCurrPer < mtrH2O.dPrevPer) {
      tft.fillRect((mtrH2O.x + (mtrH2O.w * mtrH2O.dPrevPer) - (mtrH2O.w * (mtrH2O.dPrevPer - mtrH2O.dCurrPer))), mtrH2O.y, (mtrH2O.w * (mtrH2O.dPrevPer - mtrH2O.dCurrPer)) + 1, mtrH2O.h, BLACK);
    }
    if (mtrH2O.dCurrPer > mtrH2O.dPrevPer) {
      tft.fillRect(mtrH2O.x + (mtrH2O.w * mtrH2O.dPrevPer), mtrH2O.y, (mtrH2O.w * (mtrH2O.dCurrPer - mtrH2O.dPrevPer)) + 1, mtrH2O.h, BLUE);
    }
    if (mtrH2O.dCurrPer == mtrH2O.dPrevPer) {
      tft.fillRect(mtrH2O.x, mtrH2O.y, mtrH2O.w * mtrH2O.dCurrPer, mtrH2O.h, BLUE);
    }
    tft.setCursor(4, 67);
    tft.print("H2O");
    mtrH2O.bChanged = false;
  }

  if (mtrLH2.bChanged) {
    if (mtrLH2.dCurrPer < mtrLH2.dPrevPer) {
      tft.fillRect((mtrLH2.x + (mtrLH2.w * mtrLH2.dPrevPer) - (mtrLH2.w * (mtrLH2.dPrevPer - mtrLH2.dCurrPer))), mtrLH2.y, (mtrLH2.w * (mtrLH2.dPrevPer - mtrLH2.dCurrPer)) + 2, mtrLH2.h, BLACK);
    }
    if (mtrLH2.dCurrPer > mtrLH2.dPrevPer) {
      tft.fillRect(mtrLH2.x + (mtrLH2.w * mtrLH2.dPrevPer), mtrLH2.y, (mtrLH2.w * (mtrLH2.dCurrPer - mtrLH2.dPrevPer)) + 1, mtrLH2.h, RED);
    }
    if (mtrLH2.dCurrPer == mtrLH2.dPrevPer) {
      tft.fillRect(mtrLH2.x, mtrLH2.y, mtrLH2.w * mtrLH2.dCurrPer, mtrLH2.h, RED);
    }
    tft.setCursor(4, 168);
    tft.print("LH2");
    mtrLH2.bChanged = false;
  }

  if (mtrLOX.bChanged) {
    if (mtrLOX.dCurrPer < mtrLOX.dPrevPer) {
      tft.fillRect((mtrLOX.x + (mtrLOX.w * mtrLOX.dPrevPer) - (mtrLOX.w * (mtrLOX.dPrevPer - mtrLOX.dCurrPer))), mtrLOX.y, (mtrLOX.w * (mtrLOX.dPrevPer - mtrLOX.dCurrPer)) + 2, mtrLOX.h, BLACK);
    }
    if (mtrLOX.dCurrPer > mtrLOX.dPrevPer) {
      tft.fillRect(mtrLOX.x + (mtrLOX.w * mtrLOX.dPrevPer), mtrLOX.y, (mtrLOX.w * (mtrLOX.dCurrPer - mtrLOX.dPrevPer)) + 1, mtrLOX.h, BLUE);
    }
    if (mtrLOX.dCurrPer == mtrLOX.dPrevPer) {
      tft.fillRect(mtrLOX.x, mtrLOX.y, mtrLOX.w * mtrLOX.dCurrPer, mtrLOX.h, BLUE);
    }
    tft.setCursor(4, 188);
    tft.print("LOX");
    mtrLOX.bChanged = false;
  }
}
void CandyButton(bool Pushed) {
  if (Pushed) {
    //Its currently off, turn it on
    tft.fillRect(btnCandy.x, btnCandy.y, btnCandy.w, btnCandy.h, CYAN);
    tft.setTextColor(BLACK);
    bCandyPushed = false;
    svCandy.attach(SVc);
    svCandy.write(80);
    delay(SRVDLY);
    svCandy.detach();
  } else {
    //Its currently on, turn it off
    tft.fillRect(btnCandy.x, btnCandy.y, btnCandy.w, btnCandy.h, BLACK);
    tft.drawRect(btnCandy.x, btnCandy.y, btnCandy.w, btnCandy.h, CYAN);
    tft.setTextColor(CYAN);
    bCandyPushed = true;
    svCandy.attach(SVc);
    svCandy.write(0);
    delay(SRVDLY);
    svCandy.detach();
  }
  tft.setTextSize(2);
  tft.setCursor(238, 86);
  tft.setRotation(3);
  tft.print("CANDY");
  tft.setRotation(2);
}
void LightsButton(bool Pushed) {
  if (Pushed) {
    //Its currently off, turn it on
    tft.fillRect(btnLights.x, btnLights.y, btnLights.w, btnLights.h, YELLOW);
    tft.setTextColor(BLACK);
    digitalWrite(LIGHT, HIGH);
    bLightsPushed = false;
  } else {
    //Its currently on, turn it off
    tft.fillRect(btnLights.x, btnLights.y, btnLights.w, btnLights.h, BLACK);
    tft.drawRect(btnLights.x, btnLights.y, btnLights.w, btnLights.h, YELLOW);
    tft.setTextColor(YELLOW);
    digitalWrite(LIGHT, LOW);
    bLightsPushed = true;
  }
  tft.setTextSize(2);
  tft.setCursor(231, 22);
  tft.setRotation(3);
  tft.print("LIGHTS");
  tft.setRotation(2);
}
void JetsButton(bool TextOnly, bool Pushed) {
  if (!TextOnly) {
    int i = 9;
    while (i > 0) {
      if (i % 2 == 1) {
        tft.fillTriangle(4, 214, (i * 10) + 6, 214, 4, (i * 10) + 216, YELLOW);
      } else {
        tft.fillTriangle(4, 214, (i * 10) + 6, 214, 4, (i * 10) + 216, BLACK);
      }
      i--;
    }
    for (i = 0; i <= 10; i++) {
      if (i % 2 == 0) {
        tft.fillTriangle(106, 316, 106, 214 + (i * 10), 4 + (i * 10), 316, YELLOW);
      } else {
        tft.fillTriangle(106, 316, 106, 214 + (i * 10), 4 + (i * 10), 316, BLACK);
      }
    }
    tft.drawRect(bntJetsOut.x, bntJetsOut.y, bntJetsOut.w, bntJetsOut.h, RED);  //Outter border
  }
  if (Pushed) {
    //Its currently off, turn it on
    tft.fillRect(bntJetsInn.x, bntJetsInn.y, bntJetsInn.w, bntJetsInn.h, ORANGE);  //Inner Fill
    tft.drawRect(bntJetsInn.x, bntJetsInn.y, bntJetsInn.w, bntJetsInn.h, RED);     //Inner Border
    tft.setTextColor(BLACK);

    svFan.writeMicroseconds(FANMAX);
    JetLights.fill(cRed, 0, nLEDj);
    bJetsPushed = false;
  } else {
    //Its currently on, turn it off
    tft.fillRect(bntJetsInn.x, bntJetsInn.y, bntJetsInn.w, bntJetsInn.h, BLACK);  //Inner Fill
    tft.drawRect(bntJetsInn.x, bntJetsInn.y, bntJetsInn.w, bntJetsInn.h, RED);    //Inner Border
    tft.setTextColor(WHITE);

    svFan.writeMicroseconds(FANMIN);
    JetLights.fill(cOFF, 0, nLEDj);
    bJetsPushed = true;
  }
  tft.setTextSize(2);
  tft.setCursor(240, 176);
  tft.setRotation(3);
  tft.print("JETS");
  tft.setRotation(2);
}
void setLED(double dPercent) {
  int pos = nLEDt * dPercent;
  Tanks.fill(cOFF, 0, nLEDt);
  Tanks.fill(cRed, 0, pos);
}

//This will control the Jet LEDs if
void JetLEDs() {
  //if(!bJetsPushed){
  if ((millis() - lJetDelay) > JETCNG) {
    lJetDelay = millis();
    JetLights.setPixelColor(iJetsPrev, cRed);  //Set the old one to red
    iJetsPrev = iJetsCurr;                     //set old to current
    iJetsCurr = random(0, nLEDj);              //Pick a random LED
    //Serial.print("Setting JET led # ");
    //Serial.print(iJetsCurr);

    switch (random(0, 4)) {
      case 0:
        //Serial.println(" to Gold");
        JetLights.setPixelColor(iJetsCurr, cGold);
        break;
      case 1:
        //Serial.println(" to Orange");
        JetLights.setPixelColor(iJetsCurr, cOrange);
        break;
      case 2:
        //Serial.println(" to White");
        JetLights.setPixelColor(iJetsCurr, cWhite);
        break;
      case 3:
        //Serial.println(" to Orange");
        JetLights.setPixelColor(iJetsCurr, cOrange);
        break;
    }

    //Set to white.
  }
  //}
}

void attachFan() {
  svFan.attach(SVf, 1000, 2300);
}

void setup() {
  Serial.begin(9600);

  svCandy.attach(SVc);
  svCandy.write(0);
  delay(SRVDLY);
  svCandy.detach();

  attachFan();
  svFan.writeMicroseconds(1500);

  pinMode(LIGHT, OUTPUT);

  tft.reset();
  tft.begin(identifier);

  //If CON1 holes are on right:
  //  1 makes bottom right 0,0
  //  2 makes bottom left 0,0
  //  3 makes top left 0,0
  //  4 makes rop right 0,0
  tft.setRotation(2);

  //LEDs
  Tanks.begin();
  Tanks.clear();
  Tanks.setBrightness(LEDMAX);
  Tanks.fill(cRed, 0, nLEDt);
  Tanks.show();

  JetLights.begin();
  JetLights.clear();
  JetLights.setBrightness(LEDMAX);
  JetLights.fill(cOFF, 0, nLEDj);
  //JetLights.fill(cRed, 0, nLEDj);
  JetLights.show();

  //Draw UI
  tft.fillScreen(BLACK);
  JetsButton(false, false);
  CandyButton(false);
  LightsButton(false);
  DrawMeters();

  //Servo Arm delay
  delay(5000);
  svFan.writeMicroseconds(0);
}


void loop() {
  /*TODO:

  */

  //JetLEDs();
  //JetLights.show();

  p = formatPoint();
  if (p.z > MINPRESS && p.z < MAXPRESS && (millis() - lCoolDown) > COOLDOWN) {
    lCoolDown = millis();
    p = formatPoint();

    //Check to see if buttons are pushed
    if (bntJetsOut.isPressed(p)) {
      JetsButton(true, bJetsPushed);
    }
    if (btnCandy.isPressed(p)) {
      CandyButton(bCandyPushed);
    }
    if (btnLights.isPressed(p)) {
      LightsButton(bLightsPushed);
    }

    while (p.z > MINPRESS) {
      p = formatPoint();
      delay(PRESSDLY);
    }
  }
  //Check Jets
  if (!bJetsPushed) {
    if ((millis() - lLEDDelay) > LEDDIS) {
      lLEDDelay = millis();
      if (mtrLH2.dCurrPer >= 0.03) {
        mtrLH2.Decrease();
        mtrLOX.Decrease();
        setLED(mtrLH2.dCurrPer);
      } else {
        JetsButton(true, false);
        //Serial.println("CUTOFF");
      }
    }
    JetLEDs();
  } else {
    if ((millis() - lLEDDelay) > LEDCHG) {
      lLEDDelay = millis();
      if (mtrLH2.dCurrPer <= 1.00) {
        mtrLH2.Increase(mtrLH2.dRate * 2);
        mtrLOX.Increase(mtrLH2.dRate * 2);
        setLED(mtrLH2.dCurrPer);
      }
    }
  }
  Tanks.show();
  JetLights.show();

  //Change meters
  mtrOXY.ChangeRand();
  mtrCO2.ChangeRand();
  mtrRAD.ChangeRand();
  mtrH2O.ChangeRand();

  DrawMeters();

  //
}
