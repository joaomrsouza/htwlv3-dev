#include "secrets.h"

#include "htwlv3.h"

#include <Adafruit_BME280.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#define STATE_CHECK 0
#define STATE_SEND 1
#define STATE_RECEIVE 2
#define STATE_WAIT 3

typedef struct
{
  char origin[6];
  char route[5][6];
  float temperature;
} SensorData;

Adafruit_BME280 bme;
TwoWire BMEWire = TwoWire(1);

TaskHandle_t xTaskHandleReadTemperature = NULL;
TaskHandle_t xTaskHandleSendToServer = NULL;
TaskHandle_t xTaskHandleLoraControl = NULL;

QueueHandle_t xQueueHandleSendToServer = NULL;
QueueHandle_t xQueueHandleSendToLora = NULL;

void vTaskReadTemperature(void *pvParams);
void vTaskSendToServer(void *pvParams);
void vTaskLoraControl(void *pvParams);

void cLoraOnReceive(LoraDataPacket packet);
void cLoraOnReceiveTimeout();
void cLoraOnSendDone();
void cLoraOnSendTimeout();

void loraSend(JsonDocument dataDocument);

JsonDocument dataToJson(SensorData data);
void jsonToData(JsonDocument dataDocument, SensorData *data);

void config();

void setup()
{
  config();
  Board.begin();

  BMEWire.begin(SDA, SCL);

  if (!bme.begin(0x76, &BMEWire))
  {
    Board.println("SETUP: BME280 not init.");
    for (;;)
      ;
  }

  Board.println("SETUP: BME280 init.");

  Board.print("Board init: ");
  Board.println(ORIGIN);

  xQueueHandleSendToServer = xQueueCreate(10, sizeof(SensorData));
  xQueueHandleSendToLora = xQueueCreate(10, sizeof(SensorData));

  xTaskCreatePinnedToCore(vTaskReadTemperature, "Read Temperature Task: ", configMINIMAL_STACK_SIZE + 1024, NULL, 1, &xTaskHandleReadTemperature, 0);
  xTaskCreatePinnedToCore(vTaskSendToServer, "Send to Server Task: ", configMINIMAL_STACK_SIZE + (1024 * 4), NULL, 1, &xTaskHandleSendToServer, 0);
  xTaskCreatePinnedToCore(vTaskLoraControl, "Lora Control Task: ", configMINIMAL_STACK_SIZE + (1024 * 4), NULL, 1, &xTaskHandleLoraControl, 1);
}

void loop()
{
}

