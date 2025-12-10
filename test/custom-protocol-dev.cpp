#ifndef ORIGIN
#define ORIGIN "ESP-X"
#endif

#include "htwlv3.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#define MSG_SIZE 17
#define TIMEOUT_MS 15000

#define LORA_CHECK 0
#define LORA_SEND 1
#define LORA_RECEIVE 2
#define LORA_WAIT 3

#define NODE_ANCHOR 0
#define NODE_NET 1
#define NODE_READY 2

// TODO: usar o RSSI para determinar o node mais próximo
// TODO: possívelmente fazer utilitários para trabalhar com essa lista de nodes
// TODO: Implementar ack a nível do lora
// TODO: Refletir melhor e tentar debuggar de vdd as máquinas de estado de forma simultânea

typedef struct
{
  int index;
  char name[6];
  int signalStrength;
} NetNode;

NetNode closeNodes[2];
int closeNodesCount = 0;
int nodeId = -1;

TaskHandle_t xTaskHandleLoraControl = NULL;
TaskHandle_t xTaskHandleSetupNetwork = NULL;
TaskHandle_t xTaskHandleCode = NULL;

QueueHandle_t xQueueHandleSendWithLora = NULL;
QueueHandle_t xQueueHandleReceivedFromLora = NULL;

// === LoRa ===

void loraSend(String message)
{
  Board.println("LORA: Sending: " + message);
  Board.lora->sendPacket(message.c_str());
}

void cLoraOnSendDone()
{
  xTaskNotify(xTaskHandleLoraControl, LORA_CHECK, eSetValueWithOverwrite);
  Board.println("LORA: Send done");
}

void cLoraOnSendTimeout()
{
  xTaskNotify(xTaskHandleLoraControl, LORA_SEND, eSetValueWithOverwrite);
  Board.println("LORA: Send timeout");
}

void cLoraOnReceive(LoraDataPacket packet)
{
  LoraDataPacket packetCopy;
  packetCopy.data = (char *)malloc(packet.size + 1);

  if (packetCopy.data == NULL)
    return;

  memcpy(packetCopy.data, packet.data, packet.size);

  packetCopy.data[packet.size] = '\0';
  packetCopy.rssi = packet.rssi;
  packetCopy.size = packet.size;
  packetCopy.snr = packet.snr;

  xQueueSend(xQueueHandleReceivedFromLora, &packetCopy, portMAX_DELAY);

  xTaskNotify(xTaskHandleLoraControl, LORA_CHECK, eSetValueWithOverwrite);
  Board.println("LORA: Received: " + String(packetCopy.data) + " | RSSI: " + String(packetCopy.rssi));
}

void cLoraOnReceiveTimeout()
{
  xTaskNotify(xTaskHandleLoraControl, LORA_CHECK, eSetValueWithOverwrite);
  // Board.println("LORA: Receive timeout");
}

// === Tasks ===

void vTaskLoraControl(void *pvParams)
{
  int state = LORA_CHECK;
  uint32_t notification;
  String currentMessage = "";

  while (true)
  {
    if (state == LORA_CHECK)
    {
      char currentMessageChar[MSG_SIZE];

      if (xQueueReceive(xQueueHandleSendWithLora, &currentMessageChar, 0) == pdTRUE)
      {
        currentMessage = String(currentMessageChar);
        state = LORA_SEND;
      }
      else
        state = LORA_RECEIVE;
    }

    if (state == LORA_SEND)
    {
      loraSend(currentMessage);
      state = LORA_WAIT;
    }

    if (state == LORA_RECEIVE)
    {
      Board.lora->listenToPacket(1000);
      state = LORA_WAIT;
    }

    if (state == LORA_WAIT)
      Board.process();

    if (xTaskNotifyWait(0, ULONG_MAX, &notification, pdMS_TO_TICKS(100)) == pdTRUE)
      state = notification;
  }
}

