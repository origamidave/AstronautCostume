#ifndef Rectangle_h
#define Rectangle_h

#include <TouchScreen.h>

class Rectangle {

public:
  Rectangle(void);
  Rectangle(int px, int py, int pw, int ph);
  Rectangle(int px, int py, int pw, int ph, double percent, double rate, bool change);
  int x;
  int y;
  int w;
  int h;

  bool bChanged;

  double dCurrPer;  //The current Percentage
  double dPrevPer;  //The previous Percentage
  double dRate;

  bool isPressed(TSPoint tp);

  void init(int px, int py, int pw, int ph);
  void init(int px, int py, int pw, int ph, double percent, double rate);

  void ChangeRand();
  void Decrease();
  void Increase();
  void Increase(double dR);
};




#endif
