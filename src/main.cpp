#include <Arduino.h>
#include <TaskScheduler.h>
#include <Adafruit_AM2315.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <sensor/soil_moisture.h>
#include <relay/relay_controller.h>
#include <LED/led.h>

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif
#if (SERIAL_RX_BUFFER_SIZE < 256)
#error("Buffer size limit is too short! , please fix HardwareSerial.h!");
#endif

#define Relay1 23
#define Relay2 27
#define Relay3 31
#define SSR1 39
#define SSR2 43

#define ONHUMID 0
#define ONTEMP 1
#define ONSOIL 2

#define RedLED_Pin 12
#define GreenLED_Pin 11
#define BlueLED_Pin 10

typedef struct
{
  float temp = 0, humid = 0, soilMoisture = 0;
} sensorData;

typedef struct
{
  String DevID = "", current_state = "not connected";
} connInfo;

int relayPins[] = {SSR1, SSR2, Relay1, Relay2, Relay3};
char json[200];
sensorData sensorVal;
connInfo conInfo;

Adafruit_AM2315 am2315;
RelayController rCtrl(5, 3, relayPins);
LED led(RedLED_Pin, GreenLED_Pin, BlueLED_Pin);
Adafruit_SSD1306 OLED(-1);

void updateAllSensorData();
void takeEffectOnRelay();
void updateToScreen();
void reportStatus();
void printToScreen(String text, int16_t y_pos);
void GenerateReport();

Scheduler runner;
Task updateSensorData(100, TASK_FOREVER, &updateAllSensorData);
Task relayCallback(100, TASK_FOREVER, &takeEffectOnRelay);
Task updateScreen(1000, TASK_FOREVER, &updateToScreen);
Task reportDeviceStatus(3000, TASK_FOREVER, &reportStatus);

void updateAllSensorData()
{
  asyncUpdateSoilSensor();
  sensorVal.humid = am2315.readHumidity();
  sensorVal.temp = am2315.readTemperature();
}

void takeEffectOnRelay()
{
  // Humidity : callback index = 0
  // Temperature : callback index = 1
  // ADC soil moisture : callback index = 2
  rCtrl.takeEffect(0, sensorVal.humid);
  rCtrl.takeEffect(1, sensorVal.temp);
  rCtrl.takeEffect(2, sensorVal.soilMoisture);
}

void updateToScreen()
{
  OLED.clearDisplay();
  printToScreen("Humidity = " + String(sensorVal.humid) + " %", 0);
  printToScreen("Temp = " + String(sensorVal.temp) + " C", 10);
  printToScreen("Soil = " + String(sensorVal.soilMoisture) + " %", 20);
  printToScreen("ID = " + conInfo.DevID, 30);
  printToScreen(conInfo.current_state + ".", 40);
  OLED.display();
}

void reportStatus()
{
  GenerateReport();
  Serial3.print(json);
  Serial3.print('\r');
}

void printToScreen(String text, int16_t y_pos)
{
  OLED.setCursor(0, y_pos);
  OLED.print(text);
}

bool onOffToBoolean(String onoff)
{
  return onoff == "on";
}

void GenerateReport()
{
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  root["t"] = "data";
  JsonObject &data = root.createNestedObject("data");
  for (int i = 0; i < 5; i++)
  {
    data["Relay" + String(i + 1)] = (rCtrl.getRelayState(i) == 1) ? "on" : "off";
  }
  // NAN
  data["Soil"] = sensorVal.soilMoisture == NAN ? 0 : sensorVal.soilMoisture;
  data["Temp"] = sensorVal.temp == NAN ? 0 : sensorVal.temp;
  data["Humidity"] = sensorVal.humid == NAN ? 0 : sensorVal.humid;
  root.printTo(json);
}

void setup()
{
  Serial.begin(9600);
  Serial2.begin(9600);
  Serial3.begin(115200);
  am2315.begin();
  OLED.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  OLED.clearDisplay();
  OLED.setTextColor(WHITE);
  OLED.setTextSize(1);
  pinMode(RS485Enable_pin, OUTPUT);

  runner.init();
  runner.addTask(updateSensorData);
  runner.addTask(updateScreen);
  runner.addTask(relayCallback);
  runner.addTask(reportDeviceStatus);
  updateSensorData.enable();
  relayCallback.enable();
  updateScreen.enable();
  reportDeviceStatus.enable();

  led.red();
  Serial.print("Setup complete");
}

void loop()
{
  runner.execute();
}

void serialEvent2()
{
  sensorVal.soilMoisture = readLatestSoilSensorData();
}

void serialEvent3() // Read Data from ESP8266 WiFi
{
  while (Serial3.available())
  {
    char receivedMessage[500];
    StaticJsonBuffer<500> jsonBuffer;
    Serial3.readBytesUntil('\r', receivedMessage, 490); //read the chars until we see a <CR>
    JsonObject &root = jsonBuffer.parseObject(receivedMessage);
    String cmd = root["cmd"].as<String>();
    Serial.println(cmd);
    if (cmd == "set")
    {
      for (int relayIndex = 1; relayIndex <= 5; relayIndex++)
      {
        int realRelayIndex = relayIndex - 1;
        if (root["state"]["Relay" + String(relayIndex)]["mode"].as<String>() == "manual")
        {
          rCtrl.clearEffect(realRelayIndex);
          rCtrl.setRelay(
              realRelayIndex,
              onOffToBoolean(
                  root
                      ["state"]
                      ["Relay" + String(relayIndex)]
                      ["detail"]
                          .as<String>()));
        }
        else if (root["state"]["Relay" + String(relayIndex)]["mode"].as<String>() == "auto")
        {
          // rCtrl.clearEffect(realRelayIndex);
          String sensorName = root["state"]["Relay" + String(relayIndex)]["detail"]["sensor"].as<String>();
          int symbol = ((const char *)root["state"]["Relay" + String(relayIndex)]["detail"]["symbol"])[4];
          int bound = root["state"]["Relay" + String(relayIndex)]["detail"]["trigger"].as<int>();
          rCtrl.setEffect(
              sensorName == "humidity" ? ONHUMID : sensorName == "temp" ? ONTEMP : ONSOIL,
              symbol,
              bound,
              realRelayIndex);
        }
      }
    }
    if (cmd == "fetch" || cmd == "set")
    {
      reportStatus();
    }
    else if (cmd == "display_connection_info")
    {
      conInfo.DevID = root["DevID"].as<String>();
    }
    else if (cmd == "indicate_connection_status")
    {
      conInfo.current_state = root["status"].as<String>();
      if (conInfo.current_state == "failure")
        led.red();
      else if (conInfo.current_state == "internet_connected")
        led.blue();
      else if (conInfo.current_state == "connected")
        led.green();
    }
  }
}