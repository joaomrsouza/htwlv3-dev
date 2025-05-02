/**
 * @file custom-config.cpp
 * @brief Example to use custom config for the LoRa Chip
 *
 * Depends On:
 * - htlorav3
 * - heltecautomation/Heltec ESP32 Dev-Boards@2.0.2
 *
 * @author @joaomrsouza (João Marcos Rocha Souza)
 * https://github.com/joaomrsouza
 *
 */

// Include the HelTec LoRa V3 library
#include "htlorav3.h"

// Declare the LoRa instance
HTLORAV3 lora = HTLORAV3();

// Declare the config function
void config();

void setup()
{
  // Apply the custom config before starting the LoRa Chip
  config();
  lora.begin();

  // your code here...
}

void loop()
{
  lora.process();
  // your code here...
}

// Custom config function
void config()
{
  HTLORAV3Config config;

  // Set the custom config
  config.frequency = 470E6;
  config.bandwidth = 0;
  config.spreadingFactor = 7;
  config.codingRate = 1;
  config.preambleLength = 8;
  config.fixLengthPayloadOn = false;
  config.iqInversionOn = false;
  config.txOutPower = 5;
  config.txTimeout = 3000;
  config.rxTimeout = 0;

  // Apply the custom config
  lora.setConfig(&config);
}
