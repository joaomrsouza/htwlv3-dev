#include "htwlv3.h"
#include "sclog.h"

#include <Adafruit_BME280.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#ifndef NODE_ID
#define NODE_ID 255
#endif

#define BTN_PIN 0 // Built-in button pin (USER_SW)

#define BME_INIT_AUTO_MODE false
#define NOTIFY_AUTO_MODE 0
#define NOTIFY_READ_TEMP 1

#define STATE_CHECK 0
#define STATE_SEND 1
#define STATE_RECEIVE 2
#define STATE_WAIT 3

#define SENSOR_READ_INTERVAL 10000 // milliseconds
#define LISTEN_TIMEOUT 10000       // milliseconds

typedef struct dataPacket
{
  int index;
  int nodeId;
  float temperature;
  unsigned long timestamp;
} SensorData;

SCLOG::LOG_LEVELS node1Levels[] = {SCLOG::INFO, SCLOG::WARN, SCLOG::DEBUG, SCLOG::ERROR, SCLOG::TRACE};
SCLOG::LOG_LEVELS node2Levels[] = {SCLOG::INFO, SCLOG::WARN, SCLOG::DEBUG, SCLOG::ERROR, SCLOG::TRACE};
SCLOG::LOG_LEVELS node3Levels[] = {SCLOG::INFO, SCLOG::WARN, SCLOG::DEBUG, SCLOG::ERROR, SCLOG::TRACE};

SCLOG::NodeLogConfig configs[] = {
    {1, node1Levels, 1},
    {2, node2Levels, 1},
    {3, node3Levels, 1}};

// SCLOG sc(NODE_ID, configs, 3);
SCLOG sc(0, configs, 3);

Adafruit_BME280 bme;
TwoWire BMEWire = TwoWire(1);

TaskHandle_t xTaskHandleButton = NULL;
TaskHandle_t xTaskHandleReadTemperature = NULL;
TaskHandle_t xTaskHandleLoraControl = NULL;

QueueHandle_t xQueueHandleSendWithLora = NULL;

int packetIndex = 0;
bool bmeAvailable = false;

void clearDisplay()
{
  Board.display->clearDisplay();
  Board.display->setCursor(0, 0);
}

