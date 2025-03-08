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

#include <vector>

// Declare the display and lora classes
HTWLV3::HTWLV3()
{
  display = new Adafruit_SSD1306(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire, RST_OLED);
  lora = new HTLORAV3();
  server = new WebServer();
}

// Clear memory on deconstruction
HTWLV3::~HTWLV3()
{
  delete display;
  delete lora;
  delete server;
}

void HTWLV3::begin(bool serialEnable, bool displayEnable, bool serverEnable, bool loraEnable)
{
  // === Serial ===
  if (serialEnable)
  {
    Serial.begin(115200);
    Serial.flush();
    delay(50);
    Serial.println("Serial: initialized.");
  }

  // === Display ===
  if (displayEnable)
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
  if (loraEnable)
  {
    lora->begin();

    this->println("LoRa: initialized.");
    this->print("Freq: ");
    this->println(String(HTLORAV3_FREQUENCY).c_str());
  }
  else
  {
    delete lora;
    lora = nullptr;
  }

  // === Server ===
  if (serverEnable)
  {
    WiFi.softAP(HTWLV3_SERVER_SSID, HTWLV3_SERVER_PASSWORD);
    IPAddress IP = WiFi.softAPIP();
    server->begin();

    this->println("WiFi Server: initialized.");
    this->print("IP: ");
    this->println(IP.toString().c_str());
  }
  else
  {
    delete server;
    server = nullptr;
  };
}

void HTWLV3::process()
{
  if (lora)
    lora->process();

  if (server)
    server->handleClient();
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

HTWLV3 Board;