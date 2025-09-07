/**
 * @file wifi-lora-bme.cpp
 * @brief Example to use the HelTec WiFi LoRa 32 V3 Board with a BME280 sensor and send the data to a server via WiFi or LoRa
 *
 * Depends On:
 * - htwlv3
 * - Adafruit_BME280
 * - FreeRTOS
 *
 * @author @joaomrsouza (João Marcos Rocha Souza)
 * https://github.com/joaomrsouza
 */

// Secrets file is not included in the library, you need to create it yourself
// #include "secrets.h"

// Example of secrets file
#define ORIGIN "ESP-1"
#define CLIENT_SSID "YOUR_SSID"
#define CLIENT_PASSWORD "YOUR_PASSWORD"
#define SERVER_IP "YOUR_IP"
#define SERVER_PORT 3000
// It's recommended to use the secrets file instead of defining the variables here

// Include the HelTec WiFi LoRa 32 V3 Board library
#include "htwlv3.h"

// Include the BME280 library
#include <Adafruit_BME280.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

// Define states for the communication
#define STATE_CHECK 0
#define STATE_SEND 1
#define STATE_RECEIVE 2
#define STATE_WAIT 3

// Define the data structure to be sent
typedef struct
{
  char origin[6];
  char route[5][6];
  float temperature;
} SensorData;

// Declare the BME280 instance
Adafruit_BME280 bme;
TwoWire BMEWire = TwoWire(1);

// Declare the task handles
TaskHandle_t xTaskHandleReadTemperature = NULL;
TaskHandle_t xTaskHandleSendToServer = NULL;
TaskHandle_t xTaskHandleLoraControl = NULL;

// Declare the queue handles
QueueHandle_t xQueueHandleSendToServer = NULL;
QueueHandle_t xQueueHandleSendToLora = NULL;

// Declare the task functions
void vTaskReadTemperature(void *pvParams);
void vTaskSendToServer(void *pvParams);
void vTaskLoraControl(void *pvParams);

// Declare the callback functions for the LoRa library
void cLoraOnReceive(LoraDataPacket packet);
void cLoraOnReceiveTimeout();
void cLoraOnSendDone();
void cLoraOnSendTimeout();

// Declare the function to send the data to LoRa
void loraSend(JsonDocument dataDocument);

// Declare the functions to convert the data to JSON
JsonDocument dataToJson(SensorData data);
void jsonToData(JsonDocument dataDocument, SensorData *data);

// Declare the config function
void config();

void setup()
{
  // Configure and initialize the board
  config();
  Board.begin();

  // Initialize the BME280 sensor
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

  // Initialize the queues
  xQueueHandleSendToServer = xQueueCreate(10, sizeof(SensorData));
  xQueueHandleSendToLora = xQueueCreate(10, sizeof(SensorData));

  // Create the tasks
  xTaskCreatePinnedToCore(vTaskReadTemperature, "Read Temperature Task: ", configMINIMAL_STACK_SIZE + 1024, NULL, 1, &xTaskHandleReadTemperature, 0);
  xTaskCreatePinnedToCore(vTaskSendToServer, "Send to Server Task: ", configMINIMAL_STACK_SIZE + (1024 * 4), NULL, 1, &xTaskHandleSendToServer, 0);
  // LoRa Control Task on core 1 to make sure it's always running
  xTaskCreatePinnedToCore(vTaskLoraControl, "Lora Control Task: ", configMINIMAL_STACK_SIZE + (1024 * 4), NULL, 1, &xTaskHandleLoraControl, 1);
}

void loop()
{
}

