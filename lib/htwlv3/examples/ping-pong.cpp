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
 * - htlorav3
 * - heltecautomation/Heltec ESP32 Dev-Boards@2.0.2
 *
 * @author @joaomrsouza (João Marcos Rocha Souza)
 * https://github.com/joaomrsouza
 */

// Include the HelTec WiFi LoRa 32 V3 Board library
#include "htwlv3.h"

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
  // Initialize the board with serial, display and LoRa enabled
  Board.begin(true, true, false, true);

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
  const char *message = String(count + 1).c_str();

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
