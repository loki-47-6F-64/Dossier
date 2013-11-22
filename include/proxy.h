#ifndef DOSSIER_PROXY_H
#define DOSSIER_PROXY_H

#include "file.h"

#include <vector>
#include <string>

#include <memory>

// Limited by database schema
#define MAX_NAME     45
#define MAX_CATEGORY 45
#define MAX_COMPANY  45
#define MAX_USERNAME 16

// Don't want to read client indefinetely
#define MAX_KEYWORD    55
#define MAX_PARAMETERS 10

enum _req_code {
	SEARCH,
	DOWNLOAD,
	UPLOAD,
	AUTHENTICATE
};

enum response {
	OK,
	INTERNAL_ERROR,
	CORRUPT_REQUEST
};

class requestBase {
protected:
	ioFile *_socket;
public:
	const char *err_msg = nullptr;

	// Load request from socket
	virtual int insert(ioFile *_socket) = 0;

	// Excecute request
	virtual int exec() = 0;

protected:
	int _customAppend(std::string &buf, int max);
  int _customAppend(int64_t &buf);
};

// Client upload
class requestUpload : public requestBase {
public:
	std::string company,
              username;

	int insert(ioFile *_socket);
	int exec();
};

// Client download
class requestDownload : public requestBase {
public:
  int64_t idPage;
	std::string company,
              username;

	int insert(ioFile *_socket);
	int exec();
};

class requestSearch : public requestBase {
public:
	std::string company,
              username;

	std::vector<std::string> keywords;

	int insert(ioFile *_socket);
	int exec();
};

class Proxy {
	std::unique_ptr<requestBase> _req;

	ioFile *_socket;
public:
	Proxy(ioFile *socket);
	//~Proxy();

	int readRequest();
  
};
#endif
