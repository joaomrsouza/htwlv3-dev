/**
 * @file htlorav3.cpp
 * @brief Library to use LoRa with the HelTec WiFi LoRa 32 V3 Board
 *
 * This library abstracts commands for the board peripheral LoRa Chip.
 *
 * Configuration:
 *
 * It's possible to reconfigure some parameters by redefining them in the main file, check "=== Default Config ===" section in the header file.
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
RadioEvents_t HTLORAV3::_RadioEvents;

// === Main Class ===

HTLORAV3::HTLORAV3()
{
  _idle = true;
}

void HTLORAV3::begin()
{
  // TODO: Fazer anotações sobre essas vars elas estão como flag no compilador mas não podem ser alteradas
  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);

  // Bind Radio events
  _RadioEvents.TxDone = _onTxDone;
  _RadioEvents.TxTimeout = _onTxTimeout;
  _RadioEvents.RxDone = _onRxDone;

  // Initialize and configure Radio
  Radio.Init(&_RadioEvents);
  Radio.SetChannel(HTLORAV3_FREQUENCY);
  Radio.SetTxConfig(MODEM_LORA, HTLORAV3_TX_OUT_POWER, 0, HTLORAV3_BANDWIDTH,
                    HTLORAV3_SPREADING_FACTOR, HTLORAV3_CODINGRATE,
                    HTLORAV3_PREAMBLE_LENGTH, HTLORAV3_FIX_LENGTH_PAYLOAD_ON,
                    true, 0, 0, HTLORAV3_IQ_INVERSION_ON, 3000);
  Radio.SetRxConfig(MODEM_LORA, HTLORAV3_BANDWIDTH, HTLORAV3_SPREADING_FACTOR,
                    HTLORAV3_CODINGRATE, 0, HTLORAV3_PREAMBLE_LENGTH,
                    HTLORAV3_RX_TIMEOUT, HTLORAV3_FIX_LENGTH_PAYLOAD_ON,
                    0, true, 0, 0, HTLORAV3_IQ_INVERSION_ON, true);
}

// === Getters ===

bool HTLORAV3::getIdle()
{
  return _idle;
}

// === Handlers ===

void HTLORAV3::process()
{
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

#ifdef USE_SSTREAM
int HTLORAV3::sendPacket(std::stringstream streamData)
{
  const char *data = streamData.str().c_str();

  if (!_idle)
    return 1; // Busy

  Radio.Send((uint8_t *)data, strlen(data));

  _idle = false;

  return 0;
}
#endif

void HTLORAV3::setOnReceive(void (*onReceive)(LoraDataPacket packet))
{
  _onReceive = onReceive;
}

void HTLORAV3::listenToPacket(uint32_t timeout)
{
  if (!_idle)
    return;

  _idle = false;
  Radio.Rx(timeout);
}

// === Static Handlers ===

void HTLORAV3::_onTxDone()
{
  _idle = true;
}

void HTLORAV3::_onTxTimeout()
{
  Radio.Sleep();
  _idle = true;
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

HTLORAV3 LoRa;