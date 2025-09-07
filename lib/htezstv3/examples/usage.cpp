/**
 * @file usage.cpp
 * @brief Example to use EZSettings with HelTec WiFi LoRa 32 V3 Board
 *
 * Description:
 *
 * This example demonstrates how to use the EZSettings namespace to create
 * a web-based settings interface for the HelTec WiFi LoRa 32 V3 Board.
 * It automatically sets up a web server and provides a user-friendly
 * interface to configure all board peripherals.
 *
 * Depends On:
 * - htezstv3
 * - htwlv3
 *
 * @author @joaomrsouza (João Marcos Rocha Souza)
 * https://github.com/joaomrsouza
 */

// Include the HelTec WiFi LoRa 32 V3 Board library
#include "htwlv3.h"
// Include the EZSettings library
#include "htezstv3.h"

void setup()
{
  // Initialize the board
  Board.begin();

  Board.println("EZSettings Example");
  Board.println("Setting up...");

  // Initialize the EZSettings web interface
  EZSettings::begin();

  Board.println("Setup complete");
}

void loop()
{
  Board.process();
}
