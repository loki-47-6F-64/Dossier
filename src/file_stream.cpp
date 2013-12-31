#include <unistd.h>
#include <fcntl.h>

#include "stream.h"

_FileStream::_FileStream() : _fd(-1), _eof(false) {}
void _FileStream::open(int fd) {
	_fd = fd;
}

void _FileStream::operator=(_FileStream&& stream) {
  this->_fd  =  stream._fd;
  this->_eof =  stream._eof;

  stream._fd = -1;
  stream._eof= true;
}

int _FileStream::operator>>(std::vector<unsigned char>& buf) {
	ssize_t bytes_read;

	if((bytes_read = read(_fd, buf.data(), buf.size())) < 0) {
    return -1;
  }
  else if(!bytes_read) {
    _eof = true;
  }

	// Update number of bytes in buf
  buf.resize(bytes_read);
	return 0;
}

int _FileStream::operator<<(std::vector<unsigned char>&buf) {
	return write(_fd, buf.data(), buf.size());
}

int _FileStream::access(std::string& path) {
	_fd = ::open(path.c_str(),
               O_CREAT | O_APPEND | O_RDWR,
               S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
  );
	_eof= false;

	return _fd;
}

void _FileStream::seal() {
	close(_fd);
	_fd = -1;
}

int _FileStream::fd() { return _fd;};
bool _FileStream::is_open() { return _fd != -1; }
bool _FileStream::eof() { return _eof; }
