#include <unistd.h>
#include <fcntl.h>

#include "stream.h"

FileStream::FileStream() : _fd(-1), _eof(false) {}
void FileStream::operator=(int fd) {
	_fd = fd;
}

int FileStream::operator>>(std::vector<unsigned char>& buf) {
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

int FileStream::operator<<(std::vector<unsigned char>&buf) {
	return write(_fd, buf.data(), buf.size());
}

int FileStream::access(std::string& path) {
	_fd = open(path.c_str(), O_CREAT | O_RDWR, S_IRWXU);
	_eof= false;

	return _fd;
}

void FileStream::seal() {
	close(_fd);
	_fd = -1;
}

void FileStream::flush() {
	fsync(_fd);
}

int FileStream::fd() { return _fd;};
bool FileStream::is_open() { return _fd != -1; }
bool FileStream::eof() { return _eof; }
