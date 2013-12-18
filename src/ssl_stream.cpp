#include <unistd.h>
#include <fcntl.h>

#include "stream.h"

SslStream::SslStream() : _eof(false), _ssl(nullptr) {}

void SslStream::open(int fd, SSL_CTX *ssl_ctx) {
  _ssl = SSL_new(ssl_ctx);

  SSL_accept(_ssl);
  SSL_set_fd(_ssl, fd);
}

void SslStream::operator=(SslStream&& stream) {
  this->_eof =  stream._eof;
  this->_ssl =  stream._ssl;

  stream._ssl = nullptr;
  stream._eof = true;
}

int SslStream::operator>>(std::vector<unsigned char>& buf) {
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

int SslStream::operator<<(std::vector<unsigned char>&buf) {
  return SSL_write(_ssl, buf.data(), buf.size());
}

int SslStream::access(std::string& url) {
  // Todo: implement access
  return 0;
}

void SslStream::seal() {
  // fd is stored in _ssl
  int _fd = fd();

  SSL_free(_ssl);

  ::close(_fd);  
}

void SslStream::flush() {}

int SslStream::fd() { return SSL_get_fd(_ssl); }
bool SslStream::is_open() { return _ssl != nullptr; }
bool SslStream::eof() { return _eof; }
