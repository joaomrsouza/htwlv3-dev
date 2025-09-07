/**
 * @file ping-pong.cpp
 * @brief Example to use HelTec WiFi LoRa 32 V3 Board as a ping-pong sender and receiver
 *
 * Description:
 *
 * This example demonstrates how to use the HelTec WiFi LoRa 32 V3 Board as a ping-pong sender and receiver.
 * It sends and receives messages between two boards incrementing the count and printing it on the serial monitor.
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
void sender(int count);
void receiver();
void onReceive(LoraDataPacket packet);
void onSendDone();
void onSendTimeout();

// Define states for the ping-pong communication
#define STATE_SEND 0
#define STATE_RECEIVE 1
#define STATE_WAIT 2

int state = STATE_SEND;
int count = 0;

void setup()
{
  // Configure the board before starting
  config();

  // Initialize the board
  Board.begin();

  Board.println("Ping-Pong Example");
  Board.println("Setting up...");

  // Set the callback functions for the LoRa library to receive and send packets
  Board.lora->setOnReceive(onReceive);
  Board.lora->setOnSendDone(onSendDone);
  Board.lora->setOnSendTimeout(onSendTimeout);

  Board.println("Setup complete");
}

void loop()
{
  // Prefer use callbacks and state machines to handle the communication
  switch (state)
  {
  case STATE_SEND:
    sender(count);
    state = STATE_WAIT;
    break;
  case STATE_RECEIVE:
    receiver();
    state = STATE_WAIT;
    break;
  case STATE_WAIT:
    Board.process();
    break;
  }
}

void sender(int count)
{
  delay(1000);

  String str = String(count + 1);
  const char *message = str.c_str();

  Board.print("Sending: ");
  Board.println(message);

  // Send the message to the LoRa network
  Board.lora->sendPacket(message);
}

// Callback function for when the message is sent
void onSendDone()
{
  Board.println("Send done");
  state = STATE_RECEIVE;
}

void onSendTimeout()
{
  Board.println("Send timeout");
  state = STATE_SEND;
}

void receiver()
{
  Board.lora->listenToPacket();
}

void onReceive(LoraDataPacket packet)
{
  Board.print("Received Data: ");
  Board.println(packet.data);

  count = atoi(packet.data);
  state = STATE_SEND;
}

void config()
{
  HTWLV3Config boardConfig = HTWLV3::getDefaultConfig();

  boardConfig.serialEnable = true;
  boardConfig.displayEnable = true;
  boardConfig.loraEnable = true;

  Board.setConfig(boardConfig);
}
