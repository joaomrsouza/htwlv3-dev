/**
 * @file htwlv3.cpp
 * @brief Library to use HelTec WiFi LoRa 32 V3 Board
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

#include "htwlv3.h"

// Declare the display and lora classes
HTWLV3::HTWLV3()
{
  _config = getDefaultConfig();
  display = new Adafruit_SSD1306(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire, RST_OLED);
  lora = new HTLORAV3();
  wifi = new HTWIFIV3();
}

// Clear memory on deconstruction
HTWLV3::~HTWLV3()
{
  delete display;
  delete lora;
  delete wifi;
}

void HTWLV3::begin()
{
  _initializeBoard(true);
}

// === Getters ===

HTWLV3Config HTWLV3::getConfig() const
{
  return _config;
}

HTWLV3Config HTWLV3::getDefaultConfig()
{
  HTWLV3Config defaultConfig;

  defaultConfig.serialEnable = false;
  defaultConfig.serialSpeed = 115200;
  defaultConfig.displayEnable = false;
  defaultConfig.loraEnable = false;
  defaultConfig.wifiEnable = false;

  return defaultConfig;
}

// === Setters ===

void HTWLV3::setConfig(const HTWLV3Config &config)
{
  _config = config;
}

void HTWLV3::updateConfig(const HTWLV3Config &config)
{
  setConfig(config);

  _initializeBoard();
}

// === Handlers ===

void HTWLV3::process()
{
  if (lora)
    lora->process();

  if (wifi)
    wifi->process();
}

// === Print Template implementations ===

template <typename T>
void HTWLV3::print(T value)
{
  if (Serial.availableForWrite())
    Serial.print(value);

  if (display)
  {
    _checkDisplayScroll();
    display->print(value);
    display->display();
  }
}

template <typename T>
void HTWLV3::println(T value)
{
  if (Serial.availableForWrite())
    Serial.println(value);

  if (display)
  {
    _checkDisplayScroll();
    display->println(value);
    display->display();
  }
}

void HTWLV3::println()
{
  println("");
}

void HTWLV3::print(const StringSumHelper &str)
{
  print(String(str));
}

void HTWLV3::println(const StringSumHelper &str)
{
  println(String(str));
}

void HTWLV3::print(char *str)
{
  print(String(str));
}

void HTWLV3::println(char *str)
{
  println(String(str));
}

// === Private Handlers ===

void HTWLV3::_initializeBoard(bool force)
{
  // === Serial ===

  Serial.end();
  if (_config.serialEnable)
  {
    Serial.begin(_config.serialSpeed);
    Serial.flush();
    delay(50);
    Serial.println("Serial: initialized.");
  }

  // === Display ===

  if (_config.displayEnable)
  {
    if (!force && display != nullptr)
      return;

    if (display == nullptr)
      display = new Adafruit_SSD1306(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire, RST_OLED);

    // Display Reset via software
    pinMode(RST_OLED, OUTPUT);
    digitalWrite(RST_OLED, LOW);
    delay(20);
    digitalWrite(RST_OLED, HIGH);

    // Display Initialize
    Wire.begin(SDA_OLED, SCL_OLED);
    if (!display->begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false))
    {
      Serial.println(F("Display: SSD1306 allocation failed"));
      while (1)
        ; // Don't proceed, loop forever
    }

    // Display Feedback
    display->clearDisplay();
    display->setTextColor(WHITE);
    display->setTextSize(1);
    display->setCursor(0, 0);

    this->println("Display: initialized.");
  }
  else if (display != nullptr)
  {
    if (!force)
    {
      display->clearDisplay();
      display->display();
      Wire.end();
    }

    delete display;
    display = nullptr;
  }

  // === LoRa ===

  if (_config.loraEnable)
  {
    if (!force && lora != nullptr)
      return;

    if (lora == nullptr)
      lora = new HTLORAV3();

    lora->begin();

    this->println("LoRa: initialized.");
    this->print("Freq: ");
    this->println(lora->getConfig().frequency);
  }
  else if (lora != nullptr)
  {
    delete lora;
    lora = nullptr;
  }

  // === WiFi ===

  if (_config.wifiEnable)
  {
    if (!force && wifi != nullptr)
      return;

    if (wifi == nullptr)
      wifi = new HTWIFIV3();

    wifi->begin();

    this->println("WiFi: initialized.");

    HTWIFIV3Config config = wifi->getConfig();

    if (config.clientEnable)
    {
      this->println("Client: initialized.");
    }

    if (config.serverEnable)
    {
      this->println("Server: initialized.");
      IPAddress ip = wifi->server->getIP();
      this->print("IP: ");
      this->println(ip.toString());
    }
  }
  else if (wifi != nullptr)
  {
    delete wifi;
    wifi = nullptr;
  };
}

void HTWLV3::_checkDisplayScroll()
{
  if (display && display->getCursorY() >= 64)
  {
    display->clearDisplay();
    display->setCursor(0, 0);
  }
}

HTWLV3 Board;