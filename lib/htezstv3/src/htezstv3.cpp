/**
 * @file htezstv3.cpp
 * @brief Easy Settings Plugin for HelTec WiFi LoRa 32 V3 Board
 *
 * Description:
 *
 * This library provides a simple web interface for configuring board settings including
 * Serial, Display, LoRa, and WiFi parameters through a responsive HTML interface.
 *
 * Configuration:
 *
 * The library automatically enables WiFi and creates a web server. Access the settings
 * page at http://{board_ip}/settings to configure all board peripherals.
 *
 * Depends On:
 * - htwlv3
 *
 * @author @joaomrsouza (João Marcos Rocha Souza)
 * https://github.com/joaomrsouza
 */

#include "htezstv3.h"

// === Main Function ===

void EZSettings::begin()
{
  HTWLV3Config boardConfig = Board.getConfig();

  if (!boardConfig.wifiEnable)
  {
    boardConfig.wifiEnable = true;
    Board.updateConfig(boardConfig);
  }

  HTWIFIV3Config wifiConfig = Board.wifi->getConfig();

  if (!wifiConfig.serverEnable)
  {
    wifiConfig.serverEnable = true;
    Board.wifi->updateConfig(wifiConfig);
  }

  Board.wifi->server->on("/settings", HTTP_GET, EZSettings::detail::handleRoot);
  Board.wifi->server->on("/settings/save", HTTP_POST, EZSettings::detail::handleSave);

  Board.println("EZSettings initialized");
  Board.println("Server running on http://" + Board.wifi->server->getIP().toString() + "/settings");
}

// === Detail Functions ===

