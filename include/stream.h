#ifndef DOSSIER_STREAM_H
#define DOSSIER_STREAM_H

#include <vector>
#include <string>

#include <openssl/ssl.h>
#include <openssl/err.h>
class FileStream {
	bool _eof;
	int _fd;

public:
	FileStream();

  void operator =(FileStream&& stream);
	void open(int fd);

	int operator >>(std::vector<unsigned char>& buf);
	int operator <<(std::vector<unsigned char>& buf);

	bool is_open();
	bool eof();

	int access(std::string &path);
	void seal();

	void flush();
  int fd();
};

class SslStream {
  bool _eof;

  SSL *_ssl;
public:
  SslStream();

  void operator =(SslStream&& stream);
  void open(int fd, SSL_CTX *ctx);

  int operator >>(std::vector<unsigned char>& buf);
  int operator <<(std::vector<unsigned char>& buf);

  bool is_open();
  bool eof();

  int access(std::string &path);
  void seal();

  void flush();
  int fd();
};
#endif
