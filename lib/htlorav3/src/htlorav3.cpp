/**
 * @file htlorav3.cpp
 * @brief Library to use LoRa with the HelTec WiFi LoRa 32 V3 Board
 *
 * Description:
 *
 * This library abstracts commands for the board peripheral LoRa Chip.
 *
 * Configuration:
 *
 * It's possible to configure some parameters for the LoRa Chip by using the `setConfig()` and `updateConfig()` methods.
 *
 * Depends On:
 * - heltecautomation/Heltec ESP32 Dev-Boards@2.0.2
 *
 * @author @joaomrsouza (João Marcos Rocha Souza)
 * https://github.com/joaomrsouza
 */

#include "htlorav3.h"

#define HTLORAV3_HEADER_SIZE 11

// === Static variables ===

// Node Control
unsigned int HTLORAV3::_address = 0;
HTLORAV3::LoRaStates HTLORAV3::_state = HTLORAV3::IDLE;
int HTLORAV3::_currentPacketId = 0;

// Ack Control
bool HTLORAV3::_internalCallbacks = false;
unsigned int HTLORAV3::_sendACKTo = 0;
bool HTLORAV3::_ignorePacket = false;
LoraDataPacket HTLORAV3::_lastPacket = {NULL, 0, 0, 0};

// Receive Timeout
unsigned long HTLORAV3::_receiveTimeoutMillis = 0;
unsigned long HTLORAV3::_receiveTimeoutTimestamp = 0;

// Duplication packet check
ReceivedPacketInfo HTLORAV3::_receivedPacketsList[10] = {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}};
int HTLORAV3::_receivedPacketsIndex = 0;
int HTLORAV3::_receivedPacketsCount = 0;

// Event handlers
void (*HTLORAV3::_onReceive)(LoraDataPacket packet) = NULL;
void (*HTLORAV3::_onReceiveTimeout)() = NULL;
void (*HTLORAV3::_onSendDone)() = NULL;
void (*HTLORAV3::_onSendTimeout)() = NULL;
RadioEvents_t HTLORAV3::_RadioEvents;

// === Main Class ===

HTLORAV3::HTLORAV3()
{
  _config = getDefaultConfig();
  _state = IDLE;
  _currentPacketId = 0;

  _internalCallbacks = false;
  _sendACKTo = 0;
  _ignorePacket = false;

  _receiveTimeoutMillis = 0;
  _receiveTimeoutTimestamp = 0;

  for (int i = 0; i < 10; i++)
  {
    _receivedPacketsList[i].nodeAddress = 0;
    _receivedPacketsList[i].packetId = 0;
  }
  _receivedPacketsIndex = 0;
  _receivedPacketsCount = 0;
}

// Clear memory on deconstruction
HTLORAV3::~HTLORAV3()
{
  stop();

  if (_lastPacket.data != NULL)
  {
    free(_lastPacket.data);
    _lastPacket.data = NULL;
  }
}

void HTLORAV3::begin(unsigned int address)
{
  _address = address;

  // Pass the board type and the slow clock type for Heltec Wifi LoRa 32 V3 Board
  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);

  // Bind Radio events
  _RadioEvents.TxDone = _onTxDone;
  _RadioEvents.TxTimeout = _onTxTimeout;
  _RadioEvents.RxDone = _onRxDone;
  _RadioEvents.RxTimeout = _onRxTimeout;

  // Initialize and configure Radio
  Radio.Init(&_RadioEvents);

  // Initialize radio with config
  _initializeLora();
}

void HTLORAV3::stop()
{
  Radio.Sleep();
  _state = IDLE;
}

// === Getters ===

HTLORAV3Config HTLORAV3::getConfig() const
{
  return _config;
}

HTLORAV3::LoRaStates HTLORAV3::getState()
{
  return _state;
}

HTLORAV3Config HTLORAV3::getDefaultConfig()
{
  HTLORAV3Config defaultConfig;

  defaultConfig.frequency = 433E6;
  defaultConfig.bandwidth = 0;
  defaultConfig.spreadingFactor = 7;
  defaultConfig.codingRate = 1;
  defaultConfig.preambleLength = 8;
  defaultConfig.fixLengthPayloadOn = false;
  defaultConfig.iqInversionOn = false;
  defaultConfig.txOutPower = 24;
  defaultConfig.txTimeout = 3000;
  defaultConfig.rxTimeout = 0;

  return defaultConfig;
}

// === Setters ===

void HTLORAV3::setConfig(const HTLORAV3Config &config)
{
  _config = config;
}

void HTLORAV3::updateConfig(const HTLORAV3Config &config)
{
  stop();

  setConfig(config);

  _initializeLora();
}

void HTLORAV3::setOnReceive(void (*onReceive)(LoraDataPacket packet))
{
  _onReceive = onReceive;
}

void HTLORAV3::setOnReceiveTimeout(void (*onReceiveTimeout)())
{
  _onReceiveTimeout = onReceiveTimeout;
}

