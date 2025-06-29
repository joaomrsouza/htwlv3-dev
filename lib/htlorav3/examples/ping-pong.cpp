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

// Include the HelTec LoRa V3 library
#include "htlorav3.h"

// Declare the LoRa instance
HTLORAV3 lora = HTLORAV3();

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
  Serial.begin(115200);
  Serial.println("Ping-Pong Example");
  Serial.println("Setting up...");

  lora.begin();

  Serial.println("LoRa: initialized.");
  Serial.print("Freq: ");
  Serial.println(lora.getConfig()->frequency);

  // Set the callback functions for the LoRa library to receive and send packets
  lora.setOnReceive(onReceive);
  lora.setOnSendDone(onSendDone);
  lora.setOnSendTimeout(onSendTimeout);

  Serial.println("Setup complete");
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
    lora.process();
    break;
  }
}

void sender(int count)
{
  delay(1000);

  String str = String(count + 1);
  const char *message = str.c_str();

  Serial.print("Sending: ");
  Serial.println(message);

  // Send the message to the LoRa network
  lora.sendPacket(message);
}

// Callback function for when the message is sent
void onSendDone()
{
  Serial.println("Send done");
  state = STATE_RECEIVE;
}

void onSendTimeout()
{
  Serial.println("Send timeout");
  state = STATE_SEND;
}

void receiver()
{
  lora.listenToPacket();
}

void onReceive(LoraDataPacket packet)
{
  Serial.print("Received Data: ");
  Serial.println(packet.data);

  count = atoi(packet.data);
  state = STATE_SEND;
}
