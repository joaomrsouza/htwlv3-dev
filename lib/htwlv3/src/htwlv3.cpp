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
 * It's possible to reconfigure some parameters for the LoRa Chip by redefining them in the main file, check "=== Default Config ===" section in the `htlorav3.h` file.
 *
 * Depends On:
 * - htlorav3
 * - heltecautomation/Heltec ESP32 Dev-Boards@2.0.2
 *
 * @author @joaomrsouza (João Marcos Rocha Souza)
 * https://github.com/joaomrsouza
 */

#include "htwlv3.h"

// Declare the display and lora classes
HTWLV3::HTWLV3()
{
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
  _initConfig();

  // === Serial ===
  if (_config->serialEnable)
  {
    Serial.begin(_config->serialSpeed);
    Serial.flush();
    delay(50);
    Serial.println("Serial: initialized.");
  }

  // === Display ===
  if (_config->displayEnable)
  {
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
  else
  {
    delete display;
    display = nullptr;
  }

  // === LoRa ===
  if (_config->loraEnable)
  {
    lora->begin();

    this->println("LoRa: initialized.");
    this->print("Freq: ");
    this->println(String(lora->getConfig()->frequency).c_str());
  }
  else
  {
    delete lora;
    lora = nullptr;
  }

  // === WiFi ===
  if (_config->wifiEnable)
  {
    wifi->begin();

    HTWIFIV3Config *config = wifi->getConfig();

    if (config->clientEnable)
    {
      this->println("Client: initialized.");
    }

    if (config->serverEnable)
    {
      this->println("Server: initialized.");
      IPAddress ip = wifi->server->getIP();
      this->print("IP: ");
      this->println(ip.toString().c_str());
    }
  }
  else
  {

    this->println("WiFi: DELETE");
    delete wifi;
    wifi = nullptr;
  };
}

// === Getters ===

HTWLV3Config *HTWLV3::getConfig()
{
  return _config;
}

// === Setters ===

void HTWLV3::setConfig(HTWLV3Config *config)
{
  _config = new HTWLV3Config();

  _config->serialEnable = config->serialEnable;
  _config->serialSpeed = config->serialSpeed;
  _config->displayEnable = config->displayEnable;
  _config->loraEnable = config->loraEnable;
  _config->wifiEnable = config->wifiEnable;
}

// === Handlers ===

void HTWLV3::process()
{
  if (lora)
    lora->process();

  if (wifi)
    wifi->process();
}

void HTWLV3::print(const char *str)
{
  if (Serial.availableForWrite())
    Serial.print(str);

  if (display)
  {
    if (display->getCursorY() >= 64)
    {
      display->clearDisplay();
      display->setCursor(0, 0);
    }
    display->print(str);
    display->display();
  }
}

void HTWLV3::println(const char *str)
{
  if (Serial.availableForWrite())
    Serial.println(str);

  if (display)
  {
    if (display->getCursorY() >= 64)
    {
      display->clearDisplay();
      display->setCursor(0, 0);
    }
    display->println(str);
    display->display();
  }
}

// === Private Handlers ===

void HTWLV3::_initConfig()
{

  if (_config != nullptr)
    return;

  _config = new HTWLV3Config();

  _config->serialEnable = false;
  _config->serialSpeed = 115200;
  _config->displayEnable = false;
  _config->loraEnable = false;
  _config->wifiEnable = false;
}

HTWLV3 Board;