void EZSettings::detail::handleRoot()
{
  HTWLV3Config boardConfig = Board.getConfig();

  String page = replaceAll(getBaseTemplate(), "{PAGE_CONTENT}", getSettingsTemplate());

  // === Board Config ===

  page = replaceAll(page, "{LABEL_DISPLAY_ENABLE}", boardConfig.displayEnable ? "enabled" : "disabled");
  page = replaceAll(page, "{CHECKED_DISPLAY_ENABLE}", boardConfig.displayEnable ? "checked" : "");

  page = replaceAll(page, "{LABEL_SERIAL_ENABLE}", boardConfig.serialEnable ? "enabled" : "disabled");
  page = replaceAll(page, "{CHECKED_SERIAL_ENABLE}", boardConfig.serialEnable ? "checked" : "");

  page = replaceAll(page, "{LABEL_SERIAL_SPEED}", String(boardConfig.serialSpeed));
  page = replaceAll(page, "{SELECTED_SERIAL_SPEED-4800}", boardConfig.serialSpeed == 4800 ? "selected" : "");
  page = replaceAll(page, "{SELECTED_SERIAL_SPEED-9600}", boardConfig.serialSpeed == 9600 ? "selected" : "");
  page = replaceAll(page, "{SELECTED_SERIAL_SPEED-19200}", boardConfig.serialSpeed == 19200 ? "selected" : "");
  page = replaceAll(page, "{SELECTED_SERIAL_SPEED-38400}", boardConfig.serialSpeed == 38400 ? "selected" : "");
  page = replaceAll(page, "{SELECTED_SERIAL_SPEED-57600}", boardConfig.serialSpeed == 57600 ? "selected" : "");
  page = replaceAll(page, "{SELECTED_SERIAL_SPEED-115200}", boardConfig.serialSpeed == 115200 ? "selected" : "");

  // === Lora Config ===

  page = replaceAll(page, "{LABEL_LORA_ENABLE}", boardConfig.loraEnable ? "enabled" : "disabled");
  page = replaceAll(page, "{CHECKED_LORA_ENABLE}", boardConfig.loraEnable ? "checked" : "");

  HTLORAV3Config loraConfig = boardConfig.loraEnable ? Board.lora->getConfig() : HTLORAV3::getDefaultConfig();

  String freqLabel = String(loraConfig.frequency);
  freqLabel = freqLabel.substring(0, freqLabel.length() - 9) + " MHz";
  page = replaceAll(page, "{LABEL_LORA_FREQUENCY}", freqLabel);
  page = replaceAll(page, "{SELECTED_LORA_FREQUENCY-433000000}", loraConfig.frequency == 433000000 ? "selected" : "");
  page = replaceAll(page, "{SELECTED_LORA_FREQUENCY-470000000}", loraConfig.frequency == 470000000 ? "selected" : "");
  page = replaceAll(page, "{SELECTED_LORA_FREQUENCY-868000000}", loraConfig.frequency == 868000000 ? "selected" : "");
  page = replaceAll(page, "{SELECTED_LORA_FREQUENCY-915000000}", loraConfig.frequency == 915000000 ? "selected" : "");

  String bwLabel = String(loraConfig.bandwidth);
  if (bwLabel == "0")
    bwLabel = "125 kHz";
  else if (bwLabel == "1")
    bwLabel = "250 kHz";
  else if (bwLabel == "2")
    bwLabel = "500 kHz";
  else
    bwLabel = "Reserved";
  page = replaceAll(page, "{LABEL_LORA_BANDWIDTH}", bwLabel);
  for (int i = 0; i < 4; i++)
    page = replaceAll(page, "{SELECTED_LORA_BANDWIDTH-" + String(i) + "}", loraConfig.bandwidth == i ? "selected" : "");

  page = replaceAll(page, "{LABEL_LORA_SPREADING_FACTOR}", String(loraConfig.spreadingFactor));
  for (int i = 7; i <= 12; i++)
    page = replaceAll(page, "{SELECTED_LORA_SPREADING_FACTOR-" + String(i) + "}", loraConfig.spreadingFactor == i ? "selected" : "");

  String crLabel = String(loraConfig.codingRate);
  if (crLabel == "1")
    crLabel = "4/5";
  else if (crLabel == "2")
    crLabel = "4/6";
  else if (crLabel == "3")
    crLabel = "4/7";
  else
    crLabel = "4/8";
  page = replaceAll(page, "{LABEL_LORA_CODING_RATE}", crLabel);
  for (int i = 1; i <= 4; i++)
    page = replaceAll(page, "{SELECTED_LORA_CODING_RATE-" + String(i) + "}", loraConfig.codingRate == i ? "selected" : "");

  page = replaceAll(page, "{LABEL_LORA_PREAMBLE_LENGTH}", String(loraConfig.preambleLength));
  for (int i = 6; i <= 20; i++)
    page = replaceAll(page, "{SELECTED_LORA_PREAMBLE_LENGTH-" + String(i) + "}", loraConfig.preambleLength == i ? "selected" : "");

  page = replaceAll(page, "{LABEL_LORA_FIX_LENGTH_PAYLOAD}", loraConfig.fixLengthPayloadOn ? "enabled" : "disabled");
  page = replaceAll(page, "{CHECKED_LORA_FIX_LENGTH_PAYLOAD}", loraConfig.fixLengthPayloadOn ? "checked" : "");

  page = replaceAll(page, "{LABEL_LORA_IQ_INVERSION}", loraConfig.iqInversionOn ? "enabled" : "disabled");
  page = replaceAll(page, "{CHECKED_LORA_IQ_INVERSION}", loraConfig.iqInversionOn ? "checked" : "");

  page = replaceAll(page, "{LABEL_LORA_TX_OUT_POWER}", String(loraConfig.txOutPower));
  for (int i = 0; i <= 20; i++)
    page = replaceAll(page, "{SELECTED_LORA_TX_OUT_POWER-" + String(i) + "}", loraConfig.txOutPower == i ? "selected" : "");

  page = replaceAll(page, "{VALUE_LORA_TX_TIMEOUT}", String(loraConfig.txTimeout));
  page = replaceAll(page, "{VALUE_LORA_RX_TIMEOUT}", String(loraConfig.rxTimeout));

  // === WiFi Config ===

  HTWIFIV3Config wifiConfig = boardConfig.wifiEnable ? Board.wifi->getConfig() : HTWIFIV3::getDefaultConfig();

  // === WiFi - Client Config ===

  HTWIFIV3ClientConfig clientConfig = wifiConfig.clientEnable ? Board.wifi->client->getConfig() : HTWIFIV3Client::getDefaultConfig();

  page = replaceAll(page, "{LABEL_CLIENT_ENABLE}", wifiConfig.clientEnable ? "enabled" : "disabled");
  page = replaceAll(page, "{CHECKED_CLIENT_ENABLE}", wifiConfig.clientEnable ? "checked" : "");

  page = replaceAll(page, "{VALUE_CLIENT_SSID}", clientConfig.ssid);
  page = replaceAll(page, "{VALUE_CLIENT_PASSWORD}", clientConfig.password);

  // === WiFi - Server Config ===

  HTWIFIV3ServerConfig serverConfig = wifiConfig.serverEnable ? Board.wifi->server->getConfig() : HTWIFIV3Server::getDefaultConfig();

  page = replaceAll(page, "{LABEL_SERVER_ENABLE}", wifiConfig.serverEnable ? "enabled" : "disabled");
  page = replaceAll(page, "{CHECKED_SERVER_ENABLE}", wifiConfig.serverEnable ? "checked" : "");

  page = replaceAll(page, "{VALUE_SERVER_SSID}", serverConfig.ssid);
  page = replaceAll(page, "{VALUE_SERVER_PASSWORD}", serverConfig.password);

  Board.wifi->server->send(200, "text/html", page);
}

