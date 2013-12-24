#include <unistd.h>
#include <fcntl.h>

#include "stream.h"

_SslStream::_SslStream() : _eof(false), _ssl(nullptr) {}

void _SslStream::open(SSL *ssl) {
  _ssl = ssl;
}

void _SslStream::operator=(_SslStream&& stream) {
  this->_eof =  stream._eof;
  this->_ssl =  stream._ssl;

  stream._ssl = nullptr;
  stream._eof = true;
}

int _SslStream::operator>>(std::vector<unsigned char>& buf) {
  int bytes_read;

  if((bytes_read = SSL_read(_ssl, buf.data(), buf.size())) < 0) {
    return -1;
  }
  else if(!bytes_read) {
    _eof = true;
  }

  // Update number of bytes in buf
  buf.resize(bytes_read);

  return 0;
}

int _SslStream::operator<<(std::vector<unsigned char>&buf) {
  return SSL_write(_ssl, buf.data(), buf.size());
}

int _SslStream::access(std::string& url) {
  // Todo: implement access
  return 0;
}

void _SslStream::seal() {
  // fd is stored in _ssl
  int _fd = fd();

  SSL_free(_ssl);

  ::close(_fd);  
}

void _SslStream::flush() {}

int _SslStream::fd() { return SSL_get_fd(_ssl); }
bool _SslStream::is_open() { return _ssl != nullptr; }
bool _SslStream::eof() { return _eof; }
