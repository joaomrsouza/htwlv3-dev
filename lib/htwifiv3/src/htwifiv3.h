/**
 * @file htwifiv3.h
 * @brief Library to use WiFi with the HelTec WiFi LoRa 32 V3 Board
 *
 * Description:
 *
 * This library abstracts commands for the board peripheral WiFi Chip.
 *
 * Configuration:
 *
 * It's possible to configure some parameters by using the `setConfig()` and `updateConfig()` methods.
 *
 * Depends On:
 * - heltecautomation/Heltec ESP32 Dev-Boards@2.0.2
 *
 * @author @joaomrsouza (João Marcos Rocha Souza)
 * https://github.com/joaomrsouza
 */

#ifndef HTWIFIV3_H
#define HTWIFIV3_H

// WiFi Client Libs
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// WiFi Server Libs
#include <WiFi.h>
#include <WebServer.h>

// === Structs ===

typedef struct
{
  bool clientEnable;
  bool serverEnable;
} HTWIFIV3Config;

typedef struct
{
  String ssid;
  String password;
} HTWIFIV3ClientConfig;

typedef struct
{
  String ssid;
  String password;
} HTWIFIV3ServerConfig;

// === Classes ===

/**
 * @class HTWIFIV3Client
 * @brief A class to manage WiFi communication as a client
 *
 * This class provide methods to handle WiFi communication as a client.
 *
 * @note Capabilities:
 * - Connect to a WiFi Network
 * - Send and Receive Data to/from a Server
 */
class HTWIFIV3Client
{
public:
  HTWIFIV3Client();
  ~HTWIFIV3Client();

  void begin();
  void stop();

  // === Getters ===

  /**
   * @brief Get the current config object
   *
   * @return HTWIFIV3ClientConfig
   */
  HTWIFIV3ClientConfig getConfig() const;

  /**
   * @brief Get if the WiFi is connected
   *
   * @return bool
   */
  bool getIsConnected();

  /**
   * @brief Get the default configuration object
   *
   * @return HTWIFIV3ClientConfig Default configuration with standard values
   */
  static HTWIFIV3ClientConfig getDefaultConfig();

  // === Setters ===

  /**
   * @brief Set the config object
   *
   * @param config Config object
   *
   * @warning Do not use this to change the configuration after the client is initialized, use `updateConfig()` instead
   */
  void setConfig(const HTWIFIV3ClientConfig &config);

  /**
   * @brief Update configuration after the client is initialized
   *
   * @param config New configuration
   *
   * @warning Do not use this to change the configuration before the client is initialized, use `setConfig()` instead
   *
   * @note Attention points:
   * - This function will disconnect the client
   * - This function will reset the client
   * - The client should reconnect to the network after the configuration is updated
   */
  void updateConfig(const HTWIFIV3ClientConfig &config);

  /**
   * @brief Set the SSL config object
   *
   * @param config SSL config object
   */
  void setCACert(const char *caCert);

  // === Handlers ===

  /**
   * @brief Make a GET request to a server
   *
   * @param url URL to make the GET request
   * @return JsonDocument Response from the server
   */
  JsonDocument get(String url);

  /**
   * @brief Make a POST request to a server
   *
   * @param url URL to make the POST request
   * @param jsonData JSON data to send in the request
   * @return JsonDocument Response from the server
   */
  JsonDocument post(String url, JsonDocument jsonData);

private:
  /**
   * @brief Config object
   */
  HTWIFIV3ClientConfig _config;

  /**
   * @brief WiFiClient class to work with the WiFi Chip as a client
   */
  WiFiClient *_client;

  /**
   * @brief WiFiClientSecure class to work with the WiFi Chip as a client with SSL
   */
  WiFiClientSecure *_clientSecure;

  /**
   * @brief HTTPClient class to work with the WiFi Chip as a client
   */
  HTTPClient *_http;

  // === Private Handlers ===

  /**
   * @brief Initialize the WiFi client
   */
  void _initializeWiFiClient();

  /**
   * @brief Setup the request
   *
   * @param url URL to make the request
   * @return JsonDocument Response from the server
   */
  JsonDocument _setupRequest(String url);

