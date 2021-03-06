#include "file/log.h"

LogFile error  (1024, -1, " Error: "  , -1);
LogFile warning(1024, -1, " Warning: ", -1);
LogFile info   (1024, -1, " Info: "   , -1);
LogFile debug  (1024, -1, " Debug: "  , -1);

std::mutex _LogStreamBase::_lock;
char _LogStreamBase::_date[DATE_BUFFER_SIZE];

int logStreamWrite(_LogStream<FileStream> &ls, const char *path) {
  return ls.access(path, fileStreamWrite);
}

void log_open(const char *logPath) {
  error  .access(logPath, logStreamWrite);
  warning.access(logPath, logStreamWrite);
  info   .access(logPath, logStreamWrite);
  debug  .access(logPath, logStreamWrite);

  info.append("Opened log.\n").out();
}
