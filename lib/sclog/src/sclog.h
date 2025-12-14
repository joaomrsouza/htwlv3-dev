/**
 * @file sclog.h
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

#ifndef SCLOG_H
#define SCLOG_H

#include <Arduino.h>

/**
 * @brief A class for structured logging with node-based filtering and color output
 *
 * This class provides logging functionality with support for:
 * - Node-based log filtering
 * - Multiple log levels (DEBUG, INFO, TRACE, WARN, ERROR)
 * - Colored output for serial communication
 * - Configurable log levels per node
 */
class SCLOG
{
public:
  /**
   * @brief Log levels enumeration
   */
  enum LOG_LEVELS
  {
    DEBUG,
    INFO,
    TRACE,
    WARN,
    ERROR,
  };

  /**
   * @brief Configuration structure for node log settings
   */
  struct NodeLogConfig
  {
    // Node identifier
    int nodeId;
    // Array of enabled log levels for this node
    LOG_LEVELS *levels;
    // Number of log levels in the array
    int levelsCount;
  };

  /**
   * @brief ANSI color codes enumeration
   */
  enum COLORS
  {
    RESET = 0,
    RED = 31,
    GREEN = 32,
    YELLOW = 33,
    BLUE = 34,
    MAGENTA = 35,
    CYAN = 36,
  };

  /**
   * @brief Constructor that receives an array of node configurations
   *
   * @param currentNodeId Current node identifier
   * @param nodeConfigs Array of node log configurations
   * @param nodeConfigsCount Number of node configurations in the array
   */
  SCLOG(int currentNodeId, NodeLogConfig *nodeConfigs, int nodeConfigsCount);
  ~SCLOG();

  /**
   * @brief Log a message with a specific color
   *
   * @param str Message to log
   * @param color Color to use for the log output
   */
  void log(const char *str, COLORS color);
  void log(const StringSumHelper &str, COLORS color);

  /**
   * @brief Log a message with a specific log level
   *
   * The message will only be logged if the current node has this log level enabled.
   * The color will be automatically selected based on the log level.
   *
   * @param str Message to log
   * @param level Log level
   */
  void log(const char *str, LOG_LEVELS level);
  void log(const StringSumHelper &str, LOG_LEVELS level);

  /**
   * @brief Log a message with a specific log level and color
   *
   * The message will only be logged if the current node has this log level enabled.
   *
   * @param str Message to log
   * @param level Log level
   * @param color Color to use for the log output
   */
  void log(const char *str, LOG_LEVELS level, COLORS color);
  void log(const StringSumHelper &str, LOG_LEVELS level, COLORS color);

private:
  /**
   * @brief Current node identifier
   */
  int _currentNodeId;

  /**
   * @brief Array of node log configurations
   */
  NodeLogConfig *_nodeConfigs;

  /**
   * @brief Number of node configurations in the array
   */
  int _nodeConfigsCount;

  /**
   * @brief Indicates if the class should free memory in the destructor
   */
  bool _ownsConfigs;

  /**
   * @brief Check if the current node should log messages at the specified level
   *
   * @param level Log level to check
   * @return true if the node should log at this level, false otherwise
   */
  bool _shouldLogNode(LOG_LEVELS level);

  /**
   * @brief Get the default color for a log level
   *
   * @param level Log level
   * @return COLORS Color associated with the log level
   */
  COLORS _getLevelColor(LOG_LEVELS level);

  /**
   * @brief Get the ANSI color string for a color code
   *
   * @param color Color code
   * @return const char* ANSI escape sequence string
   */
  const char *_getColorStr(COLORS color);
};

#endif // SCLOG_H