void vTaskButton(void *pvParams)
{
  bool autoMode = BME_INIT_AUTO_MODE;

  while (true)
  {
    int pressTimes = 0; // -1 for long press

    while (true)
    {
      if (digitalRead(BTN_PIN) == LOW)
      {
        unsigned long initPressTime = millis();
        vTaskDelay(pdMS_TO_TICKS(10));

        while (digitalRead(BTN_PIN) == LOW)
          vTaskDelay(pdMS_TO_TICKS(10));

        unsigned long pressDuration = millis() - initPressTime;
        if (pressDuration >= 2000)
        {
          pressTimes = -1; // Long press
          break;
        }

        pressTimes++;
        unsigned long timeoutStart = millis();
        int timeoutDuration = 500; // 500 ms for next press
        bool pressedAgain = false;

        while (millis() - timeoutStart < timeoutDuration)
        {
          if (digitalRead(BTN_PIN) == LOW)
          {
            pressedAgain = true;
            break;
          }
          vTaskDelay(pdMS_TO_TICKS(10));
        }

        if (pressedAgain)
          continue;
        else
          break;
      }
      else
        break;
    }

    if (pressTimes != 0)
      clearDisplay();

    if (pressTimes == -1) // Long press
    {
      autoMode = !autoMode;
      xTaskNotify(xTaskHandleReadTemperature, NOTIFY_AUTO_MODE, eSetValueWithOverwrite);
      Board.println("BME: Auto mode " + String(autoMode ? "enabled" : "disabled"));
    }
    else if (pressTimes == 1) // Short press
    {
      if (autoMode)
        Board.println(String(NODE_ID) + ": -> " + String(NODE_ID - 1));
      else
      {
        Board.println("BME: Manual read requested");
        xTaskNotify(xTaskHandleReadTemperature, NOTIFY_READ_TEMP, eSetValueWithOverwrite);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void vTaskReadTemperature(void *pvParams)
{
  bool autoMode = BME_INIT_AUTO_MODE;
  bool readNow = false;
  uint32_t notification;

  while (true)
  {
    if (xTaskNotifyWait(0, ULONG_MAX, &notification, pdMS_TO_TICKS(100)) == pdTRUE)
    {
      if (notification == NOTIFY_AUTO_MODE)
      {
        autoMode = !autoMode;
        readNow = false;
      }
      else if (!autoMode && notification == NOTIFY_READ_TEMP)
        readNow = true;
    }

    if (!autoMode && !readNow)
      continue;

    readNow = false;

    SensorData data;

    float temperature = bmeAvailable ? bme.readTemperature() : random(200, 300) / 10.0;

    data.nodeId = NODE_ID;
    data.timestamp = millis();
    data.index = packetIndex++;
    data.temperature = temperature;

    // sc.log("BME: Read " + String(data.temperature) + " °C" + (bmeAvailable ? "" : " (Fake)"), sc.INFO);
    Board.println(String(NODE_ID) + ": BME: " + String(data.temperature) + " C" + (bmeAvailable ? "" : " Fake"));

    xQueueSend(xQueueHandleSendWithLora, &data, portMAX_DELAY);

    vTaskDelay(pdMS_TO_TICKS(SENSOR_READ_INTERVAL));
  }
}

void vTaskLoraControl(void *pvParams)
{
  int state = STATE_CHECK;
  uint32_t notification;

  while (true)
  {
    if (state == STATE_CHECK)
    {
      sc.log("Lora Control: CHECK", sc.TRACE);
      if (uxQueueMessagesWaiting(xQueueHandleSendWithLora) > 0)
        state = STATE_SEND;
      else
        state = STATE_RECEIVE;
    }

    if (state == STATE_SEND)
    {
      sc.log("Lora Control: SEND", sc.TRACE);
      SensorData lastData;
      JsonDocument lastDataDocument;

      unsigned int destId = NODE_ID - 1;

      lastDataDocument["destId"] = destId;
      lastDataDocument["nodeId"] = NODE_ID;

      JsonArray lastDataArray = lastDataDocument["data"].to<JsonArray>();

      while (xQueueReceive(xQueueHandleSendWithLora, &lastData, 0) == pdTRUE)
      {
        JsonDocument dataToSend;

        dataToSend["nodeId"] = lastData.nodeId;
        dataToSend["index"] = lastData.index;
        dataToSend["temperature"] = lastData.temperature;
        dataToSend["timestamp"] = lastData.timestamp;

        lastDataArray.add(dataToSend);
      }

      String dataString;
      serializeJson(lastDataDocument, dataString);

      if (destId > 0)
      {
        sc.log("-> " + String(destId), sc.INFO, sc.GREEN);
        Board.println(String(NODE_ID) + ": -> " + String(destId));

        for (JsonVariant jsonData : lastDataArray)
        {
          String str;
          serializeJson(jsonData, str);
          sc.log("  " + str, sc.INFO, sc.GREEN);
          Board.println(String(NODE_ID) + ":   " + String(jsonData["nodeId"].as<int>()) + "-" + String(jsonData["index"].as<int>()));
        }

        int res = Board.lora->sendReliablePacket(dataString.c_str(), destId); // TODO: Check for return and sleep if busy
        sc.log("Lora Control: SEND - " + String(res), sc.TRACE);
        if (res != 0)
          sc.log("Lora Control: NOT ABLE TO SEND! NOT IDLE", sc.ERROR);
        state = STATE_WAIT;
      }
      else
      {
        // sc.log("Final Data:", sc.INFO, sc.GREEN);
        Board.println(String(NODE_ID) + ": Final Data:");

        for (JsonVariant jsonData : lastDataArray)
        {
          String str;
          serializeJson(jsonData, str);
          sc.log("  " + str, sc.INFO, sc.GREEN);
          Board.println(String(NODE_ID) + ":   " + String(jsonData["nodeId"].as<int>()) + "-" + String(jsonData["index"].as<int>()));
        }

        state = STATE_CHECK;
      }
    }

    if (state == STATE_RECEIVE)
    {
      int res = Board.lora->listenToPacket(LISTEN_TIMEOUT);
      sc.log("Lora Control: RECEIVE - " + String(res), sc.TRACE);
      if (res != 0)
        sc.log("Lora Control: NOT ABLE TO LISTEN! NOT IDLE", sc.ERROR);
      state = STATE_WAIT;
    }

    if (state == STATE_WAIT)
    {
      sc.log("Lora Control: WAIT - " + String(Board.lora->getState()), sc.TRACE);
      Board.process();
    }

    if (xTaskNotifyWait(0, ULONG_MAX, &notification, pdMS_TO_TICKS(100)) == pdTRUE)
      state = notification;
  }
}

void cLoraOnReceive(LoraDataPacket packet)
{
  JsonDocument loraData;
  deserializeJson(loraData, packet.data);

  unsigned int srcId = loraData["nodeId"].as<unsigned int>();

  sc.log("<- " + String(srcId), sc.TRACE, sc.YELLOW);
  Board.println(String(NODE_ID) + ": <- " + String(srcId));

  JsonArray dataArray = loraData["data"].as<JsonArray>();

  for (JsonVariant jsonData : dataArray)
  {
    String str;
    serializeJson(jsonData, str);
    sc.log("  " + str, sc.TRACE, sc.YELLOW);
    Board.println(String(NODE_ID) + ":   " + String(jsonData["nodeId"].as<int>()) + "-" + String(jsonData["index"].as<int>()));

    SensorData data;

    data.index = jsonData["index"].as<int>();
    data.nodeId = jsonData["nodeId"].as<int>();
    data.temperature = jsonData["temperature"].as<float>();
    data.timestamp = jsonData["timestamp"].as<unsigned long>();

    xQueueSend(xQueueHandleSendWithLora, &data, portMAX_DELAY);
  }

  xTaskNotify(xTaskHandleLoraControl, STATE_CHECK, eSetValueWithOverwrite);
}

void cLoraOnReceiveTimeout()
{
  xTaskNotify(xTaskHandleLoraControl, STATE_CHECK, eSetValueWithOverwrite);
  sc.log("LORA: Receive timeout", sc.WARN);
}

void cLoraOnSendDone()
{
  xTaskNotify(xTaskHandleLoraControl, STATE_CHECK, eSetValueWithOverwrite);
  sc.log("LORA: Send done", sc.TRACE);
}

void cLoraOnSendTimeout()
{
  xTaskNotify(xTaskHandleLoraControl, STATE_SEND, eSetValueWithOverwrite);
  sc.log("LORA: Send timeout", sc.WARN);
}

void config()
{
  // === Board Config ===

  HTWLV3Config boardConfig = HTWLV3::getDefaultConfig();

  boardConfig.serialEnable = true;
  boardConfig.displayEnable = true;
  boardConfig.loraEnable = true;
  // boardConfig.wifiEnable = true;

  Board.setConfig(boardConfig);

  // === LoRa Config ===

  HTLORAV3Config loraConfig = HTLORAV3::getDefaultConfig();

  loraConfig.frequency = 915E6;
  loraConfig.txOutPower = -3;
  loraConfig.spreadingFactor = 8;

  Board.lora->setConfig(loraConfig);

  Board.lora->setOnReceive(cLoraOnReceive);
  Board.lora->setOnReceiveTimeout(cLoraOnReceiveTimeout);
  Board.lora->setOnSendDone(cLoraOnSendDone);
  Board.lora->setOnSendTimeout(cLoraOnSendTimeout);

  // === WiFi Config ===

  // HTWIFIV3Config wifiConfig = HTWIFIV3::getDefaultConfig();

  // wifiConfig.clientEnable = true;

  // Board.wifi->setConfig(wifiConfig);

  // === Client Config ===

  // HTWIFIV3ClientConfig clientConfig = HTWIFIV3Client::getDefaultConfig();

  // clientConfig.ssid = CLIENT_SSID;
  // clientConfig.password = CLIENT_PASSWORD;

  // Board.wifi->client->setConfig(clientConfig);
}

void setup()
{
  config();
  Board.begin(NODE_ID);

  BMEWire.begin(SDA, SCL);

  if (bme.begin(0x76, &BMEWire))
  {
    sc.log("SETUP: BME280 init", sc.INFO);
    Board.println("SETUP: BME280 init");
    bmeAvailable = true;
  }
  else
  {
    sc.log("SETUP: BME280 not init", sc.WARN);
    Board.println("SETUP: BME280 not init");
  }

  sc.log("LOG: DEBUG ENABLED", SCLOG::DEBUG);
  sc.log("LOG: INFO ENABLED", SCLOG::INFO);
  sc.log("LOG: TRACE ENABLED", SCLOG::TRACE);
  sc.log("LOG: WARN ENABLED", SCLOG::WARN);
  sc.log("LOG: ERROR ENABLED", SCLOG::ERROR);

  sc.log("SETUP: Complete", sc.INFO);
  Board.println("SETUP: Complete");

  xQueueHandleSendWithLora = xQueueCreate(10, sizeof(SensorData));

  xTaskCreate(vTaskButton, "Button Task", configMINIMAL_STACK_SIZE + 1024, NULL, configMAX_PRIORITIES - 10, &xTaskHandleButton);
  xTaskCreate(vTaskReadTemperature, "Read Temperature Task: ", configMINIMAL_STACK_SIZE + 1024, NULL, configMAX_PRIORITIES - 5, &xTaskHandleReadTemperature);
  xTaskCreate(vTaskLoraControl, "Lora Control Task: ", configMINIMAL_STACK_SIZE + (1024 * 4), NULL, configMAX_PRIORITIES - 1, &xTaskHandleLoraControl);
}

void loop()
{
}