void HTLORAV3::setOnSendDone(void (*onSendDone)())
{
  _onSendDone = onSendDone;
}

void HTLORAV3::setOnSendTimeout(void (*onSendTimeout)())
{
  _onSendTimeout = onSendTimeout;
}

// === Handlers ===

void HTLORAV3::process()
{
  Mcu.timerhandler();
  Radio.IrqProcess();

  if (
      _receiveTimeoutTimestamp > 0 &&
      _receiveTimeoutMillis > 0 &&
      ((millis() - _receiveTimeoutTimestamp) >= _receiveTimeoutMillis))
  {
    _receiveTimeoutTimestamp = 0;
    _receiveTimeoutMillis = 0;
    _onRxTimeout();
    return;
  }

  if (_internalCallbacks && _state == IDLE && _sendACKTo > 0)
  {
    delay(50); // Wait while the destiny enters in listen mode
    _sendReliablePacket("ACK", _sendACKTo);
    if (_ignorePacket)
      _ignorePacket = false;
    else if (_onReceive != NULL)
      _onReceive(_lastPacket);
  }
}

int HTLORAV3::sendPacket(const char *data, unsigned int destinationAddress)
{
  if (_internalCallbacks)
    return 1; // User send packet returns busy if in internal callbacks

  return _sendPacket(data, destinationAddress);
}

int HTLORAV3::_sendPacket(const char *data, unsigned int destinationAddress)
{
  if (_state != IDLE)
    return 1;

  _currentPacketId++;
  if (_currentPacketId > 99)
    _currentPacketId = 1;

  // Create header with padded node origin address (3 chars) destination address (3 chars) and packet id (2 chars)
  char header[HTLORAV3_HEADER_SIZE + 1]; // headerSize + '\0'
  sprintf(header, "%03d-%03d-%02d|", _address, destinationAddress, _currentPacketId);

  String preparedData = String(header) + String(data);

  Radio.Send((uint8_t *)preparedData.c_str(), preparedData.length());

  _state = SENDING;

  return 0;
}

int HTLORAV3::sendReliablePacket(const char *data, unsigned int destinationAddress)
{
  if (destinationAddress != 0 && _address == 0)
    throw std::runtime_error("Address is not set. To send packets to certain address, set the address in both nodes first.");

  if (_internalCallbacks)
    return 1; // User send reliable packet returns busy if in internal callbacks

  return _sendReliablePacket(data, destinationAddress);
}

int HTLORAV3::_sendReliablePacket(const char *data, unsigned int destinationAddress)
{
  if (destinationAddress <= 0)
    throw std::runtime_error("Broadcast is not allowed on sendRealiablePacket.");

  if (_state != IDLE)
    return 1;

  bool acked = strcmp(data, "ACK") == 0;

  _internalCallbacks = true;

  // TODO: implementar max resend
  // TODO: Ajustar melhor timeouts e tries threshold
  // int reliableSendTries = 0;
  int ackTimeoutTries;
  const int maxAckTries = 5;
  do
  {
    ackTimeoutTries = 0;

    do
    {
      _state = IDLE;
      randomSeed(millis());
      delay(random(0, 500)); // Minimize packet colision
      _sendPacket(data, destinationAddress);

      while (_state == SENDING)
        process();

    } while (_state == SEND_TIMEOUT);

    if (acked)
      return 0;

    do
    {
      _state = IDLE;
      randomSeed(millis());
      _listenToPacket(500);

      while (_state == RECEIVING)
        process();

      if (_state == RECEIVE_TIMEOUT)
        ackTimeoutTries++;
    } while (_state == RECEIVE_TIMEOUT && ackTimeoutTries < maxAckTries);
    // reliableSendTries++;
  } while (ackTimeoutTries >= maxAckTries);

  if (_onSendDone != NULL)
    _onSendDone();

  return 0;
}

int HTLORAV3::listenToPacket(uint32_t timeout)
{
  if (_internalCallbacks)
    return 1; // User send packet returns busy if in internal callbacks

  return _listenToPacket(timeout);
}

int HTLORAV3::_listenToPacket(uint32_t timeout)
{
  if (_state != IDLE)
    return 1;

  _receiveTimeoutMillis = timeout;
  _receiveTimeoutTimestamp = millis();

  _state = RECEIVING;
  Radio.Rx(0);

  return 0;
}

bool HTLORAV3::_isPacketInBuffer(unsigned int nodeAddress, unsigned int packetId)
{
  if (_receivedPacketsCount == 0)
    return false;

  // Search through all packets in the circular buffer
  for (int i = 0; i < _receivedPacketsCount; i++)
  {
    int index;
    if (_receivedPacketsCount < 10) // Buffer not full yet, packets are stored sequentially from index 0
      index = i;
    else // Buffer is full and circular, start from the oldest packet
      index = (_receivedPacketsIndex + i) % 10;

    if (_receivedPacketsList[index].nodeAddress == nodeAddress &&
        _receivedPacketsList[index].packetId == packetId)
      return true;
  }

  return false;
}

// === Private Handlers ===

