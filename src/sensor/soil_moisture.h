#define RS485Enable_pin 7
#define RS485Transmit HIGH
#define RS485Receive LOW

const int SlaveAddress = 0x01;
const int Modbusfunction = 0x03;                                                           // Read Multiple Holding Registers
byte requestHeader[] = {SlaveAddress, Modbusfunction, 0x00, 0x01, 0x00, 0x02, 0x95, 0xCB}; // Volumetric water content rawl AD Value(VWCRAWAD) Wet less dry More

float latestReadData = 0;

void asyncUpdateSoilSensor()
{
  digitalWrite(RS485Enable_pin, RS485Transmit);
  Serial2.write(requestHeader, sizeof(requestHeader));
  delay(10);
  digitalWrite(RS485Enable_pin, RS485Receive);
}

float readLatestSoilSensorData()
{
  delay(50);
  int count = 0, buffer[10];
  bool startRead = false;
  while (Serial2.available())
  {
    byte b = Serial2.read();
    if (b == 0xFF)
      startRead = true;
    if (startRead)
    {
      buffer[count] = b;
      count++;
    }
  }
  switch (count)
  {
  case 9:
    if (buffer[2] == 4)
      latestReadData = (float)((buffer[3] << 8) + buffer[4]) / 100;
    break;
  case 10:
    if (buffer[3] == 4)
      latestReadData = (float)((buffer[4] << 8) + buffer[5]) / 100;
    break;
  default:
    break;
  }
  return latestReadData;
}