void EZSettings::detail::handleSave()
{
  bool displayEnable = Board.wifi->server->arg("display-enable").equals("on") ? true : false;
  bool serialEnable = Board.wifi->server->arg("serial-enable").equals("on") ? true : false;
  int serialSpeed = Board.wifi->server->arg("serial-speed").toInt();

  bool loraEnable = Board.wifi->server->arg("lora-enable").equals("on") ? true : false;
  double loraFrequency = Board.wifi->server->arg("lora-frequency").toDouble();
  int loraBandwidth = Board.wifi->server->arg("lora-bandwidth").toInt();
  int loraSpreadingFactor = Board.wifi->server->arg("lora-spreading-factor").toInt();
  int loraCodingRate = Board.wifi->server->arg("lora-coding-rate").toInt();
  int loraPreambleLength = Board.wifi->server->arg("lora-preamble-length").toInt();
  bool loraFixLengthPayload = Board.wifi->server->arg("lora-fix-length-payload").equals("on") ? true : false;
  bool loraIQInversion = Board.wifi->server->arg("lora-iq-inversion").equals("on") ? true : false;
  int loraTxOutPower = Board.wifi->server->arg("lora-tx-out-power").toInt();
  int loraTxTimeout = Board.wifi->server->arg("lora-tx-timeout").toInt();
  int loraRxTimeout = Board.wifi->server->arg("lora-rx-timeout").toInt();

  bool clientEnable = Board.wifi->server->arg("client-enable").equals("on") ? true : false;
  String clientSSID = Board.wifi->server->arg("client-ssid");
  String clientPassword = Board.wifi->server->arg("client-password");

  bool serverEnable = Board.wifi->server->arg("server-enable").equals("on") ? true : false;
  String serverSSID = Board.wifi->server->arg("server-ssid");
  String serverPassword = Board.wifi->server->arg("server-password");

  String pageContent = getSaveTemplate();

  String page = replaceAll(getBaseTemplate(), "{PAGE_CONTENT}", pageContent);

  Board.wifi->server->send(200, "text/html", page);

  HTWLV3Config boardConfig = Board.getConfig();

  // === Board Config ===

  boardConfig.displayEnable = displayEnable;
  boardConfig.serialEnable = serialEnable;
  boardConfig.serialSpeed = serialSpeed;
  boardConfig.loraEnable = loraEnable;
  boardConfig.wifiEnable = clientEnable || serverEnable;

  Board.updateConfig(boardConfig);

  // === Lora Config ===

  if (boardConfig.loraEnable)
  {
    HTLORAV3Config loraConfig = Board.lora->getConfig();

    loraConfig.frequency = loraFrequency;
    loraConfig.bandwidth = loraBandwidth;
    loraConfig.spreadingFactor = loraSpreadingFactor;
    loraConfig.codingRate = loraCodingRate;
    loraConfig.preambleLength = loraPreambleLength;
    loraConfig.fixLengthPayloadOn = loraFixLengthPayload;
    loraConfig.iqInversionOn = loraIQInversion;
    loraConfig.txOutPower = loraTxOutPower;
    loraConfig.txTimeout = loraTxTimeout;
    loraConfig.rxTimeout = loraRxTimeout;

    Board.lora->updateConfig(loraConfig);
  }

  // === WiFi Config ===

  if (boardConfig.wifiEnable)
  {
    HTWIFIV3Config wifiConfig = Board.wifi->getConfig();

    wifiConfig.clientEnable = clientEnable;
    wifiConfig.serverEnable = serverEnable;

    Board.wifi->updateConfig(wifiConfig);

    if (wifiConfig.clientEnable)
    {

      HTWIFIV3ClientConfig clientConfig = Board.wifi->client->getConfig();
      clientConfig.ssid = clientSSID;
      clientConfig.password = clientPassword;

      Board.wifi->client->updateConfig(clientConfig);
    }

    if (wifiConfig.serverEnable)
    {
      HTWIFIV3ServerConfig serverConfig = Board.wifi->server->getConfig();
      serverConfig.ssid = serverSSID;
      serverConfig.password = serverPassword;

      Board.wifi->server->updateConfig(serverConfig);
    }
  }
}

String EZSettings::detail::replaceAll(String str, String find, String replace)
{
  int index = 0;
  while (true)
  {
    index = str.indexOf(find, index);
    if (index == -1)
      break;
    str.replace(find, replace);
    index += replace.length();
  }
  return str;
}