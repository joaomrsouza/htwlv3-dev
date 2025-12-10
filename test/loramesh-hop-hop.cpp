#include <Arduino.h>
#include "loramesher.h"

#include "htwlv3.h"

#include <Adafruit_BME280.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#ifndef NODE_ID
#define NODE_ID 250
#endif

#ifndef ORIGIN
#define ORIGIN "ESP-X"
#endif

#define RADIO_DIO_1 14
#define RADIO_NSS 8
#define RADIO_RESET 12
#define RADIO_BUSY 13

#define BTN_PIN 0 // Built-in button pin (USER_SW)

#define NODE_DELAY 700 * NODE_ID * 2

#define SENSOR_READ_INTERVAL 5000 // milliseconds
#define SEND_INTERVAL 5000        // milliseconds

#define FORCE_SEND_TIMEOUT 5000 // milliseconds

#define DEST_ADDR 0x8088

#define MAX_PAYLOAD_SIZE 256

#define BME_INIT_AUTO_MODE true
#define NOTIFY_AUTO_MODE 0
#define NOTIFY_READ_TEMP 1

uint8_t buf[MAX_PAYLOAD_SIZE];

LoraMesher &radio = LoraMesher::getInstance();

uint16_t LOCAL_ADDR = radio.getLocalAddress();

TaskHandle_t xTaskHandleButton = NULL;
TaskHandle_t xTaskHandleReadTemperature = NULL;
TaskHandle_t vTaskHandleSendLoRaMessage = NULL;
TaskHandle_t xTaskHandleReceiveLoRaMessage = NULL;

QueueHandle_t xQueueHandleSendWithLora = NULL;
QueueHandle_t xQueueHandleReceivedFromLora = NULL;

bool bmeAvailable = false;
bool directSend = false;
int packetIndex = 0;
int lastReceivedIndex[10] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
typedef struct dataPacket
{
  int index;
  int nodeId;
  float temperature;
  unsigned long timestamp;
} SensorData;

Adafruit_BME280 bme;
TwoWire BMEWire = TwoWire(1);

uint16_t getSendAddr()
{
  // Path 0x5CEC -> 0xD510 -> 0x8088 -> Print out
  return directSend ? (LOCAL_ADDR == DEST_ADDR ? NULL : DEST_ADDR) : (LOCAL_ADDR == 0x5CEC ? 0xD510 : (LOCAL_ADDR == 0xD510 ? DEST_ADDR : NULL));
}

int getDelay(int baseDelay)
{
  return baseDelay + NODE_DELAY + random(0, baseDelay / 2); // random(300, 1500)
}

void clearDisplay()
{
  Board.display->clearDisplay();
  Board.display->setCursor(0, 0);
}

void config()
{
  HTWLV3Config boardConfig = HTWLV3::getDefaultConfig();

  boardConfig.serialEnable = true;
  boardConfig.displayEnable = true;

  Board.setConfig(boardConfig);
}

void initRadio()
{
  LoraMesher::LoraMesherConfig config = LoraMesher::LoraMesherConfig();

  config.loraCs = RADIO_NSS;    // LoRa chip select pin
  config.loraIrq = RADIO_DIO_1; // LoRa IRQ pin
  config.loraRst = RADIO_RESET; // LoRa reset pin
  config.loraIo1 = RADIO_BUSY;  // LoRa DIO1 pin
  config.module = LoraMesher::SX1262_MOD;
  config.freq = 433.000F;
  config.power = 22;
  // config.freq = 915.000F;
  // config.sf = 8;
  // config.power = 2;

  radio.begin(config);
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
        Board.println(String(ORIGIN) + " - " + String(LOCAL_ADDR, HEX));
      else
      {
        Board.println("BME: Manual read requested");
        xTaskNotify(xTaskHandleReadTemperature, NOTIFY_READ_TEMP, eSetValueWithOverwrite);
      }
    }
    else if (pressTimes == 2)
    {
      directSend = !directSend;
      Board.println("Direct send " + String(directSend ? "enabled" : "disabled"));
      Board.println(String(ORIGIN) + " - " + String(LOCAL_ADDR, HEX) + " -> " + String(getSendAddr(), HEX));
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

    data.temperature = bmeAvailable ? bme.readTemperature() : random(200, 300) / 10.0;
    data.nodeId = NODE_ID;
    data.timestamp = millis();
    data.index = packetIndex++;

    // Board.println("BME: Read " + String(data.temperature) + " °C");

    xQueueSend(xQueueHandleSendWithLora, &data, portMAX_DELAY);

    vTaskDelay(pdMS_TO_TICKS(getDelay(SENSOR_READ_INTERVAL)));
  }
}

