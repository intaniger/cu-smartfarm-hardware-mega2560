#include <Arduino.h>

#define ON_LED LOW
#define OFF_LED HIGH

class LED
{
private:
  int redPin, greenPin, bluePin;

public:
  LED(int redPin, int greenPin, int bluePin);
  ~LED();
  void green();
  void red();
  void blue();
};