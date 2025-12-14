/**
 * @file usage.cpp
 * @brief Example to use the SCLOG library for structured logging
 *
 * Description:
 *
 * This example demonstrates how to use the SCLOG library for structured logging
 * with node-based filtering and colored output. It shows how to configure log levels
 * for different nodes and how to use the various logging methods.
 *
 * Depends On:
 * - sclog
 * - Arduino.h
 *
 * @author @joaomrsouza (João Marcos Rocha Souza)
 * https://github.com/joaomrsouza
 */

#include "sclog.h"

// Define the current node ID
#ifndef NODE_ID
#define NODE_ID 1
#endif

// Configure log levels for different nodes
// Node 1: Only INFO and ERROR levels
SCLOG::LOG_LEVELS node1Levels[] = {SCLOG::INFO, SCLOG::ERROR};

// Node 2: INFO, WARN, and ERROR levels
SCLOG::LOG_LEVELS node2Levels[] = {SCLOG::INFO, SCLOG::WARN, SCLOG::ERROR};

// Node 3: All log levels enabled
SCLOG::LOG_LEVELS node3Levels[] = {SCLOG::DEBUG, SCLOG::INFO, SCLOG::TRACE, SCLOG::WARN, SCLOG::ERROR};

// Create node configurations array
SCLOG::NodeLogConfig configs[] = {
    {1, node1Levels, 2}, // Node 1: 2 levels (INFO, ERROR)
    {2, node2Levels, 3}, // Node 2: 3 levels (INFO, WARN, ERROR)
    {3, node3Levels, 5}  // Node 3: 5 levels (all)
};

// Create SCLOG instance with current node ID and configurations
SCLOG sc(NODE_ID, configs, 3);

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println("SCLOG Usage Example");
  Serial.println("===================");
  Serial.print("Current Node ID: ");
  Serial.println(NODE_ID);
  Serial.println();

  // Test all log levels
  // Only messages for enabled levels of the current node will be displayed
  sc.log("Testing DEBUG level", SCLOG::DEBUG);
  sc.log("Testing INFO level", SCLOG::INFO);
  sc.log("Testing TRACE level", SCLOG::TRACE);
  sc.log("Testing WARN level", SCLOG::WARN);
  sc.log("Testing ERROR level", SCLOG::ERROR);

  Serial.println();
  Serial.println("--- Logging with colors ---");
  Serial.println();

  // Log with specific colors (always displayed, regardless of node configuration)
  sc.log("This is a RED message", SCLOG::RED);
  sc.log("This is a GREEN message", SCLOG::GREEN);
  sc.log("This is a YELLOW message", SCLOG::YELLOW);
  sc.log("This is a BLUE message", SCLOG::BLUE);
  sc.log("This is a MAGENTA message", SCLOG::MAGENTA);
  sc.log("This is a CYAN message", SCLOG::CYAN);

  Serial.println();
  Serial.println("--- Logging with level and custom color ---");
  Serial.println();

  // Log with level and custom color (only displayed if level is enabled for current node)
  sc.log("INFO message with GREEN color", SCLOG::INFO, SCLOG::GREEN);
  sc.log("WARN message with YELLOW color", SCLOG::WARN, SCLOG::YELLOW);
  sc.log("ERROR message with RED color", SCLOG::ERROR, SCLOG::RED);

  Serial.println();
  Serial.println("--- Example: Simulating application events ---");
  Serial.println();

  // Simulate some application events
  sc.log("Application started", SCLOG::INFO);
  sc.log("Connecting to network...", SCLOG::TRACE);
  sc.log("Network connected successfully", SCLOG::INFO, SCLOG::GREEN);
  sc.log("Low battery warning", SCLOG::WARN, SCLOG::YELLOW);
  sc.log("Sensor reading: 25.5°C", SCLOG::DEBUG);
  sc.log("Failed to send packet", SCLOG::ERROR, SCLOG::RED);
  sc.log("Operation completed", SCLOG::INFO);

  Serial.println();
  Serial.println("Setup complete!");
  Serial.println();
  Serial.println("Note: Only log levels enabled for the current node will be displayed.");
  Serial.println("Change NODE_ID to see different filtering behavior.");
}

void loop()
{
  // Example: Periodic logging
  static unsigned long lastLog = 0;
  unsigned long now = millis();

  if (now - lastLog >= 5000) // Every 5 seconds
  {
    lastLog = now;
    sc.log("Heartbeat: System running", SCLOG::TRACE);
  }

  delay(100);
}
