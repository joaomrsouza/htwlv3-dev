/**
 * @file htwlv3.h
 * @brief Library to use the HelTec WiFi LoRa 32 V3 Board embedded peripherals
 *
 * Description:
 *
 * This library abstracts commands for the board peripherals LoRa Chip and OLED Display.
 *
 * Configuration:
 *
 * It's possible to reconfigure some parameters for the LoRa Chip by redefining them in the main file, check "=== Default Config ===" section in the `htlorav3.h` file.
 *
 * Depends On:
 * - htlorav3
 * - heltecautomation/Heltec ESP32 Dev-Boards@2.0.2
 *
 * @author @joaomrsouza (João Marcos Rocha Souza)
 * https://github.com/joaomrsouza
 */

#include <Arduino.h>

// Display Libs
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// LoRa Lib
#include "htlorav3.h"

// WiFi Libs
#include <WiFi.h>
#include <WebServer.h>

// === Default Config ===

#ifndef HTWLV3_SERVER_SSID
#define HTWLV3_SERVER_SSID "HTWLV3 Server"
#endif

#ifndef HTWLV3_SERVER_PASSWORD
#define HTWLV3_SERVER_PASSWORD "12345678"
#endif

/**
 * @brief A class to use HelTec Board peripherals (HelTec WiFi LoRa 32 V3)
 *
 * This class provides an abstraction to use the HelTec WiFi LoRa 32 V3 Board peripherals such as the OLED Display and the LoRa Chip
 */
class HTWLV3
{
public:
  HTWLV3();
  ~HTWLV3();

  /**
   * @brief Adafruit class to work with the OLED display
   */
  Adafruit_SSD1306 *display;

  /**
   * @brief HTLORAV3 class to work with the LoRa Chip
   */
  HTLORAV3 *lora;

  /**
   *@brief WebServer class to work with the WiFi Chip as a server
   */
  WebServer *server;

  /**
   * @brief Initialize the Board peripherals
   *
   * @param serialEnable Enable Serial (speed 115200)
   * @param displayEnable Enable OLED Display
   * @param serverEnable Enable WiFi Chip as a server
   * @param loraEnable Enable LoRa Chip
   * @param loraFreq Frequency of work for the LoRa Chip
   */
  void begin(bool serialEnable = false, bool displayEnable = false, bool serverEnable = false, bool loraEnable = false);

  /**
   * @brief Process all the enabled devices
   *
   * @warning This function should be called on `loop()`
   */
  void process();

  /**
   * @brief Print on all enabled outputs (display, Serial)
   */
  void print(const char *str);

  /**
   * @brief Println on all enabled outputs (display, Serial)
   */
  void println(const char *str);

private:
  void _displayBreakWriteLine(const char *str);
  void _displayWriteLine(const char *str);
};

extern HTWLV3 Board;