void debugPrintRoutingTable()
{
  // Set the routing table list that is being used and cannot be accessed (Remember to release use after usage)
  LM_LinkedList<RouteNode> *routingTableList = radio.routingTableListCopy();

  routingTableList->setInUse();

  char text[30];
  for (int i = 0; i < radio.routingTableSize(); i++)
  {
    RouteNode *rNode = (*routingTableList)[i];
    NetworkNode node = rNode->networkNode;
    snprintf(text, 30, ("DEBUG RT |%X(%d)->%X|"), node.address, node.metric, rNode->via);
    Board.println(String(text));
  }

  // Release routing table list usage.
  routingTableList->releaseInUse();

  // Delete the routing table list
  delete routingTableList;
}

void vTaskSendLoRaMessage(void *pvParams)
{
  JsonDocument lastDataDocument;
  SensorData data;
  unsigned long lastSendTime = 0;

  while (true)
  {
    JsonDocument dataDocumentArray;
    JsonArray dataArray = dataDocumentArray.to<JsonArray>();

    bool receivedQueueSizeGt0 = uxQueueMessagesWaiting(xQueueHandleReceivedFromLora) > 0;
    bool sendQueueSizeGt0 = uxQueueMessagesWaiting(xQueueHandleSendWithLora) > 0;
    bool timeoutExceeded = (millis() - lastSendTime) >= FORCE_SEND_TIMEOUT;

    // Se ambas tiverem, envia. Se só uma tiver e bater timeout, envia.
    if (receivedQueueSizeGt0 && sendQueueSizeGt0 || receivedQueueSizeGt0 && timeoutExceeded || sendQueueSizeGt0 && timeoutExceeded)
    {
      // debugPrintRoutingTable();

      while (xQueueReceive(xQueueHandleReceivedFromLora, &buf, 0) == pdTRUE)
      {
        deserializeJson(lastDataDocument, (char *)buf);
        JsonArray lastDataDocumentArray = lastDataDocument.as<JsonArray>();

        clearDisplay();

        for (JsonObject dataObj : lastDataDocumentArray)
        {
          dataArray.add(dataObj);

          int dataObjIndex = dataObj["index"].as<int>();
          int dataObjNodeId = dataObj["nodeId"].as<int>();
          int lastIndex = lastReceivedIndex[dataObjNodeId];

          if (lastIndex != -1 && dataObjIndex > (lastIndex + 1))
            Board.println("WARNING: Lost packets from Node " + String(dataObjNodeId) + ". Last index: " + String(lastIndex) + ", Current index: " + String(dataObjIndex));

          lastReceivedIndex[dataObjNodeId] = dataObjIndex;
        }
      }

      while (xQueueReceive(xQueueHandleSendWithLora, &data, 0) == pdTRUE)
      {
        JsonDocument dataDocument;

        dataDocument["nodeId"] = data.nodeId;
        dataDocument["index"] = data.index;
        dataDocument["timestamp"] = data.timestamp;
        dataDocument["temperature"] = data.temperature;

        dataArray.add(dataDocument);
      }

      String dataStrArray;
      serializeJson(dataDocumentArray, dataStrArray);
      lastDataDocument.set(NULL);

      if (dataArray.size() > 0)
      {
        lastSendTime = millis();
        uint16_t sendAddr = getSendAddr();
        if (sendAddr != NULL)
        {
          Board.println(String(LOCAL_ADDR, HEX) + " -> " + String(sendAddr, HEX) + ": " + String(dataStrArray));
          radio.sendReliablePacket(sendAddr, (uint8_t *)dataStrArray.c_str(), dataStrArray.length());
        }
        else
        {
          // Board.print("Final Data: ");
          Board.println("[");
          for (JsonObject dataObj : dataArray)
          {
            String singleDataStr;
            serializeJson(dataObj, singleDataStr);
            Board.println("  " + singleDataStr);
          }
          Board.println("]");
        }
      }
    }

    vTaskDelay(getDelay(SEND_INTERVAL) / portTICK_PERIOD_MS);
  }
}

