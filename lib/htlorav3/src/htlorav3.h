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
 * It's possible to configure some parameters by using the `setConfig()` and `updateConfig()` methods.
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
#include "radio/radio.h"
#include "ESP32_Mcu.h"

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
  // Output Power - dBm - [-3..22]
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

typedef struct
{
  unsigned int nodeAddress;
  unsigned int packetId;
} ReceivedPacketInfo;

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

  enum LoRaStates
  {
    IDLE,           // 0
    SENDING,        // 1
    SEND_TIMEOUT,   // 2
    RECEIVING,      // 3
    RECEIVE_TIMEOUT // 4
  };

  /**
   * @brief Setup the configured constants and binding methods
   *
   * @param address LoRa node address [1-999, Default to 0: for anonymous mode]
   */
  void begin(unsigned int address = 0);

  /**
   * @brief Stop the LoRa Chip
   */
  void stop();

  // === Getters ===

  /**
   * @brief Get the current config object
   *
   * @return HTLORAV3Config*
   */
  HTLORAV3Config getConfig() const;

  /**
   * @brief Get the state of LoRa Chip
   *
   * @return LoRaStates
   */
  LoRaStates getState();

  /**
   * @brief Get the default configuration object
   *
   * @return HTLORAV3Config Default configuration with standard values
   */
  static HTLORAV3Config getDefaultConfig();

  // === Setters ===

  /**
   * @brief Set the config object
   *
   * @param config Config object
   *
   * @warning Do not use this to change the configuration after the LoRa is initialized, use `updateConfig()` instead
   */
  void setConfig(const HTLORAV3Config &config);

  /**
   * @brief Update configuration after the LoRa is initialized
   *
   * @param config Config object
   *
   * @warning Do not use this to change the configuration before the LoRa is initialized, use `setConfig()` instead
   *
   * @note Attention points:
   * - This function will put the LoRa in sleep mode and idle state
   * - The LoRa should be initialized again after the configuration is updated
   */
  void updateConfig(const HTLORAV3Config &config);

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
   * @param destinationAddress Destination node address (0 for broadcast)
   * @return int [0: ok, 1: busy]
   */
  int sendPacket(const char *data, unsigned int destinationAddress = 0);

  /**
   * @brief Send data packets and wait for ACK
   *
   * @warning You should set an lora address at the `begin()` function to use this.
   *
   * @param data Data string to be sent
   * @param destinationAddress Destination node address (broadcast not allowed)
   * @return int [0: ok, 1: busy]
   */
  int sendReliablePacket(const char *data, unsigned int destinationAddress);

  /**
   * @brief Start listening for packets
   *
   * @note Use this to listen for incoming packets
   * @note Call `setOnReceive()` to process the received packets
   * @note Call `setOnReceiveTimeout()` to use listen timeout
   *
   * @param timeout Timeout for the listening in ms [0: continuous, any number]
   * @return int [0: ok, 1: busy]
   */
  int listenToPacket(uint32_t timeout = 0);

private:
  /**
   * @brief LoRa node address [1-999, Default to 0: for anonymous mode]
   */
  static unsigned int _address;

  /**
   * @brief Controls when the library is using callbacks internaly before call user callbacks
   */
  static bool _internalCallbacks;

  /**
   * @brief Address to send ACK to [1-999, Default to 0: for no ACK]
   */
  static unsigned int _sendACKTo;

  /**
   * @brief Current set receive timeout
   */
  static unsigned long _receiveTimeoutMillis;

  /**
   * @brief Millis timestamp for receive timeout
   */
  static unsigned long _receiveTimeoutTimestamp;

  /**
   * @brief Current packet id [1-99, Default to 0: for none sent yet]
   */
  static int _currentPacketId;

  /**
   * @brief Flag that controls when a packet should be ignored (eg: duplicated packet)
   */
  static bool _ignorePacket;

  /**
   * @brief Last packet received
   */
  static LoraDataPacket _lastPacket;

  /**
   * @brief Store the last 10 node address and packet id of the received packets (circular buffer)
   */
  static ReceivedPacketInfo _receivedPacketsList[10];

  /**
   * @brief Current index in the `_receivedPacketsList` circular buffer
   */
  static int _receivedPacketsIndex;

  /**
   * @brief Number of packets stored in the `_receivedPacketsList` list (0-10)
   */
  static int _receivedPacketsCount;

  /**
   * @brief Config object
   */
  HTLORAV3Config _config;

  /**
   * @brief RadioEvents struct for setup Radio Lib
   */
  static RadioEvents_t _RadioEvents;

  /**
   * @brief LoRa Chip state
   */
  static LoRaStates _state;

  // === Private Handlers ===

  /**
   * @brief Initialize the radio
   */
  void _initializeLora();

  /**
   * @brief Internal version of `sendPacket` that don't return busy when `_internalCallbacks` is enable
   *
   * @param data Data string to be sent
   * @param destinationAddress Destination node address (0 for broadcast)
   * @return int [0: ok, 1: busy]
   */
  int _sendPacket(const char *data, unsigned int destinationAddress = 0);

  /**
   * @brief Internal version of `sendReliablePacket` that don't return busy when `_internalCallbacks` is enable
   *
   * @param data Data string to be sent
   * @param destinationAddress Destination node address (broadcast not allowed)
   * @return int [0: ok, 1: busy]
   */
  int _sendReliablePacket(const char *data, unsigned int destinationAddress);

  /**
   * @brief Internal version of `listenToPacket` that don't return busy when `_internalCallbacks` is enable
   *
   * @param timeout Timeout for the listening in ms [0: continuous, any number]
   * @return int [0: ok, 1: busy]
   */
  int _listenToPacket(uint32_t timeout = 0);

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

  /**
   * @brief Check if a packet with the given node address and packet ID is in the buffer
   *
   * @param nodeAddress Address of the node to check
   * @param packetId Id of the packet to check
   * @return bool True if the packet is found in the buffer, false otherwise
   */
  static bool _isPacketInBuffer(unsigned int nodeAddress, unsigned int packetId);
};

/**
 * @brief Use this variable to manipulate this library
 */
extern HTLORAV3 LoRa;

#endif // HTLORAV3_H
