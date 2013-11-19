#include "stream.h"
LocalStream::LocalStream() : _data_p(0), _eof(true) {}
void LocalStream::operator=(int fd) {}

int LocalStream::operator>>(std::vector<unsigned char>& buf) {
	if(_data_p >= _cache.size()) {
		_eof = true;
		buf.resize(0);
		return 0;
	}

	// Determine amount bytes awaiting read.
	ssize_t bytes_read = _cache.size() - _data_p > buf.size() ? buf.size() : _cache.size() - _data_p;

	for(int x = 0; x < bytes_read; x++) {
		buf[x] = _cache[_data_p++];
	}
  // Update number of bytes in buf
  buf.resize(bytes_read);

  return 0;
}

int LocalStream::operator<<(std::vector<unsigned char>&buf) {
	_eof = false;
  _cache.insert(_cache.end(), buf.begin(), buf.end());
	return 0;
}

int LocalStream::access(std::string& path) {
  return 1;
}

void LocalStream::seal() {}

void LocalStream::flush() {}

bool LocalStream::is_open() { return true; }
bool LocalStream::eof() { return _eof; }
