#include "Rectangle.h"
#include <TouchScreen.h>
#include <Arduino.h>

Rectangle::Rectangle(void) {
  x = 0;
  y = 0;
  w = 0;
  h = 0;
  dCurrPer = dPrevPer = 0.50;
  dRate = 0.00;
  bChanged = false;
}

Rectangle::Rectangle(int px, int py, int pw, int ph) {
  x = px;
  y = py;
  w = pw;
  h = ph;
  dCurrPer = dPrevPer = 0.50;
  dRate = 0.00;
  bChanged = false;
}
Rectangle::Rectangle(int px, int py, int pw, int ph, double percent, double rate, bool change) {
  x = px;
  y = py;
  w = pw;
  h = ph;
  dCurrPer = dPrevPer = percent;
  dRate = rate;
  bChanged = change;
}
void Rectangle::init(int px, int py, int pw, int ph) {
  x = px;
  y = py;
  w = pw;
  h = ph;
  dCurrPer = dPrevPer = 0.50;
  dRate = 0.00;
  bChanged = false;
}
void Rectangle::init(int px, int py, int pw, int ph, double percent, double rate) {
  x = px;
  y = py;
  w = pw;
  h = ph;
  dCurrPer = dPrevPer = percent;
  dRate = rate;
  bChanged = false;
}
void Rectangle::ChangeRand() {
  /*This will change the value based on its rate.
      Will need to check to make sure it doesn't exceed 100% or go lower then 0%
  */
  dPrevPer = dCurrPer;
  long rando = random(100);
  if (dCurrPer * 100 <= 1) {
    dCurrPer += dRate;
  } else if (dCurrPer * 100 >= 100) {
    dCurrPer -= dRate;
  } else {
    if (rando >= 50) {
      //Positive
      dCurrPer += dRate;
    } else {
      //Negative
      dCurrPer -= dRate;
    }
  }
  bChanged = true;
}

void Rectangle::Decrease(){
  dPrevPer = dCurrPer;
  dCurrPer -= dRate;
  bChanged = true;
}
void Rectangle::Increase(){
  dPrevPer = dCurrPer;
  dCurrPer += dRate;
  bChanged = true;
}
void Rectangle::Increase(double dR){
  dPrevPer = dCurrPer;
  dCurrPer += dR;
  bChanged = true;
}

bool Rectangle::isPressed(TSPoint tp) {
  if (tp.x >= x && tp.y >= y) {
    if (tp.x <= x + w && tp.y <= y + h) {
      return true;
    }
  }
  return false;
}
