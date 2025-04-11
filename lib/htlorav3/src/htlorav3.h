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

// TODO: verificar o funcionamento disso
#ifdef __has_include(<sstream>)
#define USE_SSTREAM
#endif

#ifdef USE_SSTREAM
#include <sstream>
#endif

// LoRa Libs
#include <SPI.h>
#include "LoRaWan_APP.h"

// === Default Config ===

#ifndef HTLORAV3_FREQUENCY
#define HTLORAV3_FREQUENCY 470E6
#endif

#ifndef HTLORAV3_BANDWIDTH
#define HTLORAV3_BANDWIDTH 0 // [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved]
#endif

#ifndef HTLORAV3_SPREADING_FACTOR
#define HTLORAV3_SPREADING_FACTOR 7 // [SF7..SF12]
#endif

#ifndef HTLORAV3_CODINGRATE
#define HTLORAV3_CODINGRATE 1 // [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
#endif

#ifndef HTLORAV3_PREAMBLE_LENGTH
#define HTLORAV3_PREAMBLE_LENGTH 8 // Same for Tx and Rx
#endif

#ifndef HTLORAV3_FIX_LENGTH_PAYLOAD_ON
#define HTLORAV3_FIX_LENGTH_PAYLOAD_ON false
#endif

#ifndef HTLORAV3_IQ_INVERSION_ON
#define HTLORAV3_IQ_INVERSION_ON false
#endif

#ifndef HTLORAV3_TX_OUT_POWER
#define HTLORAV3_TX_OUT_POWER 5 // dBm
#endif

#ifndef HTLORAV3_RX_TIMEOUT
#define HTLORAV3_RX_TIMEOUT 0 // Symbols
#endif

// === End Default Config ===

// === Structs ===

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

  /**
   * @brief Setup the configured constants and binding methods
   *
   * @param FREQ Frequency of work for the LoRa Chip
   */
  void begin();

  /**
   * @brief Get the Idle state of LoRa Chip
   *
   * @return bool
   */
  bool getIdle();

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

#ifdef USE_SSTREAM
  int sendPacket(std::stringstream data);
#endif

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
   * @brief RadioEvents struct for setup Radio Lib
   */
  static RadioEvents_t _RadioEvents;

  /**
   * @brief LoRa Chip state
   */
  static bool _idle;

  /**
   * @brief Function to be called when a packet is received
   *
   * @note Call `setOnReceive()` to set this function
   */
  static void (*_onReceive)(LoraDataPacket packet);

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
};

/**
 * @brief Use this variable to manipulate this library
 */
extern HTLORAV3 LoRa;