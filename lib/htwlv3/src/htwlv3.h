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
 * It's possible to configure some parameters by using the `setConfig()` and `updateConfig()` methods. For configuration of the LoRa Chip, check `htlorav3.h` file and for the WiFi Chip, check `htwifiv3.h` file.
 *
 * Depends On:
 * - htlorav3
 * - htwifiv3
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
 * This class provides an abstraction to use the HelTec WiFi LoRa 32 V3 Board peripherals such as the OLED Display, the LoRa and WiFi Chips
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
   * @return HTWLV3Config
   */
  HTWLV3Config getConfig() const;

  /**
   * @brief Get the default configuration object
   *
   * @return HTWLV3Config Default configuration with standard values
   */
  static HTWLV3Config getDefaultConfig();

  // === Setters ===

  /**
   * @brief Set the config object
   *
   * @param config Config object
   *
   * @warning Do not use this to change the configuration after the Board peripherals are initialized, use `updateConfig()` instead
   */
  void setConfig(const HTWLV3Config &config);

  /**
   * @brief Update configuration after the Board peripherals are initialized
   *
   * @warning Do not use this to change the configuration before the Board peripherals are initialized, use `setConfig()` instead
   *
   * @note Attention points:
   * - This function will reinitialize the Board peripherals
   * - Untouched peripherals will not be reinitialized
   *
   * @param config Config object
   */
  void updateConfig(const HTWLV3Config &config);

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
  template <typename T>
  void print(T value);

  void print(char *str);
  void print(const StringSumHelper &str);

  /**
   * @brief Println on all enabled outputs (display, Serial)
   */
  template <typename T>
  void println(T value);

  void println(char *str);
  void println(const StringSumHelper &str);
  void println(); // Blank line

private:
  /**
   * @brief Config object
   */
  HTWLV3Config _config;

  // === Private Handlers ===

  /**
   * @brief Initialize the Board peripherals
   */
  void _initializeBoard(bool force = false);

  /**
   * @brief Helper function to manage display scrolling
   */
  void _checkDisplayScroll();
};

extern HTWLV3 Board;

#endif // HTWLV3_H
