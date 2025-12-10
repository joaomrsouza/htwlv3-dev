// TODO: extract data from sensor and send time to time via lora with hop by hop accumulation

// Vou ter uma fila de leituras que dado um certo timeout, vai ser enviado via lora
// Se um pacote chegar antes do timeout, os dados são acumulados e enviados junto com o pacote para o próximo hop
// Se não houver pacote quando os dados chegarem, simplesmente repasse o pacote para o próximo hop

#include "htwlv3.h"

#include <SPI.h>
#include "RH_SX126x.h" // LoRa driver for SX1262
#include "RHMesh.h"    // The Mesh Layer

#include <Adafruit_BME280.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#define RADIO_NSS 8
#define RADIO_DIO_1 14
#define RADIO_BUSY 13
#define RADIO_RESET 12

#define LORA_SEND 1
#define LORA_RECEIVE 2
#define LORA_CHECK 3

#ifndef NODE_ID
#define NODE_ID 250
#endif

#define RECEIVE_TIMEOUT 5000

#define SENSOR_READ_INTERVAL 15000 + (700 * NODE_ID) // milliseconds

#define DIRECT_SEND true

RH_SX126x driver(RADIO_NSS, RADIO_DIO_1, RADIO_BUSY, RADIO_RESET);
RHMesh manager(driver, NODE_ID);

uint8_t buf[RH_SX126x_MAX_MESSAGE_LEN];

TaskHandle_t xTaskHandleReadTemperature = NULL;
TaskHandle_t xTaskHandleLoraControl = NULL;

QueueHandle_t xQueueHandleSendWithLora = NULL;
QueueHandle_t xQueueHandleReceivedFromLora = NULL;

bool bmeAvailable = false;

typedef struct
{
  int nodeId;
  float temperature;
  unsigned long timestamp;
} SensorData;

Adafruit_BME280 bme;
TwoWire BMEWire = TwoWire(1);

void config()
{
  HTWLV3Config boardConfig = HTWLV3::getDefaultConfig();

  boardConfig.serialEnable = true;
  boardConfig.displayEnable = true;

  Board.setConfig(boardConfig);
}

void vTaskReadTemperature(void *pvParams)
{
  while (true)
  {
    Board.println("BME: Reading...");

    SensorData data;

    data.temperature = bmeAvailable ? bme.readTemperature() : random(200, 300) / 10.0;
    data.nodeId = NODE_ID;
    data.timestamp = millis();

    xQueueSend(xQueueHandleSendWithLora, &data, portMAX_DELAY);

    vTaskDelay(pdMS_TO_TICKS(SENSOR_READ_INTERVAL));
  }
}

void vTaskLoraControl(void *pvParams)
{
  unsigned long lastSendTime = 0;
  JsonDocument lastDataDocument;

  SensorData data;
  int state = LORA_CHECK;

  while (true)
  {
    if (state == LORA_CHECK)
    {
      if (uxQueueMessagesWaiting(xQueueHandleSendWithLora) > 0) //  || (!bmeAvailable && uxQueueMessagesWaiting(xQueueHandleReceivedFromLora) > 0)
        state = LORA_SEND;
      else
        state = LORA_RECEIVE;
    }

    if (state == LORA_SEND)
    {
      Board.println("Lora: Sending...");
      JsonDocument dataDocumentArray;
      JsonArray dataArray = dataDocumentArray.to<JsonArray>();

      if (xQueueReceive(xQueueHandleReceivedFromLora, &buf, 0) == pdTRUE)
      {
        deserializeJson(lastDataDocument, (char *)buf);
        JsonArray lastDataDocumentArray = lastDataDocument.as<JsonArray>();
        for (JsonObject dataObj : lastDataDocumentArray)
          dataArray.add(dataObj);
      }

      if (xQueueReceive(xQueueHandleSendWithLora, &data, 0) == pdTRUE)
      {
        JsonDocument dataDocument;

        dataDocument["nodeId"] = data.nodeId;
        dataDocument["temperature"] = data.temperature;
        dataDocument["timestamp"] = data.timestamp;

        dataArray.add(dataDocument);
      }

      String dataStrArray;
      serializeJson(dataDocumentArray, dataStrArray);
      lastDataDocument.set(NULL);

      if (NODE_ID > 1)
      {
        if (dataArray.size() > 0)
        {
          uint8_t destinationId = DIRECT_SEND ? 1 : NODE_ID - 1;

          Board.println("Lora: Sending data to node " + String(destinationId) + ": " + String(data.temperature));

          uint8_t result = manager.sendtoWait((uint8_t *)dataStrArray.c_str(), dataStrArray.length(), destinationId);

          if (result == RH_ROUTER_ERROR_NONE)
          {
            Board.println("  -> ACKed!");
          }
          else
          {
            Board.println("  -> FAIL: ");
            switch (result)
            {
            case RH_ROUTER_ERROR_NO_ROUTE:
              Board.println("    -> NO_ROUTE - Dest 404");
              break;
            case RH_ROUTER_ERROR_UNABLE_TO_DELIVER:
              Board.println("    -> UNABLE_TO_DELIVER - Next hop didn't ack");
              break;
            case RH_ROUTER_ERROR_INVALID_LENGTH:
              Board.println("    -> INVALID_LENGTH - Message too long");
              break;
            default:
              Board.println("    -> Unknown error: " + String(result));
              break;
            }
          }
        }
      }
      else
        Board.println("Final Data: " + String(dataStrArray));
      state = LORA_CHECK;
    }

    if (state == LORA_RECEIVE)
    {
      Board.println("Lora: Receiving...");
      uint8_t len = sizeof(buf);
      uint8_t from;
      if (manager.recvfromAckTimeout(buf, &len, RECEIVE_TIMEOUT, &from))
      {
        buf[len] = '\0'; // Null-terminate the received data for printing as string
        xQueueSend(xQueueHandleReceivedFromLora, &buf, portMAX_DELAY);
        Board.println("RECEIVED " + String(NODE_ID) + " <- " + String(from) + ": " + String((char *)buf));
      }
      state = LORA_CHECK;
    }

    // vTaskDelay(pdMS_TO_TICKS(10));
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

  if (!driver.init())
  {
    Board.println("SETUP: LoRa init NOK!");
    while (true)
      ;
  }
  Board.println("SETUP: LoRa init!");

  driver.setFrequency(433.2, true);
  driver.setTxPower(-22);

  if (!manager.init())
  {
    Board.println("SETUP: RHMesh init NOK!");
    while (true)
      ;
  }
  Board.println("SETUP: RHMesh init!");

  Board.println("SETUP: Board init: " + String(NODE_ID));

  xQueueHandleSendWithLora = xQueueCreate(10, sizeof(SensorData));
  xQueueHandleReceivedFromLora = xQueueCreate(10, sizeof(char) * RH_SX126x_MAX_MESSAGE_LEN);

  // if (bmeAvailable)
  // {
  xTaskCreatePinnedToCore(vTaskReadTemperature, "Read Temperature Task: ", configMINIMAL_STACK_SIZE + 1024, NULL, 1, &xTaskHandleReadTemperature, 0);
  // }
  xTaskCreatePinnedToCore(vTaskLoraControl, "Lora Control Task: ", configMINIMAL_STACK_SIZE + (1024 * 4), NULL, 1, &xTaskHandleLoraControl, 1);
}

void loop()
{
}