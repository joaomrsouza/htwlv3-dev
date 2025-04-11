/**
 * @file lora-sender.cpp
 * @brief Example to use HelTec WiFi LoRa 32 V3 Board as a sender
 *
 * Description:
 *
 * This example demonstrates how to use the HelTec WiFi LoRa 32 V3 Board as a sender.
 * It sends a message every second to the LoRa network.
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
void sender();
void onSendDone();
void onSendTimeout();

int count = 0;

void setup()
{
  // Initialize the board with serial, display and LoRa enabled
  Board.begin(true, true, false, true);

  Board.println("LoRa Sender Example");
  Board.println("Setting up...");

  // Set the callback functions for the LoRa library. These callbacks are optional
  Board.lora->setOnSendDone(onSendDone);
  Board.lora->setOnSendTimeout(onSendTimeout);

  Board.println("Setup complete");
}

void loop()
{
  sender();
  Board.process();
}

void sender()
{
  std::ostringstream data;

  data << "Packet " << count++;

  const char *message = data.str().c_str();

  Board.print("Sending: ");
  Board.println(message);

  // Send the message to the LoRa network
  Board.lora->sendPacket(message);

  delay(1000);
}

void onSendDone()
{
  Board.println("Send done");
}

void onSendTimeout()
{
  Board.println("Send timeout");
}
