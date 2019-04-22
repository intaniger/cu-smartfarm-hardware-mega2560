#include <Arduino.h>

class RelayController
{
private:
  int relayNo, paramNo;
  int *relayPins;
  int ***callbackTable;

public:
  RelayController(int relayNo, int paramNo, int relayPins[]);
  ~RelayController();

  void setRelay(int relayIndex, int state);
  int getRelayState(int relayIndex);

  void setEffect(int paramIndex, int symbol, int bound, int effectRelayIndex);
  void clearEffect(int relayIndex);
  void takeEffect(int paramIndex, int value);
};