void vTaskReadTemperature(void *pvParams)
{
  while (true)
  {
    Board.println("BME: Reading...");

    SensorData data;

    data.temperature = bme.readTemperature();
    strcpy(data.origin, ORIGIN);
    strcpy(data.route[0], ORIGIN);

    if (Board.wifi->client->getIsConnected())
      xQueueSend(xQueueHandleSendToServer, &data, portMAX_DELAY);
    else
      xQueueSend(xQueueHandleSendToLora, &data, portMAX_DELAY);

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void vTaskSendToServer(void *pvParams)
{
  SensorData data;

  while (true)
  {
    JsonDocument dataDocument;
    JsonArray dataArray = dataDocument.to<JsonArray>();

    Board.println("SERVER: Checking queue...");

    while (xQueueReceive(xQueueHandleSendToServer, &data, 0) == pdTRUE)
    {
      Board.print("SERVER: Data from: ");
      Board.println(String(data.origin).c_str());

      JsonDocument dataToSend = dataToJson(data);

      dataArray.add(dataToSend);
    }

    if (dataArray.size() == 0)
    {
      vTaskDelay(pdMS_TO_TICKS(1000));
      continue;
    }

    Board.println("SERVER: Sending...");

    String jsonString;
    serializeJson(dataDocument, jsonString);
    Board.println(jsonString.c_str());

    JsonDocument response = Board.wifi->client->post("http://" + String(SERVER_IP) + ":" + String(SERVER_PORT) + "/", dataDocument);

    Board.println("SERVER: Response...");
    serializeJson(response, jsonString);
    Board.println(jsonString.c_str());

    if (response["error"])
    {
      Board.print("SERVER: ERROR: ");
      Board.println(response["error_message"]);
    }
    else
    {
      String data;
      serializeJson(response["data"], data);
      Board.println(data.c_str());
    }

    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}

void vTaskLoraControl(void *pvParams)
{
  int state = STATE_CHECK;
  uint32_t notification;
  SensorData lastData;
  JsonDocument lastDataDocument;

  while (true)
  {
    if (state == STATE_CHECK)
    {
      // Board.println("LORA: Checking queue...");

      JsonArray lastDataArray = lastDataDocument.to<JsonArray>();

      while (xQueueReceive(xQueueHandleSendToLora, &lastData, 0) == pdTRUE)
      {
        JsonDocument dataToSend = dataToJson(lastData);

        lastDataArray.add(dataToSend);
      }

      if (lastDataArray.size() > 0)
        state = STATE_SEND;
      else
        state = STATE_RECEIVE;
    }

    if (state == STATE_SEND)
    {
      loraSend(lastDataDocument);
      state = STATE_WAIT;
    }

    if (state == STATE_RECEIVE)
    {
      Board.lora->listenToPacket(1000);
      state = STATE_WAIT;
    }

    if (state == STATE_WAIT)
      Board.process();

    if (xTaskNotifyWait(0, ULONG_MAX, &notification, pdMS_TO_TICKS(100)) == pdTRUE)
      state = notification;
  }
}

void cLoraOnReceive(LoraDataPacket packet)
{
  JsonDocument loraData;
  deserializeJson(loraData, packet.data);

  JsonArray dataArray = loraData.as<JsonArray>();

  for (JsonVariant jsonData : dataArray)
  {
    JsonArray routeArray = jsonData["route"];

    routeArray.add(ORIGIN);

    SensorData data;
    jsonToData(jsonData, &data);

    Board.print("LORA: Received data: ");
    Board.println(packet.data);

    if (Board.wifi->client->getIsConnected())
      xQueueSend(xQueueHandleSendToServer, &data, portMAX_DELAY);
    else
      xQueueSend(xQueueHandleSendToLora, &data, portMAX_DELAY);
  }

  xTaskNotify(xTaskHandleLoraControl, STATE_CHECK, eSetValueWithOverwrite);
  Board.println("LORA: Receive done");
}

void cLoraOnReceiveTimeout()
{
  xTaskNotify(xTaskHandleLoraControl, STATE_CHECK, eSetValueWithOverwrite);
  Board.println("LORA: Receive timeout");
}

void cLoraOnSendDone()
{
  xTaskNotify(xTaskHandleLoraControl, STATE_CHECK, eSetValueWithOverwrite);
  Board.println("LORA: Send done");
}

void cLoraOnSendTimeout()
{
  xTaskNotify(xTaskHandleLoraControl, STATE_SEND, eSetValueWithOverwrite);
  Board.println("LORA: Send timeout");
}

void loraSend(JsonDocument dataDocument)
{
  Board.print("LORA: Sending data: ");

  String dataString;
  serializeJson(dataDocument, dataString);

  Board.println(dataString.c_str());
  Board.lora->sendPacket(dataString.c_str());
}

JsonDocument dataToJson(SensorData data)
{
  JsonDocument dataDocument;

  JsonDocument routeData;
  JsonArray routeArray = routeData.to<JsonArray>();

  for (int i = 0; i < 5; i++)
  {
    if (data.route[i][0] != '\0')
      routeArray.add(data.route[i]);
  }

  dataDocument["origin"] = data.origin;
  dataDocument["route"] = routeData;
  dataDocument["temperature"] = data.temperature;

  return dataDocument;
}

void jsonToData(JsonDocument dataDocument, SensorData *data)
{
  const char *origin = dataDocument["origin"].as<const char *>();

  if (origin)
    strcpy(data->origin, origin);
  else
    strcpy(data->origin, "\0");

  data->temperature = dataDocument["temperature"];

  // Inicializar array route
  for (int j = 0; j < 5; j++)
    data->route[j][0] = '\0';

  JsonArray routeArray = dataDocument["route"];

  int i = 0;
  for (JsonVariant value : routeArray)
  {
    if (i >= 5)
      break; // Prevenir buffer overflow

    Board.print(String(i).c_str());
    Board.println(value.as<const char *>());
    strcpy(data->route[i], value.as<const char *>());
    i++;
  }
}

void config()
{
  // === Board Config ===

  HTWLV3Config boardConfig;

  boardConfig.serialEnable = true;
  boardConfig.serialSpeed = 115200;
  boardConfig.displayEnable = true;
  boardConfig.loraEnable = true;
  boardConfig.wifiEnable = true;

  Board.setConfig(&boardConfig);

  // === LoRa Config ===

  if (boardConfig.loraEnable)
  {
    HTLORAV3Config loraConfig;

    loraConfig.frequency = 470E6;
    loraConfig.bandwidth = 0;
    loraConfig.spreadingFactor = 7;
    loraConfig.codingRate = 1;
    loraConfig.preambleLength = 8;
    loraConfig.fixLengthPayloadOn = false;
    loraConfig.iqInversionOn = false;
    loraConfig.txOutPower = 12;
    loraConfig.txTimeout = 3000;
    loraConfig.rxTimeout = 0;

    Board.lora->setConfig(&loraConfig);

    Board.lora->setOnReceive(cLoraOnReceive);
    Board.lora->setOnReceiveTimeout(cLoraOnReceiveTimeout);
    Board.lora->setOnSendDone(cLoraOnSendDone);
    Board.lora->setOnSendTimeout(cLoraOnSendTimeout);
  }

  // === WiFi Config ===

  if (boardConfig.wifiEnable)
  {
    HTWIFIV3Config wifiConfig;

    wifiConfig.clientEnable = true;
    wifiConfig.serverEnable = false;

    Board.wifi->setConfig(&wifiConfig);

    // === Client Config ===

    if (wifiConfig.clientEnable)
    {
      HTWIFIV3ClientConfig clientConfig;

      clientConfig.ssid = CLIENT_SSID;
      clientConfig.password = CLIENT_PASSWORD;

      Board.wifi->client->setConfig(&clientConfig);
    }

    // === Server Config ===

    if (wifiConfig.serverEnable)
    {
      HTWIFIV3ServerConfig serverConfig;

      serverConfig.ssid = SERVER_SSID;
      serverConfig.password = SERVER_PASSWORD;

      Board.wifi->server->setConfig(&serverConfig);
    }
  }
}