void vTaskCode(void *pvParams)
{
  Board.println("Code Task started...");

  while (true)
  {
    // Code here...
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void vTaskSetupNetwork(void *pvParams)
{
  int state = ORIGIN == "ESP-1" ? NODE_ANCHOR : NODE_NET;
  Board.println("Setup Network as " + String(state == NODE_ANCHOR ? "ANCHOR" : "NODE"));
  char messageSend[MSG_SIZE];
  LoraDataPacket messageReceived;
  bool setupReady = false;
  int discoveredCloseNodes = 0;

  while (true)
  {
    if (state == NODE_ANCHOR)
    {
      Board.println("ANCHOR Protocol");

      // If nobody answers in 5 seconds, assume network is ready, delegate discovery to other nodes
      int timeout = TIMEOUT_MS;
      int tries = 2;
      do
      {
        if (timeout >= TIMEOUT_MS)
        {
          if (tries > 0)
          {
            strcpy(messageSend, ("RLY:" + String(ORIGIN)).c_str());
            Board.println("Sending: " + String(messageSend));
            xQueueSend(xQueueHandleSendWithLora, &messageSend, portMAX_DELAY);
            tries--;
            timeout = 0;
            continue;
          }

          Board.println("Timeout: " + String(timeout));
          if (discoveredCloseNodes == 0)
          {
            Board.println("No nodes found, initiating net ready state");
            nodeId = 0;
            strcpy(messageSend, ("RDY:" + String(closeNodes[closeNodesCount - 1].name) + ":" + String(ORIGIN) + ":" + nodeId).c_str());
            xQueueSend(xQueueHandleSendWithLora, &messageSend, portMAX_DELAY);
            state = NODE_READY;
            setupReady = true;
            Board.println("Node ready, ID: " + String(nodeId));
            break;
          }

          strcpy(messageSend, ("DCV:" + String(closeNodes[closeNodesCount - 1].name)).c_str());
          xQueueSend(xQueueHandleSendWithLora, &messageSend, portMAX_DELAY);
          Board.println("Anchor setup done");
          state = NODE_NET;
          setupReady = true;
          break;
        }
        timeout += 100;
      } while (xQueueReceive(xQueueHandleReceivedFromLora, &messageReceived, 100) != pdTRUE);
      Board.println("Response received");

      if (state != NODE_ANCHOR)
        continue;

      String message = String(messageReceived.data);
      free(messageReceived.data);

      String cmd = message.substring(0, message.indexOf(":"));
      Board.println("POST Response received. Message: " + message);

      if (cmd.equals("INP"))
      {
        String senderName = message.substring(message.indexOf(":") + 1);

        bool nodeExists = false;

        for (int i = 0; i < closeNodesCount; i++)
          if (String(closeNodes[i].name).equals(senderName))
            nodeExists = true;

        if (!nodeExists)
        {
          tries = 2;
          Board.println("Node discovered: " + String(senderName));
          NetNode senderNode = {.index = closeNodesCount, .signalStrength = messageReceived.rssi};
          strcpy(senderNode.name, senderName.c_str());
          closeNodes[closeNodesCount] = senderNode;
          closeNodesCount++;
          discoveredCloseNodes++;
        }

        Board.println("Sending ACK to: " + String(senderName));
        strcpy(messageSend, ("ACK:" + senderName).c_str());
        xQueueSend(xQueueHandleSendWithLora, &messageSend, portMAX_DELAY);
      }
      continue;
    }

    if (state == NODE_NET)
    {
      Board.println("NODE Protocol");
      do
        vTaskDelay(pdMS_TO_TICKS(100));
      while (xQueueReceive(xQueueHandleReceivedFromLora, &messageReceived, 0) != pdTRUE);

      String message = String(messageReceived.data);
      free(messageReceived.data);

      String cmd = message.substring(0, message.indexOf(":"));

      if (!setupReady && cmd.equals("RLY"))
      {
        String senderName = message.substring(message.indexOf(":") + 1);

        bool nodeExists = false;

        for (int i = 0; i < closeNodesCount; i++)
          if (String(closeNodes[i].name).equals(senderName))
            nodeExists = true;

        if (!nodeExists)
        {
          NetNode senderNode = {.index = closeNodesCount, .signalStrength = messageReceived.rssi};
          strcpy(senderNode.name, senderName.c_str());
          closeNodes[closeNodesCount] = senderNode;
          closeNodesCount++;
        }

        // Resend if ack not received
        int timeout = TIMEOUT_MS;
        do
        {
          if (timeout >= TIMEOUT_MS)
          {
            strcpy(messageSend, ("INP:" + String(ORIGIN)).c_str());
            xQueueSend(xQueueHandleSendWithLora, &messageSend, portMAX_DELAY);
            timeout = 0;
          }
          vTaskDelay(pdMS_TO_TICKS(100));
          timeout += 100;
        } while (xQueueReceive(xQueueHandleReceivedFromLora, &messageReceived, 0) != pdTRUE);

        message = String(messageReceived.data);
        free(messageReceived.data);

        if (message.equals("ACK:" + String(ORIGIN)))
        {
          setupReady = true;
          Board.println("Network setup done");
          continue;
        }
        continue;
      }

      if (cmd.equals("DCV"))
      {
        String senderName = message.substring(message.indexOf(":") + 1);

        if (!senderName.equals(String(ORIGIN)))
          setupReady = false; // TODO: Isso não vai funcionar se os nodes estiverem longe
        else
          state = NODE_ANCHOR;

        continue;
      }

      if (message.startsWith("RDY:" + String(ORIGIN)))
      {
        String senderNumber = String(message);
        senderNumber.replace("RDY:" + String(ORIGIN) + ":", "");
        String sender = senderNumber.substring(0, message.indexOf(":"));
        nodeId = senderNumber.substring(message.indexOf(":") + 1).toInt() + 1;

        String sendTo = "";

        for (int i = 0; i < closeNodesCount; i++)
          if (!String(closeNodes[i].name).equals(sender)) // TODO: Isso parece não ter funcionado??
            sendTo = String(closeNodes[i].name);

        strcpy(messageSend, ("RDY:" + String(sendTo) + ":" + String(ORIGIN) + ":" + nodeId).c_str());
        xQueueSend(xQueueHandleSendWithLora, &messageSend, portMAX_DELAY);
        state = NODE_READY;
        Board.println("Node ready, ID: " + String(nodeId));
      }

      continue;
    }

    if (state == NODE_READY)
    {
      Board.println("Node ready, waiting for network commands");
      break;
    }
  }

  xTaskCreatePinnedToCore(vTaskCode, "Code Task", configMINIMAL_STACK_SIZE + (1024 * 4), NULL, 1, &xTaskHandleCode, 1);
  vTaskDelete(NULL);
}

void config()
{
  HTWLV3Config boardConfig = HTWLV3::getDefaultConfig();

  boardConfig.serialEnable = true;
  // boardConfig.displayEnable = true;
  boardConfig.loraEnable = true;

  Board.setConfig(boardConfig);

  // HTLORAV3Config loraConfig = HTLORAV3::getDefaultConfig();

  // loraConfig.txOutPower = 12;

  // Board.lora->setConfig(loraConfig);

  Board.lora->setOnReceive(cLoraOnReceive);
  Board.lora->setOnReceiveTimeout(cLoraOnReceiveTimeout);
  Board.lora->setOnSendDone(cLoraOnSendDone);
  Board.lora->setOnSendTimeout(cLoraOnSendTimeout);
}

void setup()
{
  config();
  Board.begin();

  Board.println("Board init: " + String(ORIGIN));

  xQueueHandleSendWithLora = xQueueCreate(10, sizeof(char) * MSG_SIZE);
  xQueueHandleReceivedFromLora = xQueueCreate(10, sizeof(LoraDataPacket));

  xTaskCreatePinnedToCore(vTaskLoraControl, "Lora Control Task: ", configMINIMAL_STACK_SIZE + (1024 * 4), NULL, 1, &xTaskHandleLoraControl, 0);

  xTaskCreatePinnedToCore(vTaskSetupNetwork, "Setup Network Task: ", configMINIMAL_STACK_SIZE + (1024 * 4), NULL, 1, &xTaskHandleSetupNetwork, 1);
}

void loop()
{
}