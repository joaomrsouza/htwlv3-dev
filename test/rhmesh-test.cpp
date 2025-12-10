#include <SPI.h>
#include "RH_SX126x.h" // LoRa driver for SX1262
#include "RHMesh.h"    // The Mesh Layer

// --- LoRa Hardware Pins (Adjust these for your specific board!) ---
#define RADIO_DIO_1 14
#define RADIO_NSS 8
#define RADIO_RESET 12
#define RADIO_BUSY 13
#define Vext 36 // External power control pin

// #define LORA_CLK 9
// #define LORA_MISO 11
// #define LORA_MOSI 10

// --- Mesh Network Configuration ---
// Each node in your mesh MUST have a unique address from 1 to 254
// Address 0 is reserved for broadcast.
#ifndef MY_NODE_ADDRESS
#define MY_NODE_ADDRESS 1 // This node's unique ID
#endif

#ifndef ORIGIN
#define ORIGIN "ESP-X"
#endif

// --- RadioHead Instances ---
// Create the LoRa driver instance
RH_SX126x driver(RADIO_NSS, RADIO_DIO_1, RADIO_BUSY, RADIO_RESET);

// Create the RHMesh manager instance
// RHMesh(driver, thisNodeAddress)
// You can optionally add a router table size and a Time-To-Live (TTL) for messages
RHMesh manager(driver, MY_NODE_ADDRESS);

// Packet buffer
uint8_t buf[RH_SX126x_MAX_MESSAGE_LEN];

int lastDestNode = MY_NODE_ADDRESS;

void print(String str)
{
  Serial.println(String(ORIGIN) + ": " + str);
}

void setup()
{
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW);

  Serial.begin(115200);
  while (!Serial)
    ;
  print("Node " + String(MY_NODE_ADDRESS));

  // DEBUG: Print current state of BUSY pin before init
  // pinMode(RADIO_BUSY, INPUT); // Ensure it's set as input
  // Serial.print("BUSY pin (before init): ");
  // Serial.println(digitalRead(RADIO_BUSY));
  delay(100); // Give it a moment

  // --- Initialize LoRa Driver ---
  if (!driver.init())
  {
    print("LoRa init NOK!");
    while (1)
      ;
  }
  print("LoRa init OK!");

  // --- Configure LoRa Radio Settings ---
  // Set frequency (check local regulations)
  driver.setFrequency(433.2, true);
  // if (!driver.setFrequency(433.2, true))
  // { // Example: 915 MHz for US
  //   Serial.println("setFrequency failed");
  //   while (1)
  //     ;
  // }
  // Serial.println("Set Freq to 433.2MHz");

  // Set transmit power (23 dBm is max for LoRa, use sparingly)
  driver.setTxPower(23);

  // You can also set other LoRa parameters if needed:
  // driver.setSpreadingFactor(7);
  // driver.setSignalBandwidth(125000);
  // driver.setCodingRate4(5);

  // --- Initialize RHMesh Manager ---
  if (!manager.init())
  {
    print("RHMesh init NOK!");
    while (1)
      ;
  }
  print("RHMesh init OK!");
}

void loop()
{
  // --- Sending Logic ---
  static unsigned long lastSendTime = 0;
  const long sendInterval = MY_NODE_ADDRESS == 1 ? 31000 : MY_NODE_ADDRESS == 2 ? 47000
                                                                                : 67000;

  if (millis() - lastSendTime > sendInterval)
  {
    char data[50];
    snprintf(data, sizeof(data), "%d!", MY_NODE_ADDRESS);

    // Alternate between Node 2 and Node 3
    switch (MY_NODE_ADDRESS)
    {
    case 1:
      lastDestNode = lastDestNode == 2 ? 3 : 2;
      break;
    case 2:
      lastDestNode = lastDestNode == 3 ? 1 : 3;
      break;
    case 3:
      lastDestNode = lastDestNode == 1 ? 2 : 1;
      break;
    }

    print("SEND " + String(MY_NODE_ADDRESS) + " -> " + String(lastDestNode) + ": " + String(data));

    // Send the message using RHMesh
    uint8_t result = manager.sendtoWait((uint8_t *)data, strlen(data), lastDestNode);
    if (result == RH_ROUTER_ERROR_NONE)
    {
      print("  -> ACKed!");
    }
    else
    {
      print("  -> FAIL: ");
      switch (result)
      {
      case RH_ROUTER_ERROR_NO_ROUTE:
        print("    -> NO_ROUTE - Dest 404");
        break;
      case RH_ROUTER_ERROR_UNABLE_TO_DELIVER:
        print("    -> UNABLE_TO_DELIVER - Next hop didn't ack");
        break;
      case RH_ROUTER_ERROR_INVALID_LENGTH:
        print("    -> INVALID_LENGTH - Message too long");
        break;
      default:
        print("    -> Unknown error: " + String(result));
        break;
      }
    }
    lastSendTime = millis();
  }

  // --- Receiving Logic ---
  // Check if a message is available for *this* node
  uint8_t len = sizeof(buf);
  uint8_t from;
  if (manager.recvfromAckTimeout(buf, &len, 2000, &from))
  {                  // Wait up to 2 seconds for a message
    buf[len] = '\0'; // Null-terminate the received data for printing as string
    print("RECEIVED " + String(MY_NODE_ADDRESS) + " <- " + String(from) + ": " + String((char *)buf));
  }
}