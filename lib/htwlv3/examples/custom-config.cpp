/**
 * @file custom-config.cpp
 * @brief Example to use custom config for the HelTec WiFi LoRa 32 V3 Board
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
  // All the available default configs are commented and already set, change only what you need

  // === Board Config ===

  HTWLV3Config boardConfig = HTWLV3::getDefaultConfig();

  // boardConfig.serialEnable = false;
  // boardConfig.serialSpeed = 115200;
  // boardConfig.displayEnable = false;
  boardConfig.loraEnable = true;
  boardConfig.wifiEnable = true;

  Board.setConfig(boardConfig);

  // === LoRa Config ===

  HTLORAV3Config loraConfig = HTLORAV3::getDefaultConfig();

  // loraConfig.frequency = 915E6;
  // loraConfig.bandwidth = 0;
  // loraConfig.spreadingFactor = 7;
  // loraConfig.codingRate = 1;
  // loraConfig.preambleLength = 8;
  // loraConfig.fixLengthPayloadOn = false;
  // loraConfig.iqInversionOn = false;
  loraConfig.txOutPower = 12;
  // loraConfig.txTimeout = 3000;
  // loraConfig.rxTimeout = 0;

  Board.lora->setConfig(loraConfig);

  // === WiFi Config ===

  HTWIFIV3Config wifiConfig = HTWIFIV3::getDefaultConfig();

  wifiConfig.clientEnable = true;
  // wifiConfig.serverEnable = false;

  Board.wifi->setConfig(wifiConfig);

  // === Client Config ===

  HTWIFIV3ClientConfig clientConfig = HTWIFIV3Client::getDefaultConfig();

  clientConfig.ssid = "YOUR_SSID";
  clientConfig.password = "YOUR_PASSWORD";

  Board.wifi->client->setConfig(clientConfig);

  // You don't need to set a config if you don't want to use it

  // === Server Config ===

  // HTWIFIV3ServerConfig serverConfig = HTWIFIV3Server::getDefaultConfig();

  // serverConfig.ssid = "YOUR_SSID";
  // serverConfig.password = "YOUR_PASSWORD";

  // Board.wifi->server->setConfig(serverConfig);
}
