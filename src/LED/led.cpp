#include "led.h"

LED::LED(int redPin, int greenPin, int bluePin)
{
  this->redPin = redPin;
  this->greenPin = greenPin;
  this->bluePin = bluePin;
  pinMode(this->redPin, OUTPUT);
  pinMode(this->greenPin, OUTPUT);
  pinMode(this->bluePin, OUTPUT);
}

LED::~LED() {}

void LED::red()
{
  digitalWrite(this->redPin, ON_LED);
  digitalWrite(this->greenPin, OFF_LED);
  digitalWrite(this->bluePin, OFF_LED);
}

void LED::green()
{
  digitalWrite(this->redPin, OFF_LED);
  digitalWrite(this->greenPin, ON_LED);
  digitalWrite(this->bluePin, OFF_LED);
}

void LED::blue()
{
  digitalWrite(this->redPin, OFF_LED);
  digitalWrite(this->greenPin, OFF_LED);
  digitalWrite(this->bluePin, ON_LED);
}