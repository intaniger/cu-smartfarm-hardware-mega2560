#include "relay_controller.h"

RelayController::RelayController(int relayNo, int paramNo, int relayPins[])
{
  this->relayNo = relayNo;
  this->paramNo = paramNo;

  this->relayPins = new int[relayNo];
  for (int i = 0; i < relayNo; i++)
  {
    this->relayPins[i] = relayPins[i];
    pinMode(this->relayPins[i], OUTPUT);
    digitalWrite(this->relayPins[i], LOW);
  }

  this->callbackTable = new int **[paramNo]();
  for (int i = 0; i < paramNo; i++)
  {
    this->callbackTable[i] = new int *[relayNo]();
    for (int j = 0; j < relayNo; j++)
    {
      this->callbackTable[i][j] = new int[3]();
    }
  }
}

RelayController::~RelayController() {}

void RelayController::setEffect(int paramIndex, int symbol, int bound, int effectRelayIndex)
{
  // char output[100];
  // sprintf(output, "setEffect(paramIndex = %d, symbol = %d, bound = %d, effectRelayIndex = %d)", paramIndex, symbol, bound, effectRelayIndex);
  // Serial.print(output);
  this->clearEffect(effectRelayIndex);
  for (int effectIndex = 0; effectIndex < this->relayNo; effectIndex++)
  {
    if (this->callbackTable[paramIndex][effectIndex][0] == 0)
    {
      this->callbackTable[paramIndex][effectIndex][0] = symbol;
      this->callbackTable[paramIndex][effectIndex][1] = bound;
      this->callbackTable[paramIndex][effectIndex][2] = effectRelayIndex;
      return;
    }
  }
}

void RelayController::takeEffect(int paramIndex, int value)
{
  for (int effectIndex = 0; effectIndex < 5; effectIndex++)
  {
    // [/*symbol*/, /*bound*/, /*effect relay*/]
    // symbol 99 represented 'less than'
    // symbol 101 represented 'more than'
    int symbol = this->callbackTable[paramIndex][effectIndex][0];
    int bound = this->callbackTable[paramIndex][effectIndex][1];
    int effectRelay = this->callbackTable[paramIndex][effectIndex][2];
    if (symbol != 0 || bound != 0)
    {
      // Serial.print(symbol);
      // Serial.print(",");
      // Serial.print(bound);
      // Serial.print(",");
      // Serial.print(effectRelay);
      // Serial.println();
      switch (symbol)
      {
      case 99:
        this->setRelay(effectRelay, (value < bound));
        break;
      case 101:
        this->setRelay(effectRelay, (value > bound));
        break;
      }
    }
  }
}

void RelayController::clearEffect(int relayIndex)
{
  for (int i = 0; i < this->paramNo; i++)
  {
    for (int j = 0; j < this->relayNo; j++)
    {
      if (this->callbackTable[i][j][2] == relayIndex)
        this->callbackTable[i][j][0] = 0;
    }
  }
}

void RelayController::setRelay(int relayIndex, int state)
{
  Serial.println("Set " + String(relayIndex) + "@" + String(this->relayPins[relayIndex]) + ":" + String(state));
  digitalWrite(this->relayPins[relayIndex], state);
}

int RelayController::getRelayState(int relayIndex)
{
  return digitalRead(this->relayPins[relayIndex]);
}