#include "htwlv3.h"
#include "htezstv3.h"

void config();

void setup()
{
  config();
  Board.begin();

  EZSettings::begin();
}

void loop()
{
  Board.process();
}

void config()
{
  // === Board Config ===

  HTWLV3Config boardConfig = HTWLV3::getDefaultConfig();

  boardConfig.wifiEnable = true;

  Board.setConfig(boardConfig);

  // === WiFi Config ===

  HTWIFIV3Config wifiConfig = HTWIFIV3::getDefaultConfig();

  wifiConfig.serverEnable = true;

  Board.wifi->setConfig(wifiConfig);

  // === Server Config ===

  HTWIFIV3ServerConfig serverConfig;

  serverConfig.ssid = "ESP";
  serverConfig.password = "12345678";

  Board.wifi->server->setConfig(serverConfig);
}
