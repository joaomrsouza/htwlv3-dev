// Copyright (c) Heltec Automation. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "heltecv3.h"

Heltec_ESP32::Heltec_ESP32()
{

#if defined(Class_Wifi_Kit) || defined(Class_WIFI_LORA)
  display = new SSD1306Wire(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED); // addr , freq , i2c group , resolution , rst
#elif defined(WIRELESS_STICK)
  display = new SSD1306Wire(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_64_32, RST_OLED);
#endif
}

Heltec_ESP32::~Heltec_ESP32()
{
#ifdef Heltec_Screen
  delete display;
#endif
}

void Heltec_ESP32::begin(bool DisplayEnable, bool LoRaEnable, bool SerialEnable, bool PABOOST, long BAND)
{

#ifdef Heltec_Vext
  VextON();
  delay(100);
#endif

  // UART
  if (SerialEnable)
  {
    Serial.begin(115200);
    Serial.flush();
    delay(50);
    Serial.print("Serial initial done\r\n");
  }
  Serial.print("SERIAL ENABLE DONE\r\n");

  // OLED
  if (DisplayEnable)
  {
    Serial.print("SCREEN ENABLE\r\n");
#ifndef Heltec_Screen
    if (SerialEnable)
    {
      Serial.print("Board does not have an on board display, Display option must be FALSE!!!\r\n");
    }
#endif

#ifdef Heltec_Screen
    Serial.print("SCREEN INIT\r\n");
    display->init();
    // display->flipScreenVertically();
    Serial.print("SCREEN SET FONT\r\n");
    display->setFont(ArialMT_Plain_10);
    Serial.print("SCREEN DRAW STRING\r\n");
    display->drawString(0, 0, "OLED initial done!");
    Serial.print("SCREEN DISPLAY\r\n");
    display->display();

    if (SerialEnable)
    {
      Serial.print("you can see OLED printed OLED initial done!\r\n");
    }
#endif
  }

  // LoRa INIT
  if (LoRaEnable)
  {
#ifndef Heltec_LoRa
    if (SerialEnable)
    {
      Serial.print("Board does not have LoRa function, LoRa option must be FALSE!!!\r\n");
    }
#endif

#ifdef Heltec_LoRa
    // LoRaClass LoRa;

    SPI.begin(SCK, MISO, MOSI, SS);
    LoRa.setPins(SS, RST_LoRa, DIO0);
    if (!LoRa.begin(BAND, PABOOST))
    {
      if (SerialEnable)
      {
        Serial.print("Starting LoRa failed!\r\n");
      }
#ifdef Heltec_Screen
      if (DisplayEnable)
      {
        display->clear();
        display->drawString(0, 0, "Starting LoRa failed!");
        display->display();
        delay(300);
      }
#endif
      while (1)
        ;
    }
    if (SerialEnable)
    {
      Serial.print("LoRa Initial success!\r\n");
    }
#ifdef Heltec_Screen
    if (DisplayEnable)
    {
      display->clear();
      display->drawString(0, 0, "LoRa Initial success!");
      display->display();
      delay(300);
    }
#endif

#endif
  }
#ifdef LED
  pinMode(LED, OUTPUT);
#endif
}

#ifdef Heltec_Vext
void Heltec_ESP32::VextON(void)
{
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW);
}

void Heltec_ESP32::VextOFF(void) // Vext default OFF
{
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, HIGH);
}
#endif

Heltec_ESP32 Heltec;