void HTLORAV3::_initializeLora()
{
  Radio.SetChannel(_config.frequency);

  Radio.SetTxConfig(
      MODEM_LORA,
      _config.txOutPower,
      0,
      _config.bandwidth,
      _config.spreadingFactor,
      _config.codingRate,
      _config.preambleLength,
      _config.fixLengthPayloadOn,
      true,
      0,
      0,
      _config.iqInversionOn,
      _config.txTimeout);

  Radio.SetRxConfig(
      MODEM_LORA,
      _config.bandwidth,
      _config.spreadingFactor,
      _config.codingRate,
      0,
      _config.preambleLength,
      _config.rxTimeout,
      _config.fixLengthPayloadOn,
      0,
      true,
      0,
      0,
      _config.iqInversionOn,
      true);
}

void HTLORAV3::_onTxDone()
{
  if (_onSendDone != NULL && !_internalCallbacks)
    _onSendDone();

  if (_sendACKTo > 0)
  {
    _internalCallbacks = false;
    _sendACKTo = 0;
  }

  _state = IDLE;
}

void HTLORAV3::_onTxTimeout()
{
  Radio.Sleep();
  _state = _internalCallbacks ? SEND_TIMEOUT : IDLE;

  if (_onSendTimeout != NULL && !_internalCallbacks)
    _onSendTimeout();
}

void HTLORAV3::_onRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
  int originAddress = -1;
  int destinationAddress = -1;
  int packetId = -1;
  bool hasHeader = false;

  // Extract header if present
  if (size >= HTLORAV3_HEADER_SIZE)
  {
    char header[HTLORAV3_HEADER_SIZE + 1];
    memcpy(header, payload, HTLORAV3_HEADER_SIZE);
    header[HTLORAV3_HEADER_SIZE] = '\0';

    // Parse header format: XXX-XXX-XX|
    if (header[3] == '-' && header[7] == '-' && header[10] == '|')
    {
      hasHeader = true;
      char originAddressStr[4] = {header[0], header[1], header[2], '\0'};
      char destinationAddressStr[4] = {header[4], header[5], header[6], '\0'};
      char packetIdStr[3] = {header[8], header[9], '\0'};

      originAddress = atoi(originAddressStr);
      destinationAddress = atoi(destinationAddressStr);
      packetId = atoi(packetIdStr);
    }
  }

  if (destinationAddress > 0 && destinationAddress != _address)
    return; // Packet not for this node

  bool packetInBuffer = _isPacketInBuffer(originAddress, packetId);

  if (packetInBuffer)
    _ignorePacket = true;
  else
  {
    _receiveTimeoutMillis = 0;
    _receiveTimeoutTimestamp = 0;
  }

  uint16_t dataSize = hasHeader ? (size - HTLORAV3_HEADER_SIZE) : size;
  uint16_t dataOffset = hasHeader ? HTLORAV3_HEADER_SIZE : 0;

  LoraDataPacket packet;
  packet.data = (char *)malloc(dataSize + 1);

  if (packet.data == NULL)
    throw std::runtime_error("Not enough memory to receive packet.");

  packet.rssi = rssi;
  packet.size = dataSize;
  packet.snr = snr;

  // Copy only the data without the header
  memcpy(packet.data, payload + dataOffset, dataSize);
  packet.data[dataSize] = '\0';

  bool acked = strcmp(packet.data, "ACK") == 0;

  if (destinationAddress > 0)
  {
    _internalCallbacks = !acked;
    _sendACKTo = acked ? 0 : originAddress;
  }

  if (destinationAddress > 0 && !acked)
  {
    if (_lastPacket.data != NULL)
      free(_lastPacket.data);

    _lastPacket.data = (char *)malloc(dataSize + 1);
    if (_lastPacket.data != NULL)
    {
      memcpy(_lastPacket.data, packet.data, dataSize + 1);
      _lastPacket.rssi = packet.rssi;
      _lastPacket.size = packet.size;
      _lastPacket.snr = packet.snr;
    }

    // Store packet info in the list (only if we have valid header info)
    if (hasHeader && originAddress != -1 && packetId != -1 && !packetInBuffer)
    {
      _receivedPacketsList[_receivedPacketsIndex].nodeAddress = originAddress;
      _receivedPacketsList[_receivedPacketsIndex].packetId = packetId;

      // Update circular buffer index
      _receivedPacketsIndex = (_receivedPacketsIndex + 1) % 10;

      // Update count (max 10)
      if (_receivedPacketsCount < 10)
        _receivedPacketsCount++;
    }
  }

  Radio.Sleep();

  if (_onReceive != NULL && !_internalCallbacks && !acked)
    _onReceive(packet);

  free(packet.data);
  _state = IDLE;
}

void HTLORAV3::_onRxTimeout()
{
  _state = _internalCallbacks ? RECEIVE_TIMEOUT : IDLE;

  if (_onReceiveTimeout != NULL && !_internalCallbacks)
    _onReceiveTimeout();
}
HTLORAV3 LoRa;