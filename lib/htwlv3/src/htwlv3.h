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

#ifndef HTWLV3_H
#define HTWLV3_H

#include <Arduino.h>

// Display Libs
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// LoRa Lib
#include "htlorav3.h"

// WiFi Lib
#include "htwifiv3.h"

// === Structs ===

typedef struct
{
  bool serialEnable;
  int serialSpeed;
  bool displayEnable;
  bool loraEnable;
  bool wifiEnable;
} HTWLV3Config;

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
   *@brief HTWIFIV3 class to work with the WiFi Chip
   */
  HTWIFIV3 *wifi;

  /**
   * @brief Initialize the Board peripherals
   */
  void begin();

  // === Getters ===

  /**
   * @brief Get the current config object
   *
   * @return HTWLV3Config*
   */
  HTWLV3Config *getConfig();

  // === Setters ===

  /**
   * @brief Set the config object
   *
   * @param config Config object
   */
  void setConfig(HTWLV3Config *config);

  // === Handlers ===

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
  /**
   * @brief Config object
   */
  HTWLV3Config *_config;

  // === Private Handlers ===

  /**
   * @brief Initialize the config object
   */
  void _initConfig();
};

extern HTWLV3 Board;

#endif // HTWLV3_H
