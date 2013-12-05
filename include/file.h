#ifndef DOSSIER_FILE_H
#define DOSSIER_FILE_H

#include <sys/select.h>
#include <functional>

#include "stream.h"
/* Represents file in memory, storage or socket */
template <class Stream>
class _File {
	Stream _stream;

	std::vector<unsigned char> _cache;
  std::vector<unsigned char>::const_iterator _data_p;

  long _microsec;
public:
  // Change of cacheSize only affects next load
  int cacheSize;

  // Wether or not to close the file after deletion of _File
  bool close_after_delete = true;

	_File(int cacheSize, long microsec = -1) 
      : cacheSize(cacheSize), _microsec(microsec) {}

	_File(int cacheSize, int fd, long microsec = -1) 
      : cacheSize(cacheSize), _microsec(microsec) 
  {
		_stream = fd;
	}

	~_File() {
		if(close_after_delete && is_open())
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
		return _stream << _cache;
	}

	// Buffer pointers
	unsigned char next() {
		// Load new _cache if end of buffer is reached
  	if(end_of_buffer()) {
    	load(cacheSize);
    }

  	return *_data_p++;
	}

  void eachLoaded(std::function<bool(unsigned char)> f) {
    if(end_of_buffer()) {
      load(cacheSize);
    }

    while(!end_of_buffer()) {
      if(!f(*_data_p++))
        break;
    }
  }
	// Append buffer
	inline void append(std::vector<unsigned char>& buffer) {
		_cache.insert(_cache.end(), buffer.begin(), buffer.end());
	}

	inline void append(std::string &buffer) {
		_cache.insert(_cache.end(), buffer.begin(), buffer.end());
	}

  inline void append(unsigned char ch) {
    _cache.push_back(ch);
  }

  inline void append(std::string &&buffer) {
    append(buffer);
  }

  int access(std::string &&path) {
    return access(path);
  }

  int access(std::string& path) {
    if(is_open())
      seal();

    return _stream.access(path);
  }

	inline void flush() { _stream.flush(); }
	inline std::vector<unsigned char> &getCache() { return _cache; }
	inline bool eof() { return _stream.eof(); }
	inline bool end_of_buffer() {return _data_p == _cache.cend();}
	inline bool is_open() { return _stream.is_open(); }
	inline void seal() { _stream.seal(); }


  template<class Out=_File<Stream>>
  void copy(Out &out) {
    std::vector<unsigned char> &cache = out.getCache();

    while(!eof()) {
      cache.clear();
      eachLoaded([&](unsigned char data_p) {
        cache.push_back(data_p);
        return true;
      });
      out.out();
    }
  }
};

typedef _File<FileStream> ioFile;

#endif
