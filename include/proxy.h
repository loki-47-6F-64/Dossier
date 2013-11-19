#ifndef DOSSIER_PROXY_H
#define DOSSIER_PROXY_H

#include "file.h"

#include <vector>
#include <string>

#include <memory>

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
	// Execute request.
	//virtual void operator ()();

	// Load request from socket
	virtual int insert(ioFile *_socket) = 0;

	// Excecute request
	virtual int exec() = 0;

protected:
	int _customAppend(std::string &buf, int max);
};

// Client upload
class requestUpload : public requestBase {
public:
	std::string company,
							category,
							name;

	int insert(ioFile *_socket);
	int exec();
};

// Client download
class requestDownload : public requestBase {
public:
	std::string company,
              category,
              name;

	int insert(ioFile *_socket);
	int exec();
};

class requestSearch : public requestBase {
public:
	std::string company,
              category,
							name;

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
