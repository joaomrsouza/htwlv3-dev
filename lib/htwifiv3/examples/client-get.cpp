/**
 * @file client-get.cpp
 * @brief Example to use the WiFi Client to get data from a server
 *
 * Depends On:
 * - htwifiv3
 * - heltecautomation/Heltec ESP32 Dev-Boards@2.0.2
 *
 * @author @joaomrsouza (João Marcos Rocha Souza)
 * https://github.com/joaomrsouza
 *
 */

// Include the HelTec WiFi V3 library
#include "htwifiv3.h"

// Declare the WiFi instance
HTWIFIV3 wifi = HTWIFIV3();

// Declare the config function
void config();

void setup()
{
  Serial.begin(115200);

  Serial.println("Setting up...");
  // Apply the custom config before starting the WiFi Client
  config();
  wifi.begin();

  Serial.println("Waiting for connection...");
  while (!wifi.client->getIsConnected())
  {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\nSetup complete.");
}

void loop()
{
  // Get the data from the server
  Serial.println("Getting data...");

  // It automatically uses the secure connection if the URL starts with https://
  // In this case, make sure to set the CA certificate before getting the data
  // wifi.client->setCACert(caCert);
  JsonDocument response = wifi.client->get("http://127.0.0.1:8080/");

  // Check if there was an error
  if (response["error"])
  {
    // Print the error message
    Serial.print("ERROR: ");
    Serial.println(response["error_message"].as<String>());
  }
  else
  {
    // Print the data received
    Serial.println("Data received:");
    String data;
    serializeJson(response["data"], data);
    Serial.println(data.c_str());
  }

  delay(10000);
}

// Custom config function
void config()
{
  HTWIFIV3Config config;
  config.clientEnable = true;
  config.serverEnable = false;

  wifi.setConfig(&config);

  // If the client is enabled, set the custom config
  if (config.clientEnable)
  {
    HTWIFIV3ClientConfig clientConfig;

    clientConfig.ssid = "YOUR_SSID";
    clientConfig.password = "YOUR_PASSWORD";

    wifi.client->setConfig(&clientConfig);
  }
}
