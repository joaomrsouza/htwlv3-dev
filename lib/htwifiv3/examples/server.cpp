/**
 * @file server.cpp
 * @brief Example to use the WiFi Server to serve a simple HTML page and receive form data
 *
 * Depends On:
 * - htwifiv3
 *
 * @author @joaomrsouza (João Marcos Rocha Souza)
 * https://github.com/joaomrsouza
 */

#include "htwifiv3.h"

// Declare the WiFi instance
HTWIFIV3 wifi = HTWIFIV3();

// Declare the config function
void config();

// Declare the handlers
void handleRoot();
void handlePost();

void setup()
{
  Serial.begin(115200);

  Serial.println("Setting up...");
  // Apply the custom config before starting the WiFi Server
  config();
  wifi.begin();

  // Register the handlers
  wifi.server->on("/", HTTP_GET, handleRoot);
  wifi.server->on("/post", HTTP_POST, handlePost);

  // Print the server IP
  Serial.println("Server running on http://" + wifi.server->getIP().toString());
  Serial.println("Setup complete.");
}

void loop()
{
  // Process the WiFi Server events
  wifi.process();
}

// Base template string
String pageTemplate = R"(<!DOCTYPE html>
    <html>
      <head>
        <title>Hello World</title>
        <meta name="viewport" content="width=device-width, initial-scale=1" />
      </head>
      <body>
        {PAGE_CONTENT}
        <form action="/post" method="post">
          <input type="text" name="name" placeholder="Name" />
          <input type="submit" value="Send" />
        </form>
      </body>
    </html>
)";

// Handle the root path
void handleRoot()
{
  // Mount the page string
  String page = String(pageTemplate);

  page.replace("{PAGE_CONTENT}", "<h1>Hello World</h1>");

  // Send the page
  wifi.server->send(200, "text/html", page);
}

// Handle the post path
void handlePost()
{
  // Get the name from the form
  String name = wifi.server->arg("name");

  // Mount the page string
  String page = String(pageTemplate);

  page.replace("{PAGE_CONTENT}", "<h1>Hello, " + name + "!</h1>");

  // Send the page
  wifi.server->send(200, "text/html", page);
}

// Custom config function
void config()
{
  HTWIFIV3Config wifiConfig = HTWIFIV3::getDefaultConfig();

  wifiConfig.serverEnable = true;

  wifi.setConfig(wifiConfig);

  // Set the server config
  HTWIFIV3ServerConfig serverConfig = HTWIFIV3Server::getDefaultConfig();

  serverConfig.ssid = "ESP_SSID";
  serverConfig.password = "ESP_PASSWORD";

  wifi.server->setConfig(serverConfig);
}
