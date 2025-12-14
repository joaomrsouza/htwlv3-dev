/**
 * @file sclog.cpp
 * @brief Library for structured logging with node-based filtering and color output
 *
 * Description:
 *
 * This library provides a structured logging system that supports node-based filtering and colored output for serial communication.
 * It allows configuring different log levels per node, enabling selective logging in multi-node systems.
 *
 * Configuration:
 *
 * The library uses a `NodeLogConfig` structure to configure which log levels should be displayed for each node.
 * Multiple nodes can be configured, and each node can have multiple log levels enabled.
 *
 * Depends On:
 * - Arduino.h
 *
 * @author @joaomrsouza (João Marcos Rocha Souza)
 * https://github.com/joaomrsouza
 */

#include "sclog.h"

SCLOG::SCLOG(int currentNodeId, NodeLogConfig *nodeConfigs, int nodeConfigsCount)
{
  _currentNodeId = currentNodeId;
  _nodeConfigs = nodeConfigs;
  _nodeConfigsCount = nodeConfigsCount;
  _ownsConfigs = false;
}

SCLOG::~SCLOG()
{
  if (_ownsConfigs && _nodeConfigs != nullptr)
  {
    for (int i = 0; i < _nodeConfigsCount; i++)
      delete[] _nodeConfigs[i].levels;
    delete[] _nodeConfigs;
  }
}

void SCLOG::log(const char *str, COLORS color)
{
  log(String(str), color);
}

void SCLOG::log(const StringSumHelper &str, COLORS color)
{
  Serial.print(_getColorStr(color));
  Serial.println(String(_currentNodeId) + ": " + str);
  Serial.print(_getColorStr(RESET));
}

void SCLOG::log(const char *str, LOG_LEVELS level)
{
  log(String(str), level);
}

void SCLOG::log(const StringSumHelper &str, LOG_LEVELS level)
{
  if (!_shouldLogNode(level))
    return;

  log(str, _getLevelColor(level));
}

void SCLOG::log(const char *str, LOG_LEVELS level, COLORS color)
{
  log(String(str), level, color);
}

void SCLOG::log(const StringSumHelper &str, LOG_LEVELS level, COLORS color)
{
  if (!_shouldLogNode(level))
    return;

  log(str, color);
}

bool SCLOG::_shouldLogNode(LOG_LEVELS level)
{
  if (_nodeConfigs == nullptr)
    return false;

  for (int i = 0; i < _nodeConfigsCount; i++)
    if (_currentNodeId == _nodeConfigs[i].nodeId)
      for (int j = 0; j < _nodeConfigs[i].levelsCount; j++)
        if (_nodeConfigs[i].levels[j] == level)
          return true;

  return false;
}

SCLOG::COLORS SCLOG::_getLevelColor(LOG_LEVELS level)
{
  switch (level)
  {
  case INFO:
    return BLUE;
  case TRACE:
    return CYAN;
  case DEBUG:
    return MAGENTA;
  case WARN:
    return YELLOW;
  case ERROR:
    return RED;

  default:
    return RESET;
  }
}

const char *SCLOG::_getColorStr(COLORS color)
{
  static char colorStr[16]; // Buffer estático para armazenar a string de cor
  snprintf(colorStr, sizeof(colorStr), "\x1B[%dm", color);
  return colorStr;
}
