#include <Arduino.h>
#include "loramesher.h"

#define BOARD_LED BUILTIN_LED
#define LED_ON LOW
#define LED_OFF HIGH

#define RADIO_DIO_1 14
#define RADIO_NSS 8
#define RADIO_RESET 12
#define RADIO_BUSY 13

#ifndef ORIGIN
#define ORIGIN "ESP-X"
#endif

LoraMesher &radio = LoraMesher::getInstance();

uint16_t LOCAL_ADDR = radio.getLocalAddress();

uint16_t SEND_ADDR1 = LOCAL_ADDR == 0x5CEC ? 0xD510 : (LOCAL_ADDR == 0xD510 ? 0x8088 : 0x5CEC);
uint16_t SEND_ADDR2 = LOCAL_ADDR == 0x5CEC ? 0x8088 : (LOCAL_ADDR == 0xD510 ? 0x5CEC : 0xD510);

uint16_t currentSendAddr = SEND_ADDR1;

uint32_t dataCounter = 0;

struct dataPacket
{
  uint32_t counter = 0;
};

dataPacket *helloPacket = new dataPacket;

// Led flash
void led_Flash(uint16_t flashes, uint16_t delaymS)
{
  uint16_t index;
  for (index = 1; index <= flashes; index++)
  {
    digitalWrite(BOARD_LED, LED_ON);
    delay(delaymS);
    digitalWrite(BOARD_LED, LED_OFF);
    delay(delaymS);
  }
}

void printPacket(dataPacket data)
{
  Serial.printf("%s: Count %d\n", ORIGIN, data.counter);
}

void printDataPacket(AppPacket<dataPacket> *packet)
{
  Serial.printf("%s: <- %X (Size: %d)\n", ORIGIN, packet->src, packet->payloadSize);

  // Get the payload to iterate through it
  dataPacket *dPacket = packet->payload;
  size_t payloadLength = packet->getPayloadLength();

  for (size_t i = 0; i < payloadLength; i++)
  {
    // Print the packet
    printPacket(dPacket[i]);
  }
}

void processReceivedPackets(void *)
{
  for (;;)
  {
    /* Wait for the notification of processReceivedPackets and enter blocking */
    ulTaskNotifyTake(pdPASS, portMAX_DELAY);
    led_Flash(1, 100); // one quick LED flashes to indicate a packet has arrived

    // Iterate through all the packets inside the Received User Packets FiFo
    while (radio.getReceivedQueueSize() > 0)
    {
      Serial.printf("%s: Queue size: %d\n", ORIGIN, radio.getReceivedQueueSize() > 0);

      // Get the first element inside the Received User Packets FiFo
      AppPacket<dataPacket> *packet = radio.getNextAppPacket<dataPacket>();

      // Print the data packet
      printDataPacket(packet);

      // Delete the packet when used. It is very important to call this function to release the memory of the packet.
      radio.deletePacket(packet);
    }
  }
}

TaskHandle_t receiveLoRaMessage_Handle = NULL;

void createReceiveMessages()
{
  int res = xTaskCreate(
      processReceivedPackets,
      "Receive App Task",
      4096,
      (void *)1,
      2,
      &receiveLoRaMessage_Handle);
  if (res != pdPASS)
  {
    Serial.printf("%s: ERROR on Receive App Task creation: %d\n", ORIGIN, res);
  }

  radio.setReceiveAppDataTaskHandle(receiveLoRaMessage_Handle);
}

void setupLoraMesher()
{
  // gpio_install_isr_service(0);

  LoraMesher::LoraMesherConfig config = LoraMesher::LoraMesherConfig();

  config.loraCs = RADIO_NSS;    // LoRa chip select pin
  config.loraIrq = RADIO_DIO_1; // LoRa IRQ pin
  config.loraRst = RADIO_RESET; // LoRa reset pin
  config.loraIo1 = RADIO_BUSY;  // LoRa DIO1 pin
  config.module = LoraMesher::SX1262_MOD;
  config.freq = 433.000F;

  radio.begin(config);

  // Create the receive task and add it to the LoRaMesher
  createReceiveMessages();

  // Start LoRaMesher
  radio.start();

  Serial.printf("%s: Lora initialized\n", ORIGIN);
}

void setup()
{
  Serial.begin(115200);

  Serial.printf("%s: INIT (%X -> %X)\n", ORIGIN, LOCAL_ADDR, currentSendAddr);
  pinMode(BOARD_LED, OUTPUT); // setup pin as output for indicator LED
  led_Flash(2, 125);          // two quick LED flashes to indicate program start
  setupLoraMesher();
}

void loop()
{
  Serial.printf("%s: Send: %d -> %X\n", ORIGIN, dataCounter, currentSendAddr);

  helloPacket->counter = dataCounter++;

  // Create packet and send it.
  radio.createPacketAndSend(currentSendAddr, helloPacket, 1);

  currentSendAddr = currentSendAddr == SEND_ADDR1 ? SEND_ADDR2 : SEND_ADDR1;

  // Wait 20 seconds to send the next packet
  int delay = ORIGIN == "ESP-1" ? 5000 : ORIGIN == "ESP-2" ? 10000
                                                           : 15000;
  vTaskDelay(delay / portTICK_PERIOD_MS);
}