/**
 * @file lora-receiver.cpp
 * @brief Example to use HelTec WiFi LoRa 32 V3 Board as a receiver
 *
 * Description:
 *
 * This example demonstrates how to use the HelTec WiFi LoRa 32 V3 Board as a receiver.
 * It listens for incoming messages and prints them to the serial monitor.
 *
 * Depends On:
 * - htlorav3
 *
 * @author @joaomrsouza (João Marcos Rocha Souza)
 * https://github.com/joaomrsouza
 */

// Include the HelTec LoRa V3 library
#include "htlorav3.h"

// Declare the LoRa instance
HTLORAV3 lora = HTLORAV3();

// Declare functions to receive events from the LoRa library
void receiver();
void onReceive(LoraDataPacket packet);

void setup()
{
  Serial.begin(115200);
  Serial.println("LoRa Receiver Example");
  Serial.println("Setting up...");

  lora.begin();

  Serial.println("LoRa: initialized.");
  Serial.print("Freq: ");
  Serial.println(lora.getConfig().frequency);

  // Set the callback function for the LoRa library to receive packets
  lora.setOnReceive(onReceive);

  Serial.println("Setup complete");
}

void loop()
{
  receiver();
  lora.process();
}

void receiver()
{
  lora.listenToPacket();
}

void onReceive(LoraDataPacket packet)
{
  Serial.println("Received Data:");
  Serial.println(packet.data);
  Serial.print("Size: ");
  Serial.println(packet.size);
  Serial.print("RSSI: ");
  Serial.println(packet.rssi);
  Serial.print("SNR: ");
  Serial.println(packet.snr);
}
