// ! LoRa.h da heltec Não funciona nessa placa

/*
  This is a simple example show the Heltec.LoRa sended data in OLED.

  The onboard OLED display is SSD1306 driver and I2C interface. In order to make the
  OLED correctly operation, you should output a high-low-high(1-0-1) signal by soft-
  ware to OLED's reset pin, the low-level signal at least 5ms.

  OLED pins to ESP32 GPIOs via this connecthin:
  OLED_SDA -- GPIO4
  OLED_SCL -- GPIO15
  OLED_RST -- GPIO16

  by Aaron.Lee from HelTec AutoMation, ChengDu, China
  成都惠利特自动化科技有限公司
  https://heltec.org

  this project also realess in GitHub:
  https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series
*/

// #define WIFI_LORA_32_V3 true
// #define Heltec_Wifi
// #define Heltec_LoRa
// #define Heltec_Screen
// #include "Arduino.h"

#include <heltecv3.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// #include "images.h"

// SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED); // addr , freq , i2c group , resolution , rst
Adafruit_SSD1306 display(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire, RST_OLED); // !ESSE

#define BAND 433.2e6 // you can set band here directly,e.g. 868E6,915E6

unsigned int counter = 0;
String rssi = "RSSI --";
String packSize = "--";
String packet;

void logo()
{
  // Heltec.display->clear();
  // Heltec.display->drawString(0, 0, "LOGO");
  // Heltec.display->drawXbm(0, 5, logo_width, logo_height, logo_bits);
  // Heltec.display->display();
}

void VextON(void)
{
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW);
}

void setup()
{
  // WIFI Kit series V1 not support Vext control
  // Heltec.begin(true /*DisplayEnable*/, false /*Heltec.Heltec.Heltec.LoRa*/, true /*Serial*/, true /*PABOOST*/, BAND /*long BAND*/);
  Serial.begin(115200);
  VextON();
  delay(100);
  pinMode(RST_OLED, OUTPUT);
  digitalWrite(RST_OLED, LOW);
  delay(20);
  digitalWrite(RST_OLED, HIGH);

  Wire.begin(SDA_OLED, SCL_OLED);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false))
  { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("LORA SENDER");
  display.display();

  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST_LoRa, DIO0);
  if (!LoRa.begin(BAND, true))
  {
    Serial.print("Starting LoRa failed!\r\n");
    display.clearDisplay();
    display.print("Starting LoRa failed!");
    display.display();
    delay(300);
    while (1)
      ;
  }
  Serial.print("LoRa Initial success!\r\n");

  display.init();
  display.clear();
  display.drawString(0, 0, "HELLO");
  display.display();
  Serial.println("SETUP");

  // Heltec.display->init();
  // Heltec.display->flipScreenVertically();
  // Heltec.display->setFont(ArialMT_Plain_10);
  // logo();
  // delay(1500);
  // Heltec.display->clear();

  // Heltec.display->drawString(0, 0, "Heltec.LoRa Initial success!");
  // Heltec.display->display();
  delay(1000);
}

void loop()
{

  Serial.println("LOOP");
  // Heltec.display->clear();
  // Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  // Heltec.display->setFont(ArialMT_Plain_10);

  // Heltec.display->drawString(0, 0, "Sending packet: ");
  // Heltec.display->drawString(90, 0, String(counter));
  // Heltec.display->display();

  // send packet
  // LoRa.beginPacket();

  /*
   * LoRa.setTxPower(txPower,RFOUT_pin);
   * txPower -- 0 ~ 20
   * RFOUT_pin could be RF_PACONFIG_PASELECT_PABOOST or RF_PACONFIG_PASELECT_RFO
   *   - RF_PACONFIG_PASELECT_PABOOST -- LoRa single output via PABOOST, maximum output 20dBm
   *   - RF_PACONFIG_PASELECT_RFO     -- LoRa single output via RFO_HF / RFO_LF, maximum output 14dBm
   */
  // LoRa.setTxPower(14, RF_PACONFIG_PASELECT_PABOOST);
  // LoRa.print("hello ");
  // LoRa.print(counter);
  // LoRa.endPacket();

  // counter++;
  // digitalWrite(LED, HIGH); // turn the LED on (HIGH is the voltage level)
  // delay(1000);             // wait for a second
  // digitalWrite(LED, LOW);  // turn the LED off by making the voltage LOW
  delay(1000); // wait for a second
}