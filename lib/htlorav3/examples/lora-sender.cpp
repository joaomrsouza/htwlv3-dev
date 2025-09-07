/**
 * @file lora-sender.cpp
 * @brief Example to use HelTec WiFi LoRa 32 V3 Board as a sender
 *
 * Description:
 *
 * This example demonstrates how to use the HelTec WiFi LoRa 32 V3 Board as a sender.
 * It sends a message every second to the LoRa network and prints the message to the serial monitor.
 *
 * Depends On:
 * - htlorav3
 *
 * @author @joaomrsouza (João Marcos Rocha Souza)
 * https://github.com/joaomrsouza
 */

// Include the HelTec LoRa V3 library
#include "htlorav3.h"

#include <sstream>

// Declare the LoRa instance
HTLORAV3 lora = HTLORAV3();

// Declare functions to receive events from the LoRa library
void sender();
void onSendDone();
void onSendTimeout();

int count = 0;

void setup()
{
  Serial.begin(115200);
  Serial.println("LoRa Sender Example");
  Serial.println("Setting up...");

  lora.begin();

  Serial.println("LoRa: initialized.");
  Serial.print("Freq: ");
  Serial.println(lora.getConfig().frequency);

  // Set the callback functions for the LoRa library. These callbacks are optional
  lora.setOnSendDone(onSendDone);
  lora.setOnSendTimeout(onSendTimeout);

  Serial.println("Setup complete");
}

void loop()
{
  sender();
  lora.process();
}

void sender()
{
  std::ostringstream data;

  data << "Packet " << count++;

  const char *message = data.str().c_str();

  Serial.print("Sending: ");
  Serial.println(message);

  // Send the message to the LoRa network
  lora.sendPacket(message);

  delay(1000);
}

void onSendDone()
{
  Serial.println("Send done");
}

void onSendTimeout()
{
  Serial.println("Send timeout");
}
