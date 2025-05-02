/**
 * @file htwifiv3.cpp
 * @brief Library to use WiFi with the HelTec WiFi LoRa 32 V3 Board
 *
 * Description:
 *
 * This library abstracts commands for the board peripheral WiFi Chip.
 *
 * Configuration:
 *
 * It's possible to reconfigure some parameters for the WiFi Chip by redefining them in the main file, check "=== Default Config ===" section in the header file.
 *
 * Depends On:
 * - heltecautomation/Heltec ESP32 Dev-Boards@2.0.2
 *
 * @author @joaomrsouza (João Marcos Rocha Souza)
 * https://github.com/joaomrsouza
 */

#include "htwifiv3.h"

// === Main Class ===

HTWIFIV3::HTWIFIV3()
{
  _config = nullptr;
  client = new HTWIFIV3Client();
  server = new HTWIFIV3Server();
}

HTWIFIV3::~HTWIFIV3()
{
  delete _config;
  delete client;
  delete server;
}

void HTWIFIV3::begin()
{
  _initConfig();

  if (_config->clientEnable)
    client->begin();

  if (_config->serverEnable)
    server->begin();
}

// Getters

HTWIFIV3Config *HTWIFIV3::getConfig()
{
  return _config;
}

// Setters

void HTWIFIV3::setConfig(HTWIFIV3Config *config)
{
  _config = new HTWIFIV3Config();

  _config->clientEnable = config->clientEnable;
  _config->serverEnable = config->serverEnable;
}

// Handlers

void HTWIFIV3::process()
{
  if (_config->serverEnable)
    server->handleClient();
}

// Private Handlers

void HTWIFIV3::_initConfig()
{
  if (_config != nullptr)
    return;

  _config = new HTWIFIV3Config();

  _config->clientEnable = false;
  _config->serverEnable = false;
}

// === Client Class ===

HTWIFIV3Client::HTWIFIV3Client()
{
  _config = nullptr;
  _client = new WiFiClient();
  _clientSecure = new WiFiClientSecure();
  _http = new HTTPClient();
}

HTWIFIV3Client::~HTWIFIV3Client()
{
  delete _config;
  delete _client;
  delete _clientSecure;
  delete _http;
}

void HTWIFIV3Client::begin()
{
  _initConfig();

  WiFi.mode(WIFI_STA);
  WiFi.begin(_config->ssid, _config->password);
}

// Getters

HTWIFIV3ClientConfig *HTWIFIV3Client::getConfig()
{
  return _config;
}

bool HTWIFIV3Client::getIsConnected()
{
  return WiFi.status() == WL_CONNECTED;
}

// Setters

void HTWIFIV3Client::setConfig(HTWIFIV3ClientConfig *config)
{
  _config = new HTWIFIV3ClientConfig();

  _config->ssid = config->ssid;
  _config->password = config->password;
}

void HTWIFIV3Client::setCACert(const char *caCert)
{
  _clientSecure->setCACert(caCert);
}

// Handlers

JsonDocument HTWIFIV3Client::get(String url)
{
  JsonDocument response = _setupRequest(url);

  if (response["error"])
    return response;

  return _processRequest(response, _http->GET());
}

JsonDocument HTWIFIV3Client::post(String url, JsonDocument jsonData)
{
  JsonDocument response = _setupRequest(url);
  if (response["error"])
    return response;

  String jsonString;
  serializeJson(jsonData, jsonString);

  return _processRequest(response, _http->POST(jsonString));
}

// Private Handlers

void HTWIFIV3Client::_initConfig()
{
  if (_config != nullptr)
    return;

  _config = new HTWIFIV3ClientConfig();
  _config->ssid = "HTWIFIV3 Client";
  _config->password = "12345678";
}

JsonDocument HTWIFIV3Client::_setupRequest(String url)
{
  JsonDocument response;

  response["data"] = JsonObject();
  response["error"] = false;
  response["error_message"] = "";

  if (!this->getIsConnected())
  {
    response["error"] = true;
    response["error_message"] = "WiFi not connected";

    return response;
  }

  // Check if URL starts with https
  if (url.startsWith("https://"))
    _http->begin(*_clientSecure, url);
  else
    _http->begin(*_client, url);

  _http->addHeader("Content-Type", "application/json");

  return response;
}

JsonDocument HTWIFIV3Client::_processRequest(JsonDocument response, int responseCode)
{
  if (responseCode <= 0)
  {

    response["error"] = true;
    response["error_message"] = "HTTP GET request failed: " + _http->errorToString(responseCode);

    _http->end();
    return response;
  }

  deserializeJson(response["data"], _http->getString());
  _http->end();

  return response;
}

// === Server Class ===

HTWIFIV3Server::HTWIFIV3Server()
{
  _config = nullptr;
}

HTWIFIV3Server::~HTWIFIV3Server()
{
  delete _config;
}

void HTWIFIV3Server::begin()
{
  _initConfig();

  WiFi.softAP(_config->ssid, _config->password);
}

// Getters

HTWIFIV3ServerConfig *HTWIFIV3Server::getConfig()
{
  return _config;
}

IPAddress HTWIFIV3Server::getIP()
{
  return WiFi.softAPIP();
}

// Setters

void HTWIFIV3Server::setConfig(HTWIFIV3ServerConfig *config)
{
  _config = new HTWIFIV3ServerConfig();

  _config->ssid = config->ssid;
  _config->password = config->password;
}

// Private Handlers

void HTWIFIV3Server::_initConfig()
{
  if (_config != nullptr)
    return;

  _config = new HTWIFIV3ServerConfig();
  _config->ssid = "HTWIFIV3 Server";
  _config->password = "12345678";
}
