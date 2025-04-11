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
 * - htlorav3
 * - heltecautomation/Heltec ESP32 Dev-Boards@2.0.2
 *
 * @author @joaomrsouza (João Marcos Rocha Souza)
 * https://github.com/joaomrsouza
 */

// Include the HelTec WiFi LoRa 32 V3 Board library
#include "htwlv3.h"

// Declare functions to receive events from the LoRa library
void receiver();
void onReceive(LoraDataPacket packet);

void setup()
{
  // Initialize the board with serial, display and LoRa enabled
  Board.begin(true, true, false, true);

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
  Board.println(String(packet.size).c_str());
  Board.print("RSSI: ");
  Board.println(String(packet.rssi).c_str());
  Board.print("SNR: ");
  Board.println(String(packet.snr).c_str());
}
