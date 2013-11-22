#include <errno.h>
#include <string.h>
#include <ctime>

#include "main.h"
namespace Log {
  char date[DATE_BUFFER_SIZE];


  std::ofstream f_out;
  std::mutex log_lock;
};

void Log::open(const char *path) {
  f_out.open(path, std::ios::app);
}

void Log::close() {
  f_out << std::endl;
  f_out.close();
}

void Log::Error(const char *msg, int err) {
	if(!errno) {
		Error(msg);
		return;
	}

  std::lock_guard<std::mutex>lock(log_lock);
  _scanDate();

  f_out << date << " Error: " << msg << ": " << strerror(err) << std::endl;
}

void Log::Warning(const char *msg, int err) {
  std::lock_guard<std::mutex>lock {log_lock};
  _scanDate();
  
  f_out << date << " Warning: " << msg << ": " << strerror(err) << std::endl;
}

void Log::Info(const char *msg, int err) {
  std::lock_guard<std::mutex>lock{log_lock};
  _scanDate();

  f_out << date << " Info: " << msg << ": " << strerror(err) << std::endl;
}

void Log::Error(const char* msg) {
  std::lock_guard<std::mutex>lock{log_lock};
  _scanDate();

  f_out << date << " Error: " << msg << std::endl;
}

void Log::Warning(const char *msg) {
  std::lock_guard<std::mutex>lock{log_lock};
  _scanDate();

  f_out << date << " Warning: " << msg << std::endl;
}

void Log::Info(const char *msg) {
  std::lock_guard<std::mutex>lock{log_lock};
  _scanDate();

  f_out << date << " Info: " << msg << std::endl;
}

void Log::Debug(const char *file_name, int line, const char *msg) {
  ON_DEBUG(
    std::lock_guard<std::mutex>lock{log_lock};
    _scanDate();

    f_out << date << 
					" Debug: " << file_name 
					<< ": " << line 
					<< " :" << msg 
					<< std::endl;
  )
}

void Log::Debug(const char *file_name, int line, int64_t msg) {
  ON_DEBUG(
    std::lock_guard<std::mutex>lock{log_lock};
    _scanDate();

    f_out << date <<
          " Debug: " << file_name
          << ": " << line
          << " :" << msg
          << std::endl;
  )
}

void Log::_scanDate() {
  std::time_t t = std::time(NULL);
  strftime(date, sizeof(date), "[%Y:%m:%d:%H:%M:%S]", std::localtime(&t));
}