void vTaskReceiveLoRaMessage(void *)
{
  while (true)
  {
    /* Wait for the notification of vTaskReceiveLoRaMessage and enter blocking */
    ulTaskNotifyTake(pdPASS, portMAX_DELAY);

    // Iterate through all the packets inside the Received User Packets FiFo
    while (radio.getReceivedQueueSize() > 0)
    {
      // Board.println(String(LOCAL_ADDR, HEX) + ": Queue size: " + radio.getReceivedQueueSize());

      // Get the first element inside the Received User Packets FiFo
      AppPacket<uint8_t> *packet = radio.getNextAppPacket<uint8_t>();

      // Board.println(String(LOCAL_ADDR, HEX) + ": <- " + String(packet->src, HEX) + " (Size: " + packet->payloadSize + ")");

      // Get the payload (string JSON como array de bytes)
      uint8_t *dPacket = (uint8_t *)packet->payload;
      size_t payloadLength = packet->getPayloadLength();

      size_t copySize = payloadLength < MAX_PAYLOAD_SIZE ? payloadLength : MAX_PAYLOAD_SIZE - 1;
      memcpy(buf, dPacket, copySize);
      buf[copySize] = (uint8_t)'\0';                                               // Adicionar null terminator para garantir string válida
      Board.println(String(LOCAL_ADDR, HEX) + ": <- " + String(packet->src, HEX)); //  + ": " + String((char *)buf)

      // Enviar o buffer completo para a fila
      xQueueSend(xQueueHandleReceivedFromLora, &buf, portMAX_DELAY);

      // Delete the packet when used. It is very important to call this function to release the memory of the packet.
      radio.deletePacket(packet);
    }
  }
}

void setup()
{
  config();
  Board.begin();
  BMEWire.begin(SDA, SCL);

  if (bme.begin(0x76, &BMEWire))
  {
    bmeAvailable = true;
    Board.println("SETUP: BME280 init.");
  }
  else
    Board.println("SETUP: BME280 not init.");

  initRadio();

  Board.print(String(LOCAL_ADDR, HEX) + ": INIT (" + String(LOCAL_ADDR, HEX) + " -> " + String(getSendAddr(), HEX) + ")");

  xQueueHandleSendWithLora = xQueueCreate(10, sizeof(SensorData));
  xQueueHandleReceivedFromLora = xQueueCreate(10, sizeof(uint8_t[MAX_PAYLOAD_SIZE]));

  // Create the receive task and add it to the LoRaMesher
  xTaskCreate(vTaskButton, "Button Task", configMINIMAL_STACK_SIZE + 1024, NULL, 1, &xTaskHandleButton);
  xTaskCreate(vTaskReadTemperature, "Read Temperature Task", configMINIMAL_STACK_SIZE + 1024, NULL, 1, &xTaskHandleReadTemperature);
  xTaskCreate(vTaskReceiveLoRaMessage, "Receive LoRa Message", configMINIMAL_STACK_SIZE + (1024 * 4), (void *)1, 2, &xTaskHandleReceiveLoRaMessage);
  xTaskCreate(vTaskSendLoRaMessage, "Send LoRa Message", configMINIMAL_STACK_SIZE + (1024 * 4), NULL, 1, &vTaskHandleSendLoRaMessage);

  radio.setReceiveAppDataTaskHandle(xTaskHandleReceiveLoRaMessage);

  // Start LoRaMesher
  radio.start();

  Board.println(String(LOCAL_ADDR, HEX) + ": Lora initialized");
  Board.println("SETUP: Completed");
}

void loop()
{
}