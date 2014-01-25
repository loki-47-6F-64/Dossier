#include <unistd.h>
#include <fcntl.h>

#include "stream.h"

int fileStreamRead(_FileStream& fs, const char *file_path) {
  int _fd = ::open(file_path, O_RDONLY, 0);
  
  fs.open(_fd);
  return fs.is_open() == false;
}

int fileStreamWrite(_FileStream& fs, const char *file_path) {
  int _fd = ::open(file_path,
               O_CREAT | O_APPEND | O_WRONLY,
               S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
  );
  
  fs.open(_fd);
  return fs.is_open() == false;
}

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

void _FileStream::seal() {
	close(_fd);
	_fd = -1;
}

int _FileStream::fd() { return _fd;};
bool _FileStream::is_open() { return _fd != -1; }
bool _FileStream::eof() { return _eof; }
