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
 * It's possible to configure some parameters for the WiFi Chip by using the `setConfig()` and `updateConfig()` methods.
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
  _config = getDefaultConfig();
  client = new HTWIFIV3Client();
  server = new HTWIFIV3Server();
}

HTWIFIV3::~HTWIFIV3()
{
  stop();
  delete client;
  delete server;
}

void HTWIFIV3::begin()
{
  _initializeWiFi(true);
}

void HTWIFIV3::stop()
{
  if (_config.clientEnable)
    client->stop();

  if (_config.serverEnable)
    server->stop();
}

// Getters

HTWIFIV3Config HTWIFIV3::getConfig() const
{
  return _config;
}

HTWIFIV3Config HTWIFIV3::getDefaultConfig()
{
  HTWIFIV3Config defaultConfig;

  defaultConfig.clientEnable = false;
  defaultConfig.serverEnable = false;

  return defaultConfig;
}

// Setters

void HTWIFIV3::setConfig(const HTWIFIV3Config &config)
{
  _config = config;
}

void HTWIFIV3::updateConfig(const HTWIFIV3Config &config)
{
  setConfig(config);

  _initializeWiFi();
}

// Handlers

void HTWIFIV3::process()
{
  if (_config.serverEnable)
    server->handleClient();
}

// Private Handlers

void HTWIFIV3::_initializeWiFi(bool force)
{
  if (_config.clientEnable)
  {
    if (!force && client != nullptr)
      return;

    if (client == nullptr)
      client = new HTWIFIV3Client();

    client->begin();
  }
  else if (client != nullptr)
  {
    delete client;
    client = nullptr;
  }

  if (_config.serverEnable)
  {
    if (!force && server != nullptr)
      return;

    if (server == nullptr)
      server = new HTWIFIV3Server();

    server->begin();
  }
  else if (server != nullptr)
  {
    delete server;
    server = nullptr;
  }
}

// === Client Class ===

HTWIFIV3Client::HTWIFIV3Client()
{
  _config = getDefaultConfig();
  _client = new WiFiClient();
  _clientSecure = new WiFiClientSecure();
  _http = new HTTPClient();
}

HTWIFIV3Client::~HTWIFIV3Client()
{
  stop();
  delete _client;
  delete _clientSecure;
  delete _http;
}

void HTWIFIV3Client::begin()
{
  _initializeWiFiClient();
}

void HTWIFIV3Client::stop()
{
  WiFi.disconnect(true, true);
}

// Getters

HTWIFIV3ClientConfig HTWIFIV3Client::getConfig() const
{
  return _config;
}

HTWIFIV3ClientConfig HTWIFIV3Client::getDefaultConfig()
{
  HTWIFIV3ClientConfig defaultConfig;

  defaultConfig.ssid = "HTWIFIV3 Client";
  defaultConfig.password = "12345678";

  return defaultConfig;
}

bool HTWIFIV3Client::getIsConnected()
{
  return WiFi.status() == WL_CONNECTED;
}

// Setters

void HTWIFIV3Client::setConfig(const HTWIFIV3ClientConfig &config)
{
  _config = config;
}

void HTWIFIV3Client::updateConfig(const HTWIFIV3ClientConfig &config)
{
  stop();

  setConfig(config);

  _initializeWiFiClient();
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

void HTWIFIV3Client::_initializeWiFiClient()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(_config.ssid, _config.password);
}

JsonDocument HTWIFIV3Client::_setupRequest(String url)
{
  JsonDocument response;

  response["data"] = JsonObject();
  response["status"] = NULL;
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
  response["status"] = responseCode;

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

HTWIFIV3Server::HTWIFIV3Server() : WebServer(80)
{
  _config = getDefaultConfig();
}

HTWIFIV3Server::~HTWIFIV3Server()
{
  stop();
}

void HTWIFIV3Server::begin()
{
  _initializeWiFiServer();
}

void HTWIFIV3Server::stop()
{
  WebServer::stop();
  WiFi.softAPdisconnect(true);
}

// Getters

HTWIFIV3ServerConfig HTWIFIV3Server::getConfig() const
{
  return _config;
}

HTWIFIV3ServerConfig HTWIFIV3Server::getDefaultConfig()
{
  HTWIFIV3ServerConfig defaultConfig;

  defaultConfig.ssid = "HTWIFIV3 Server";
  defaultConfig.password = "12345678";

  return defaultConfig;
}

IPAddress HTWIFIV3Server::getIP()
{
  return WiFi.softAPIP();
}

// Setters

void HTWIFIV3Server::setConfig(const HTWIFIV3ServerConfig &config)
{
  _config = config;
}

void HTWIFIV3Server::updateConfig(const HTWIFIV3ServerConfig &config)
{
  stop();

  setConfig(config);

  _initializeWiFiServer();
}

// Private Handlers

void HTWIFIV3Server::_initializeWiFiServer()
{
  WiFi.softAP(_config.ssid, _config.password);
  WebServer::begin();
}