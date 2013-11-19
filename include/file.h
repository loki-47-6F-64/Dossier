#ifndef DOSSIER_FILE_H
#define DOSSIER_FILE_H



#include "stream.h"

/* Represents file in memory, storage or socket */
template <class Stream>
class _File {
	Stream _stream;

	int _data_p;
	std::vector<unsigned char> _cache;

public:
	_File() {}
	_File(int fd) {
		_stream = fd;
	}

	~_File() {
		if(is_open())
			seal();
	}

	int access(std::string&& path) {
		if(is_open())
    	seal();

  	return _stream.access(path);
	}

	//void asyncLoad();
	//void asyncOut();

	// Load file into _cache.data(), replaces old _cache.data()
	int load(size_t max_bytes) {
		_cache.resize(max_bytes);

  	if((_stream >> _cache) < 0)
    	return -1;

  	_data_p = 0;
  	return 0;
	}

	// Write to file
	int out() {
		return _stream << _cache;
	}

	// Replace buffer
	void replace(std::vector<unsigned char>&& buffer) {
		_cache = std::move(buffer);

  	_data_p = 0;
	}

	// Buffer pointers
	unsigned char next() {
		// Load new _cache if end of buffer is reached
  	if(end_of_buffer())
    	load(_cache.capacity());

  	return _cache[_data_p++];
	}

	// Append buffer
	void append(std::vector<unsigned char>&& buffer) {
		_cache.insert(_cache.end(), buffer.begin(), buffer.end());
	}

	void append(std::string &&buffer) {
		_cache.insert(_cache.end(), buffer.begin(), buffer.end());
	}

	void flush() { _stream.flush(); }

	std::vector<unsigned char> &getCache() { return _cache; }


	bool eof() { return _stream.eof(); }
	bool end_of_buffer() { return _data_p >= _cache.size(); }
	bool is_open() { return _stream.is_open(); }

	void seal() { _stream.seal(); }
};

typedef _File<LocalStream> ioFile;

#endif
