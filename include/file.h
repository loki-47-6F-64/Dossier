#ifndef DOSSIER_FILE_H
#define DOSSIER_FILE_H

#include <sys/select.h>
#include <functional>

#include "stream.h"

namespace FileErr {
  enum {
    SYS_ERROR = -1,
    OK,
    TIMEOUT
  };
};

/* Represents file in memory, storage or socket */
template <class Stream, class... Args>
class FD /* File descriptor */ {
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

	FD(int cacheSize, long microsec = -1) 
      : cacheSize(cacheSize), _microsec(microsec) {}

	FD(int cacheSize, long microsec, Args... params) 
      : cacheSize(cacheSize), _microsec(microsec)
  {
		_stream.open(std::forward<Args>(params)...);
	}

	~FD() {
		if(is_open())
			seal();
	}

	// Load file into _cache.data(), replaces old _cache.data()
  /* Returns -1 on error.
     Returns  0 on succes
     Returns  1 on timeout */
	int load(size_t max_bytes) {
		_cache.resize(max_bytes);

    if(_microsec >= 0) {
      timeval tv { 0, _microsec };

      fd_set read;

      FD_ZERO(&read);
      FD_SET(_stream.fd(), &read);

      int f = select(_stream.fd() + 1, &read, nullptr, nullptr, &tv);
      if(f <= 0) {
        /* f ==  0 -> return  1
           f == -1 -> return -1 */
        return 1 + 2*f;
      }
    }

  	if((_stream >> _cache) < 0)
    	return -1;

  	_data_p = _cache.cbegin();
  	return 0;
	}

	// Write to file
	inline int out() {
		if((_stream << _cache) >= 0) {
      clear();
      return 0;
    }
    return -1;
	}

  // Usefull when fine controll is nessesary
	unsigned char next() {
		// Load new _cache if end of buffer is reached
  	if(end_of_buffer()) {
    	load(cacheSize);
    }

    // If _cache.empty() return '\0'
  	return _cache.empty() ? '\0' : *_data_p++;
	}

  int eachByte(std::function<bool(unsigned char)> f) {
    while(!eof()) {
      if(end_of_buffer()) {
        if(load(cacheSize)) {
          return -1;
        }
      }

      if(!f(*_data_p++))
        break;
    }

    return 0;
  }

	// Append buffer
	inline FD& append(std::vector<unsigned char>& buffer) {
		_cache.insert(_cache.end(), buffer.begin(), buffer.end());

    return *this;
	}

	inline FD& append(std::string &buffer) {
		_cache.insert(_cache.end(), buffer.begin(), buffer.end());

    return *this;
	}

  inline FD& append(unsigned char ch) {
    _cache.push_back(ch);

    return *this;
  }

  inline FD& append(char ch) {
    return append(static_cast<unsigned char>(ch));
  }

  inline FD& append(long integer) {
    return append(std::to_string(integer));
  }

  inline FD& append(int integer) {
    return append(std::to_string(integer));
  }

  inline FD& append(std::string &&buffer) {
    return append(buffer);
  }

  inline FD& clear() {
    _cache.clear();

    return *this;
  }

  int access(std::string &&path) {
    return access(path);
  }

  int access(std::string& path) {
    if(is_open())
      seal();

    return _stream.access(path);
  }

	inline std::vector<unsigned char> &getCache() { return _cache; }
	inline bool eof() { return _stream.eof(); }
	inline bool end_of_buffer() {return _data_p == _cache.cend();}
	inline bool is_open() { return _stream.is_open(); }
	inline void seal() { _stream.seal(); }


  /* 
     Copies max bytes from this to out
     If max == -1 copy the whole file
     On failure: return (Error)-1 or (Timeout)1
     On success: return  0
     On attempt to copy more bytes than max: return 2
  */
  template<class Out>
  int copy(Out &out, int64_t max = -1) {
    std::vector<unsigned char> &cache = out.getCache();

    while(!eof() && max) {
      if(end_of_buffer()) {
        int err;
        if((err = load(cacheSize))) {
          return err;
        }
      }

      while(!end_of_buffer()) {
        cache.push_back(*_data_p++);

        if(!max--) {
          return 2;
        }
      }
      out.out();
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
