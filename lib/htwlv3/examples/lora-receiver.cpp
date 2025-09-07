/**
 * @file lora-receiver.cpp
 * @brief Example to use HelTec WiFi LoRa 32 V3 Board as a receiver
 *
 * Description:
 *
 * This example demonstrates how to use the HelTec WiFi LoRa 32 V3 Board as a receiver.
 * It listens for incoming messages and prints them to the serial monitor and displays them on the OLED screen.
 *
 * Depends On:
 * - htwlv3
 *
 * @author @joaomrsouza (João Marcos Rocha Souza)
 * https://github.com/joaomrsouza
 */

// Include the HelTec WiFi LoRa 32 V3 Board library
#include "htwlv3.h"

// Declare the config function
void config();

// Declare functions to receive events from the LoRa library
void receiver();
void onReceive(LoraDataPacket packet);

void setup()
{
  // Configure the board before starting
  config();

  // Initialize the board
  Board.begin();

  Board.println("LoRa Receiver Example");
  Board.println("Setting up...");

  // Set the callback function for the LoRa library to receive packets
  Board.lora->setOnReceive(onReceive);

  Board.println("Setup complete");
}

void loop()
{
  receiver();
  Board.process();
}

void receiver()
{
  Board.lora->listenToPacket();
}

void onReceive(LoraDataPacket packet)
{
  Board.println("Received Data:");
  Board.println(packet.data);
  Board.print("Size: ");
  Board.println(packet.size);
  Board.print("RSSI: ");
  Board.println(packet.rssi);
  Board.print("SNR: ");
  Board.println(packet.snr);
}

void config()
{
  HTWLV3Config boardConfig = HTWLV3::getDefaultConfig();

  boardConfig.serialEnable = true;
  boardConfig.displayEnable = true;
  boardConfig.loraEnable = true;

  Board.setConfig(boardConfig);
}
