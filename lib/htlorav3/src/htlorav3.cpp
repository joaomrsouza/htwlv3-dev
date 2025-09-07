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

// === Static variables ===

bool HTLORAV3::_idle = true;
void (*HTLORAV3::_onReceive)(LoraDataPacket packet) = NULL;
void (*HTLORAV3::_onReceiveTimeout)() = NULL;
void (*HTLORAV3::_onSendDone)() = NULL;
void (*HTLORAV3::_onSendTimeout)() = NULL;
RadioEvents_t HTLORAV3::_RadioEvents;

// === Main Class ===

HTLORAV3::HTLORAV3()
{
  _config = getDefaultConfig();
  _idle = true;
}

// Clear memory on deconstruction
HTLORAV3::~HTLORAV3()
{
  stop();
}

void HTLORAV3::begin()
{
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
  _idle = true;
}

// === Getters ===

HTLORAV3Config HTLORAV3::getConfig() const
{
  return _config;
}

bool HTLORAV3::getIdle()
{
  return _idle;
}

HTLORAV3Config HTLORAV3::getDefaultConfig()
{
  HTLORAV3Config defaultConfig;

  defaultConfig.frequency = 915E6;
  defaultConfig.bandwidth = 0;
  defaultConfig.spreadingFactor = 7;
  defaultConfig.codingRate = 1;
  defaultConfig.preambleLength = 8;
  defaultConfig.fixLengthPayloadOn = false;
  defaultConfig.iqInversionOn = false;
  defaultConfig.txOutPower = 5;
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
}

int HTLORAV3::sendPacket(const char *data)
{
  if (!_idle)
    return 1; // Busy

  Radio.Send((uint8_t *)data, strlen(data));

  _idle = false;

  return 0;
}

void HTLORAV3::listenToPacket(uint32_t timeout)
{
  if (!_idle)
    return;

  _idle = false;
  Radio.Rx(timeout);
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
  _idle = true;

  if (_onSendDone != NULL)
    _onSendDone();
}

void HTLORAV3::_onTxTimeout()
{
  Radio.Sleep();
  _idle = true;

  if (_onSendTimeout != NULL)
    _onSendTimeout();
}

void HTLORAV3::_onRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
  LoraDataPacket packet;
  packet.data = (char *)malloc(size + 1);

  if (packet.data == NULL)
    return; // Not enough memory to receive data

  packet.rssi = rssi;
  packet.size = size;
  packet.snr = snr;

  memcpy(packet.data, payload, size);
  packet.data[size] = '\0';

  Radio.Sleep();

  if (_onReceive != NULL)
    _onReceive(packet);

  free(packet.data);

  _idle = true;
}

void HTLORAV3::_onRxTimeout()
{
  _idle = true;

  if (_onReceiveTimeout != NULL)
    _onReceiveTimeout();
}
HTLORAV3 LoRa;