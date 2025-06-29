/**
 * @file htlorav3.h
 * @brief Library to use LoRa with the HelTec WiFi LoRa 32 V3 Board
 *
 * Description:
 *
 * This library abstracts commands for the board peripheral LoRa Chip.
 *
 * Configuration:
 *
 * It's possible to reconfigure some parameters by redefining them in the main file, check "=== Default Config ===" section bellow.
 *
 * Depends On:
 * - heltecautomation/Heltec ESP32 Dev-Boards@2.0.2
 *
 * @author @joaomrsouza (João Marcos Rocha Souza)
 * https://github.com/joaomrsouza
 *
 * */

#ifndef HTLORAV3_H
#define HTLORAV3_H

// LoRa Libs
#include <SPI.h>
#include "LoRaWan_APP.h"

// === Structs ===

/**
 * @brief Config object for the LoRa Chip
 */
typedef struct
{
  // Channel RF frequency
  double frequency;
  // Bandwidth - [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved]
  int bandwidth;
  // Spreading Factor - [SF7..SF12]
  int spreadingFactor;
  // Coding Rate - [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
  int codingRate;
  // Preamble Length - Symbols - Same for Tx and Rx
  int preambleLength;
  // Fix Length Payload On
  bool fixLengthPayloadOn;
  // IQ Inversion On
  bool iqInversionOn;
  // Output Power - dBm
  int txOutPower;
  // TX Timeout - ms
  int txTimeout;
  // RX Timeout - Symbols
  int rxTimeout;
} HTLORAV3Config;

typedef struct
{
  char *data;
  int16_t rssi;
  uint16_t size;
  int8_t snr;
} LoraDataPacket;

/**
 * @class HTLORAV3
 * @brief A class to manage LoRa communication
 *
 * This class provide methods to handle LoRa communication.
 *
 * @note Capabilities:
 * - Send Packets
 * - Listen to Packets
 */
class HTLORAV3
{
public:
  HTLORAV3();
  ~HTLORAV3();

  /**
   * @brief Setup the configured constants and binding methods
   *
   * @param FREQ Frequency of work for the LoRa Chip
   */
  void begin();

  // === Getters ===

  /**
   * @brief Get the current config object
   *
   * @return HTLORAV3Config*
   */
  HTLORAV3Config *getConfig();

  /**
   * @brief Get the Idle state of LoRa Chip
   *
   * @return bool
   */
  bool getIdle();

  // === Setters ===

  /**
   * @brief Set the config object
   *
   * @param config Config object
   */
  void setConfig(HTLORAV3Config *config);

  /**
   * @brief Set the onReceive function called when a packet is received
   *
   * @warning This function should be called in `setup()` if you want to process received packets
   *
   * @note Use this to process the received packets
   * @note Use `listenToPacket()` to start listening to packets
   *
   * @param onReceive Function that should be called when a packet arrives
   */
  void setOnReceive(void (*onReceive)(LoraDataPacket packet));

  /**
   * @brief Set the onReceiveTimeout function called when `listenToPacket()` timeout
   *
   * @param onReceiveTimeout Function that should be called when `listenToPacket()` timeout
   */
  void setOnReceiveTimeout(void (*onReceiveTimeout)());

  /**
   * @brief Set the onSendDone function called after a packet is sent
   *
   * @param onSendDone Function that should be called after a packet is sent
   */
  void setOnSendDone(void (*onSendDone)());

  /**
   * @brief Set the onSendTimeout function called when a packet sent timeout
   *
   * @param onSendTimeout Function that should be called when a packet sent timeout
   */
  void setOnSendTimeout(void (*onSendTimeout)());

  // === Handlers ===

  /**
   * @brief Process the radio interruption requests
   *
   * @warning This function should be called on `loop()`
   *
   * @note Equivalent to Radio.IrqProcess
   */
  void process();

  /**
   * @brief Send data packets
   *
   * @param data Data string to be sent
   * @return int [0: ok, 1: busy]
   */
  int sendPacket(const char *data);

  /**
   * @brief Start listening for packets
   *
   * @note Use this to listen for incoming packets
   * @note Call `setOnReceive()` to process the received packets
   *
   * @param timeout Timeout for the listening in ms [0: continuous, any number]
   */
  void listenToPacket(uint32_t timeout = 0);

private:
  /**
   * @brief Config object
   */
  HTLORAV3Config *_config;

  /**
   * @brief RadioEvents struct for setup Radio Lib
   */
  static RadioEvents_t _RadioEvents;

  /**
   * @brief LoRa Chip state
   */
  static bool _idle;

  // === Private Handlers ===

  /**
   * @brief Initialize the config object
   */
  void _initConfig();

  /**
   * @brief Function to be called when a packet is received
   *
   * @note Call `setOnReceive()` to set this function
   */
  static void (*_onReceive)(LoraDataPacket packet);

  /**
   * @brief Function to be called when `listenToPacket()` timeout
   *
   * @note Call `setOnReceiveTimeout()` to set this function
   */
  static void (*_onReceiveTimeout)();

  /**
   * @brief Function to be called after a packet is sent
   *
   * @note Call `setOnSendDone()` to set this function
   */
  static void (*_onSendDone)();

  /**
   * @brief Function to be called when a packet sent timeout
   *
   * @note Call `setOnSendTimeout()` to set this function
   */
  static void (*_onSendTimeout)();

  // === Static Handlers ===

  /**
   * @brief Internal function to be called when a packet is sent
   */
  static void _onTxDone();

  /**
   * @brief Internal function to be called when packet sent timeout
   */
  static void _onTxTimeout();

  /**
   * @brief Internal function to be called when a packet is received
   *
   * This function process the packet info and call `_onReceive` if available
   *
   * @param payload Data received payload
   * @param size Payload size
   * @param rssi Received Signal Strength Indicator
   * @param snr Signal-to-Noise Ratio
   */
  static void _onRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);

  /**
   * @brief Internal function to be called when `listenToPacket()` timeout
   */
  static void _onRxTimeout();
};

/**
 * @brief Use this variable to manipulate this library
 */
extern HTLORAV3 LoRa;

#endif // HTLORAV3_H
