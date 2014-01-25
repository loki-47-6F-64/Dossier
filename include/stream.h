#ifndef DOSSIER_STREAM_H
#define DOSSIER_STREAM_H

#include <vector>
#include <string>

#include <openssl/ssl.h>

class _FileStream {
	bool _eof;
	int _fd;

public:
	_FileStream();

  void operator =(_FileStream&& stream);
	void open(int fd);

	int operator >>(std::vector<unsigned char>& buf);
	int operator <<(std::vector<unsigned char>& buf);

	bool is_open();
	bool eof();

	void seal();

  int fd();
};

int fileStreamRead(_FileStream& fs, const char *file_path);
int fileStreamWrite(_FileStream& fs, const char *file_path);

class _SslStream {
  bool _eof;

  SSL *_ssl;
public:
  _SslStream();

  void operator =(_SslStream&& stream);
  void open(SSL *ssl);

  int operator >>(std::vector<unsigned char>& buf);
  int operator <<(std::vector<unsigned char>& buf);

  bool is_open();
  bool eof();

  void seal();

  int fd();
};

#define SslStream  _SslStream,  SSL*
#define FileStream _FileStream, int
#endif
