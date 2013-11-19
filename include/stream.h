#ifndef DOSSIER_STREAM_H
#define DOSSIER_STREAM_H

#include <vector>
#include <string>

class FileStream {
	bool _eof;
	int _fd;

public:
	FileStream();

	void operator =(int fd);

	int operator >>(std::vector<unsigned char>& buf);
	int operator <<(std::vector<unsigned char>& buf);

	bool is_open();
	bool eof();

	int access(std::string &path);
	void seal();

	void flush();
};

class LocalStream {
	std::vector<unsigned char> _cache;
	int _data_p;

	bool _eof;

public:
	LocalStream();

	/* unsupported */
	void operator =(int fd);

  int operator >>(std::vector<unsigned char>& buf);
  int operator <<(std::vector<unsigned char>& buf);
  
  bool is_open();
  bool eof();

	/* unsupported */
  int access(std::string &path);
  void seal();
  
	/* unsupported */
  void flush();
};

#endif
