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

  for (int i = 0; i < 8; i++)
    _lineBuffer[i] = nullptr;
  _currentLineBufferIndex = 0;
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
    this->_displayBreakWriteLine(str);
}

void HTWLV3::println(const char *str)
{
  if (Serial.availableForWrite())
    Serial.println(str);

  if (display)
    this->_displayBreakWriteLine(str);
}

void HTWLV3::_displayBreakWriteLine(const char *str)
{
  const char *start = str;
  const char *end;

  while ((end = strchr(start, '\n')) != nullptr)
  {
    // Calculate the length of the current line
    size_t length = end - start;

    // Allocate memory for the line and copy it
    char *line = new char[length + 1]; // +1 for the null terminator
    strncpy(line, start, length);
    line[length] = '\0'; // Null-terminate the string

    // TODO: Cortar em 21 chars

    this->_displayWriteLine(line);
    free(line);

    start = end + 1; // Move past the newline character
  }

  // Handle the last segment if there's any remaining text
  if (*start != '\0')
  {
    size_t length = strlen(start);
    char *line = new char[length + 1];
    strcpy(line, start);
    this->_displayWriteLine(line);
    free(line);
  }
}

void HTWLV3::_displayWriteLine(const char *str)
{
  if (_currentLineBufferIndex == 8)
  {
    display->clearDisplay();
    display->setCursor(0, 0);
    free(_lineBuffer[0]);

    for (int i = 0; i < 7; i++)
    {
      _lineBuffer[i] = _lineBuffer[i + 1];
      display->println(_lineBuffer[i]);
    }

    display->display();
    _currentLineBufferIndex = 7;
  }

  _lineBuffer[_currentLineBufferIndex] = (char *)malloc(strlen(str) + 1);

  if (_lineBuffer[_currentLineBufferIndex] == nullptr)
    return;

  strcpy(_lineBuffer[_currentLineBufferIndex], str);

  display->println(_lineBuffer[_currentLineBufferIndex]);
  display->display();

  _currentLineBufferIndex++;
}

HTWLV3 Board;