  /**
   * @brief Process the request
   *
   * @param response Response from the server
   * @param responseCode Response code from the server
   * @return JsonDocument Response from the server
   */
  JsonDocument _processRequest(JsonDocument response, int responseCode);
};

/**
 * @class HTWIFIV3Server
 * @brief A class to manage WiFi communication as a server
 *
 * This class provide methods to handle WiFi communication as a server.
 *
 * @note Capabilities:
 * - Create a WiFi Server
 * - Handle incoming connections
 * - Send and Receive Data to/from a Client
 */
class HTWIFIV3Server : public WebServer
{
public:
  HTWIFIV3Server();
  ~HTWIFIV3Server();

  void begin();
  void stop();

  // === Getters ===

  /**
   * @brief Get the current config object
   *
   * @return HTWIFIV3ServerConfig
   */
  HTWIFIV3ServerConfig getConfig() const;

  /**
   * @brief Get the IP address of the server
   *
   * @return IPAddress
   */
  IPAddress getIP();

  /**
   * @brief Get the default configuration object
   *
   * @return HTWIFIV3ServerConfig Default configuration with standard values
   */
  static HTWIFIV3ServerConfig getDefaultConfig();

  // === Setters ===

  /**
   * @brief Set the config object
   *
   * @param config Config object
   */
  void setConfig(const HTWIFIV3ServerConfig &config);

  /**
   * @brief Update configuration after the server is initialized
   *
   * @param config New configuration
   *
   * @warning Do not use this to change the configuration before the server is initialized, use `setConfig()` instead
   *
   * @note Attention points:
   * - This function will stop the server
   * - The server should be started again after the configuration is updated
   */
  void updateConfig(const HTWIFIV3ServerConfig &config);

  // === Handlers ===

private:
  /**
   * @brief Config object
   */
  HTWIFIV3ServerConfig _config;

  // === Private Handlers ===

  /**
   * @brief Initialize the WiFi server
   */
  void _initializeWiFiServer();
};

// === Main Class ===

/**
 * @class HTWIFIV3
 * @brief A class to manage WiFi communication
 *
 * This class provide methods to handle WiFi communication.
 *
 * @note Capabilities:
 * - Connect to a WiFi Network
 * - Send and Receive Data to/from a Server
 */
class HTWIFIV3
{
public:
  HTWIFIV3();
  ~HTWIFIV3();

  /**
   * @brief HTWIFIV3Client class to work with the WiFi Chip as a client
   */
  HTWIFIV3Client *client;

  /**
   * @brief HTWIFIV3Server class to work with the WiFi Chip as a server
   */
  HTWIFIV3Server *server;

  /**
   * @brief Initialize the WiFi Chip
   */
  void begin();

  /**
   * @brief Stop the WiFi Chip
   */
  void stop();

  // === Getters ===

  /**
   * @brief Get the current config object
   *
   * @return HTWIFIV3Config
   */
  HTWIFIV3Config getConfig() const;

  /**
   * @brief Get the default configuration object
   *
   * @return HTWIFIV3Config Default configuration with standard values
   */
  static HTWIFIV3Config getDefaultConfig();

  // === Setters ===

  /**
   * @brief Set the config object
   *
   * @param config Config object
   */
  void setConfig(const HTWIFIV3Config &config);

  /**
   * @brief Update configuration after the WiFi is initialized
   *
   * @param config New configuration
   *
   * @warning Do not use this to change the configuration before the WiFi is initialized, use `setConfig()` instead
   *
   * @note Attention points:
   * - This function will disconnect the client and server
   * - The client and server should be initialized again after the configuration is updated
   */
  void updateConfig(const HTWIFIV3Config &config);

  // === Handlers ===

  /**
   * @brief Process the server
   *
   * @warning This function should be called on `loop()`
   *
   * @note Equivalent to server->handleClient()
   */
  void process();

private:
  /**
   * @brief Config object
   */
  HTWIFIV3Config _config;

  // === Private Handlers ===

  /**
   * @brief Initialize the WiFi
   */
  void _initializeWiFi(bool force = false);
};

#endif // HTWIFIV3_H
