#include <Arduino.h>

#ifndef SCLOG_H
#define SCLOG_H

class SCLOG
{
public:
  enum LOG_LEVELS
  {
    DEBUG,
    INFO,
    TRACE,
    WARN,
    ERROR,
  };

  struct NodeLogConfig
  {
    int nodeId;
    LOG_LEVELS *levels;
    int levelsCount;
  };

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

  // Construtor que recebe array de configurações de nós
  SCLOG(int currentNodeId, NodeLogConfig *nodeConfigs, int nodeConfigsCount);

  ~SCLOG();

  void log(const StringSumHelper &str, COLORS color);
  void log(const char *str, COLORS color);

  void log(const StringSumHelper &str, LOG_LEVELS level);
  void log(const char *str, LOG_LEVELS level);

  void log(const StringSumHelper &str, LOG_LEVELS level, COLORS color);
  void log(const char *str, LOG_LEVELS level, COLORS color);

private:
  bool _shouldLogNode(LOG_LEVELS level);

  COLORS _getLevelColor(LOG_LEVELS level);

  const char *_getColorStr(COLORS color);

  int _currentNodeId;
  NodeLogConfig *_nodeConfigs;
  int _nodeConfigsCount;
  bool _ownsConfigs; // Indica se deve liberar memória no destrutor
};

#endif // SCLOG_H