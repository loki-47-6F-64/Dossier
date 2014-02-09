#ifndef DOSSIER_FILE_H
#define DOSSIER_FILE_H

#include <sys/select.h>
#include <functional>

#include "stream.h"

namespace FileErr {
  enum {
    STREAM_ERR = -1,
    OK,
    TIMEOUT,
    COPY_STREAM_ERR,
    BREAK, // Break from eachByte()
    CUSTOM_ERR, // Error propagated from eachByte()
  };
};

/* Represents file in memory, storage or socket */
template <class Stream, class... Args>
class FD /* File descriptor */ {
  static constexpr int READ = 0, WRITE = 1;

  Stream _stream;

  std::vector<unsigned char> _cache;
  std::vector<unsigned char>::const_iterator _data_p;

  long _microsec;

public:
  // Change of cacheSize only affects next load
  int cacheSize;

  FD(FD&& other) {
    _stream = std::move(other._stream);
    _cache  = std::move(other._cache);

    _data_p = _cache.cbegin();
    _microsec = other._microsec;
    cacheSize = other.cacheSize;
  }

  void operator=(FD&& other) {
    _stream = std::move(other._stream);
    _cache  = std::move(other._cache);

    _data_p = _cache.cbegin();
    _microsec = other._microsec;
    cacheSize = other.cacheSize;
  }

  FD(int cacheSize, long microsec = -1) 
      : cacheSize(cacheSize), _microsec(microsec) {}

  FD(int cacheSize, long microsec, Args... params) 
      : cacheSize(cacheSize), _microsec(microsec)
  {
    _stream.open(std::forward<Args>(params)...);
  }

  ~FD() {
    seal();
  }

  // Load file into _cache.data(), replaces old _cache.data()
  /* Returns -1 on error.
     Returns  0 on succes
     Returns  1 on timeout */
  int load(size_t max_bytes) {
    _cache.resize(max_bytes);

    int err;
    if((err = _select(READ))) {
      return err;
    }

    if((_stream >> _cache) < 0)
      return FileErr::STREAM_ERR;

    _data_p = _cache.cbegin();
    return FileErr::OK;
  }

  // Write to file
  inline int out() {
    int err;
    if((err = _select(WRITE))) {
      return err;
    }

    // On success clear
    if((_stream << _cache) >= 0) {
      clear();
      return FileErr::OK;
    }
    return FileErr::STREAM_ERR;
  }

  // Usefull when fine controll is nessesary
  unsigned char next() {
    // Load new _cache if end of buffer is reached
    if(end_of_buffer()) {
      if(load(cacheSize)) {
        return '\0';
      }
    }

    // If _cache.empty() return '\0'
    return _cache.empty() ? '\0' : *_data_p++;
  }

  int eachByte(std::function<int(unsigned char)> f) {
    while(!eof()) {
      if(end_of_buffer()) {
        int err;
        if((err = load(cacheSize))) {
          return err;
        }

        continue;
      }

      int custom_err;
      if((custom_err = f(*_data_p++)))
        return custom_err > FileErr::BREAK ? custom_err : FileErr::OK;
    }

    return FileErr::OK;
  }

  // Append buffer
  inline FD& append(std::vector<unsigned char>& buffer) {
    _cache.insert(_cache.end(), buffer.begin(), buffer.end());

    return *this;
  }

  inline FD& append(const unsigned char ch) {
    _cache.push_back(ch);

    return *this;
  }

  inline FD& append(const char ch) {
    return append(static_cast<unsigned char>(ch));
  }

  inline FD& append(const long long integer) {
    return append(std::to_string(integer));
  }

  inline FD& append(const unsigned long long integer) {
    return append(std::to_string(integer));
  }

  inline FD& append(const long integer) {
    return append(std::to_string(integer));
  }

  inline FD& append(const unsigned long integer) {
    return append(std::to_string(integer));
  }

  inline FD& append(const int integer) {
    return append(std::to_string(integer));
  }

  inline FD& append(const unsigned int integer) {
    return append(std::to_string(integer));
  }

  inline FD& append(std::string const &buffer) {
    _cache.insert(_cache.end(), buffer.begin(), buffer.end());

    return *this;
  }

  // const char *
  inline FD& append(std::string &&buffer) {
    return append(buffer);
  }

  inline FD& clear() {
    _cache.clear();
    _data_p = _cache.cbegin();


    return *this;
  }

  int access(const char *path, std::function<int(Stream &, const char *)> open) {
    seal();

    return open(_stream, path);
  }

  int access(std::string& path, std::function<int(Stream &, const char *)> open) {
    return access(path.c_str(), open);
  }

  inline std::vector<unsigned char> &getCache() { return _cache; }
  inline bool eof() { return _stream.eof(); }
  inline bool end_of_buffer() {return _data_p == _cache.cend();}
  inline bool is_open() { return _stream.is_open(); }

  inline void seal() {
    if(is_open())
      _stream.seal();
  }


  /* 
     Copies max bytes from this to out
     If max == -1 copy the whole file
     On failure: return (Error)-1 or (Timeout)1
     On success: return  0
  */
  template<class Out>
  int copy(Out &out, int64_t max = -1) {
    std::vector<unsigned char> &cache = out.getCache();

    while(!eof() && max) {
      if(end_of_buffer()) {
        int err;
        if((err = load(cacheSize))) {
          return err; // STREAM_ERR or TIMEOUT
        }
      }

      while(!end_of_buffer()) {
        cache.push_back(*_data_p++);

        if(!max)
          break;

        --max;
      }
      if(out.out()) {
        return FileErr::COPY_STREAM_ERR;
      }
    }
    return FileErr::OK;
  }
private:

  int _select(const int read) {
    if(_microsec >= 0) {
      timeval tv { 0, _microsec };

      fd_set selected;

      FD_ZERO(&selected);
      FD_SET(_stream.fd(), &selected);

      int result;
      if(read == READ) {
        result = select(_stream.fd() + 1, &selected, nullptr, nullptr, &tv);
      }
      else if(read == WRITE) {
        result = select(_stream.fd() + 1, nullptr, &selected, nullptr, &tv);
      }

      if(result <= 0) {
        /* f ==  0 -> return  1
           f == -1 -> return -1 */
        return 1 + 2*result;
      }
    }

    return 0;
  }
};

template<class File>
void _print(File& file) {
  file.out();
}

template<class File, class Out, class... Args>
void _print(File& file, Out&& out, Args&&... params) {
  file.append(out);
  _print(file, std::forward<Args>(params)...);
}

/*
 * First clear file, then recursively print all params
 */
template<class File, class... Args>
void print(File& file, Args&&... params) {
  file.clear();

  _print(file, std::forward<Args>(params)...);
}

typedef FD<FileStream> ioFile;
typedef FD<SslStream> sslFile;
#endif