// Task to read the temperature from the BME280 sensor and send it to the server or LoRa
void vTaskReadTemperature(void *pvParams)
{
  while (true)
  {
    Board.println("BME: Reading...");

    // Create the data structure to be sent
    SensorData data;

    // Read the temperature from the BME280 sensor
    data.temperature = bme.readTemperature();
    strcpy(data.origin, ORIGIN);
    strcpy(data.route[0], ORIGIN);

    // Send the data to the server or LoRa
    if (Board.wifi->client->getIsConnected())
      xQueueSend(xQueueHandleSendToServer, &data, portMAX_DELAY);
    else
      xQueueSend(xQueueHandleSendToLora, &data, portMAX_DELAY);

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

// Task to send the data to the server
void vTaskSendToServer(void *pvParams)
{
  SensorData data;

  while (true)
  {
    // Create the data document
    JsonDocument dataDocument;
    JsonArray dataArray = dataDocument.to<JsonArray>();

    Board.println("SERVER: Checking queue...");

    // Receive the data from the queue
    while (xQueueReceive(xQueueHandleSendToServer, &data, 0) == pdTRUE)
    {
      Board.print("SERVER: Data from: ");
      Board.println(data.origin);

      // Convert the data to JSON
      JsonDocument dataToSend = dataToJson(data);

      // Add the data to the array
      dataArray.add(dataToSend);
    }

    // If there is no data, wait for 1 second and continue
    if (dataArray.size() == 0)
    {
      vTaskDelay(pdMS_TO_TICKS(1000));
      continue;
    }

    // Send the data to the server
    Board.println("SERVER: Sending...");

    // Convert the data to a JSON string
    String jsonString;
    serializeJson(dataDocument, jsonString);
    Board.println(jsonString);

    // Send the data to the server
    JsonDocument response = Board.wifi->client->post("http://" + String(SERVER_IP) + ":" + String(SERVER_PORT) + "/", dataDocument);

    Board.println("SERVER: Response...");
    serializeJson(response, jsonString);
    Board.println(jsonString);

    // If there is an error, print the error message
    if (response["error"])
    {
      Board.print("SERVER: ERROR: ");
      Board.println(response["error_message"]);
    }
    else
    {
      String data;
      serializeJson(response["data"], data);
      Board.println(data);
    }

    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}

// Task to control the LoRa communication
void vTaskLoraControl(void *pvParams)
{
  // Set the initial state
  int state = STATE_CHECK;
  uint32_t notification;
  SensorData lastData;
  JsonDocument lastDataDocument;

  while (true)
  {
    // Check if there is data to send to LoRa
    if (state == STATE_CHECK)
    {
      JsonArray lastDataArray = lastDataDocument.to<JsonArray>();

      while (xQueueReceive(xQueueHandleSendToLora, &lastData, 0) == pdTRUE)
      {
        // Convert the data to JSON
        JsonDocument dataToSend = dataToJson(lastData);

        // Add the data to the array
        lastDataArray.add(dataToSend);
      }

      // If there is data, set the state to send, otherwise set the state to receive
      if (lastDataArray.size() > 0)
        state = STATE_SEND;
      else
        state = STATE_RECEIVE;
    }

    // Send the data to LoRa
    if (state == STATE_SEND)
    {
      // Send the data to LoRa and wait for the send done callback
      loraSend(lastDataDocument);
      state = STATE_WAIT;
    }

    // Receive the data from LoRa
    if (state == STATE_RECEIVE)
    {
      // Listen to a packet for 1 second and wait for the receive done callback or timeout
      Board.lora->listenToPacket(1000);
      state = STATE_WAIT;
    }

    // Process the board
    if (state == STATE_WAIT)
      Board.process();

    // Wait for the notification
    if (xTaskNotifyWait(0, ULONG_MAX, &notification, pdMS_TO_TICKS(100)) == pdTRUE)
      state = notification;
  }
}

// Callback function to receive the data from LoRa
void cLoraOnReceive(LoraDataPacket packet)
{
  // Convert the data to JSON
  JsonDocument loraData;
  deserializeJson(loraData, packet.data);

  JsonArray dataArray = loraData.as<JsonArray>();

  // Add the origin to the route array
  for (JsonVariant jsonData : dataArray)
  {
    // Get the route array
    JsonArray routeArray = jsonData["route"];

    // Add the origin to the route array
    routeArray.add(ORIGIN);

    // Convert the data to a JSON string
    SensorData data;
    jsonToData(jsonData, &data);

    Board.print("LORA: Received data: ");
    Board.println(packet.data);

    // Send the data to the server or LoRa
    if (Board.wifi->client->getIsConnected())
      xQueueSend(xQueueHandleSendToServer, &data, portMAX_DELAY);
    else
      xQueueSend(xQueueHandleSendToLora, &data, portMAX_DELAY);
  }

  // Notify the LoRa Control Task to go back to the check state
  xTaskNotify(xTaskHandleLoraControl, STATE_CHECK, eSetValueWithOverwrite);
  Board.println("LORA: Receive done");
}

// Callback function to LoRa receive timeout
void cLoraOnReceiveTimeout()
{
  // Notify the LoRa Control Task to go back to the check state
  xTaskNotify(xTaskHandleLoraControl, STATE_CHECK, eSetValueWithOverwrite);
  Board.println("LORA: Receive timeout");
}

// Callback function to LoRa send done
void cLoraOnSendDone()
{
  // Notify the LoRa Control Task to go back to the check state
  xTaskNotify(xTaskHandleLoraControl, STATE_CHECK, eSetValueWithOverwrite);
  Board.println("LORA: Send done");
}

// Callback function to LoRa send timeout
void cLoraOnSendTimeout()
{
  // Notify the LoRa Control Task to go back to the send state
  xTaskNotify(xTaskHandleLoraControl, STATE_SEND, eSetValueWithOverwrite);
  Board.println("LORA: Send timeout");
}

// Function to send the data to LoRa
void loraSend(JsonDocument dataDocument)
{
  Board.print("LORA: Sending data: ");

  // Convert the data to a JSON string
  String dataString;
  serializeJson(dataDocument, dataString);

  Board.println(dataString);
  // Send the data to LoRa
  Board.lora->sendPacket(dataString.c_str());
}

// Function to convert the data to JSON
JsonDocument dataToJson(SensorData data)
{
  // Create the data document
  JsonDocument dataDocument;

  // Create the route data document
  JsonDocument routeData;
  JsonArray routeArray = routeData.to<JsonArray>();

  // Add the route to the data document
  for (int i = 0; i < 5; i++)
  {
    // If the route is not empty, add it to the data document
    if (data.route[i][0] != '\0')
      routeArray.add(data.route[i]);
  }

  // Add the origin, route and temperature to the data document
  dataDocument["origin"] = data.origin;
  dataDocument["route"] = routeData;
  dataDocument["temperature"] = data.temperature;

  return dataDocument;
}

// Function to convert the JSON data to a data structure
void jsonToData(JsonDocument dataDocument, SensorData *data)
{
  // Get the origin from the data document
  const char *origin = dataDocument["origin"].as<const char *>();

  // If the origin is not empty, copy it to the data structure
  if (origin)
    strcpy(data->origin, origin);
  else
    strcpy(data->origin, "\0");

  // Get the temperature from the data document
  data->temperature = dataDocument["temperature"];

  // Initialize the route array
  for (int j = 0; j < 5; j++)
    data->route[j][0] = '\0';

  // Get the route array from the data document
  JsonArray routeArray = dataDocument["route"];

  // Add the route to the data structure
  int i = 0;
  for (JsonVariant value : routeArray)
  {
    if (i >= 5)
      break;

    // Copy the value to the data structure
    strcpy(data->route[i], value.as<const char *>());
    i++;
  }
}

// Function to configure the board
void config()
{
  // === Board Config ===

  HTWLV3Config boardConfig = HTWLV3::getDefaultConfig();

  boardConfig.serialEnable = true;
  boardConfig.displayEnable = true;
  boardConfig.loraEnable = true;
  boardConfig.wifiEnable = true;

  Board.setConfig(boardConfig);

  // === LoRa Config ===

  HTLORAV3Config loraConfig = HTLORAV3::getDefaultConfig();

  loraConfig.txOutPower = 12;

  Board.lora->setConfig(loraConfig);

  Board.lora->setOnReceive(cLoraOnReceive);
  Board.lora->setOnReceiveTimeout(cLoraOnReceiveTimeout);
  Board.lora->setOnSendDone(cLoraOnSendDone);
  Board.lora->setOnSendTimeout(cLoraOnSendTimeout);

  // === WiFi Config ===

  HTWIFIV3Config wifiConfig = HTWIFIV3::getDefaultConfig();

  wifiConfig.clientEnable = true;

  Board.wifi->setConfig(wifiConfig);

  // === Client Config ===

  HTWIFIV3ClientConfig clientConfig = HTWIFIV3Client::getDefaultConfig();

  clientConfig.ssid = CLIENT_SSID;
  clientConfig.password = CLIENT_PASSWORD;

  Board.wifi->client->setConfig(clientConfig);
}