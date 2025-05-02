/**
 * @file custom-config.cpp
 * @brief Example to use custom config for the HelTec WiFi LoRa 32 V3 Board
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

// Declare the config function
void config();

void setup()
{
  // Configure the board before starting
  config();

  // Initialize the board
  Board.begin();

  // Your code here...
}

void loop()
{
  Board.process();
  // Your code here...
}

// Custom config for multiple peripherals
void config()
{
  // === Board Config ===

  HTWLV3Config boardConfig;

  boardConfig.serialEnable = true;
  boardConfig.serialSpeed = 115200;
  boardConfig.displayEnable = true;
  boardConfig.loraEnable = false;
  boardConfig.wifiEnable = false;

  Board.setConfig(&boardConfig);

  // === LoRa Config ===

  // Make sure the LoRa is enabled before setting the config
  if (boardConfig.loraEnable)
  {
    HTLORAV3Config loraConfig;

    loraConfig.frequency = 470E6;
    loraConfig.bandwidth = 0;
    loraConfig.spreadingFactor = 7;
    loraConfig.codingRate = 1;
    loraConfig.preambleLength = 8;
    loraConfig.fixLengthPayloadOn = false;
    loraConfig.iqInversionOn = false;
    loraConfig.txOutPower = 5;
    loraConfig.txTimeout = 3000;
    loraConfig.rxTimeout = 0;

    Board.lora->setConfig(&loraConfig);
  }

  // === WiFi Config ===

  // Make sure the WiFi is enabled before setting the config
  if (boardConfig.wifiEnable)
  {
    HTWIFIV3Config wifiConfig;

    wifiConfig.clientEnable = true;
    wifiConfig.serverEnable = false;

    Board.wifi->setConfig(&wifiConfig);

    // === Client Config ===

    // Make sure the client is enabled before setting the config
    if (wifiConfig.clientEnable)
    {
      HTWIFIV3ClientConfig clientConfig;

      clientConfig.ssid = "YOUR_SSID";
      clientConfig.password = "YOUR_PASSWORD";

      Board.wifi->client->setConfig(&clientConfig);
    }

    // === Server Config ===

    // Make sure the server is enabled before setting the config
    if (wifiConfig.serverEnable)
    {
      HTWIFIV3ServerConfig serverConfig;

      serverConfig.ssid = "YOUR_SSID";
      serverConfig.password = "YOUR_PASSWORD";

      Board.wifi->server->setConfig(&serverConfig);
    }
  }
}
