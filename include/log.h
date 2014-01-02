#ifndef DOSSIER_LOG_H
#define DOSSIER_LOG_H

#include <ctime>
#include <vector>
#include <string>
#include <mutex>

#include "file.h"

constexpr int DATE_BUFFER_SIZE = 21 + 1; // Full string plus '\0'
class _LogStreamBase {
public:
  static std::mutex _lock;
  static char _date[DATE_BUFFER_SIZE];
};

template<class Stream, class... Args>
class _LogStream : public _LogStreamBase {
  Stream _stream;

  std::string _prepend;
public:
  _LogStream() {}

  void operator =(_LogStream&& stream) {
    _stream = std::forward(stream);
  }

  void open(std::string&& prepend, Args... params) {
    _prepend = std::move(prepend);
    _stream.open(std::forward<Args>(params)...);
  }

  int operator >>(std::vector<unsigned char>& buf) {
    return 0;
  }

  int operator <<(std::vector<unsigned char>& buf) {
    std::lock_guard<std::mutex> lock(_LogStreamBase::_lock);
    
    std::time_t t = std::time(NULL);
    strftime(_LogStreamBase::_date, DATE_BUFFER_SIZE, "[%Y:%m:%d:%H:%M:%S]", std::localtime(&t));

    buf.insert(buf.begin(), _prepend.cbegin(), _prepend.cend());

    buf.insert(
               buf.begin(),
               _LogStreamBase::_date,
               _LogStreamBase::_date + DATE_BUFFER_SIZE -1 //omit '\0'
    ); 

    buf.push_back('\n');
    return _stream << buf;
  }

  bool is_open() {
    return _stream.is_open();
  }

  bool eof() {
    return false;
  }

  int access(std::string &path) {
    return _stream.access(path);
  }

  void seal() {
    _stream.seal();
  }

  int fd() {
    return _stream.fd();
  }
};

#define LogStream _LogStream<FileStream>, std::string &&, int
typedef FD<LogStream> LogFile;

extern void log_open(const char *logPath);

extern LogFile error;
extern LogFile warning;
extern LogFile info;
extern LogFile debug;
#endif
