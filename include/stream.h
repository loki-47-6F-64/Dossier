#ifndef DOSSIER_STREAM_H
#define DOSSIER_STREAM_H

#include <vector>
#include <string>

class FileStream {
	bool _eof;
	int _fd;

public:
	FileStream();

  void operator =(FileStream&& stream);
	void operator =(int fd);

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
