#include "log.h"

LogFile error  (1024, " Error: "  , -1);
LogFile warning(1024, " Warning: ", -1);
LogFile info   (1024, " Info: "   , -1);
LogFile debug  (1024, " Debug: "  , -1);

std::mutex _LogStreamBase::_lock;
char _LogStreamBase::_date[DATE_BUFFER_SIZE];

void log(LogFile& logFile) {
  logFile.append('\n').out();
}

void log_open(const char *logPath) {
  error  .access(logPath);
  warning.access(logPath);
  info   .access(logPath);
  debug  .access(logPath);

  info.append("Opened log.\n").out();
}
