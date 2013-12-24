#ifndef DOSSIER_PROXY_H
#define DOSSIER_PROXY_H

#include <vector>
#include <string>

#include <memory>

#include "file.h"
// Limited by database schema
#define MAX_NAME     45
#define MAX_CATEGORY 45
#define MAX_COMPANY  45
#define MAX_USERNAME 16

// Don't want to read client indefinetely
#define MAX_KEYWORD    55
#define MAX_PARAMETERS 10
#define MAX_TOKEN      255
#define MAX_PASSWORD   255

class Database;

enum _req_code {
  SEARCH,
  DOWNLOAD,
  UPLOAD,
  NEW_COMPANY
};

enum _response {
  OK,
  INTERNAL_ERROR,
  CORRUPT_REQUEST
};

class requestBase {
protected:
	sslFile *_socket;
public:
  int64_t idUser;

	const char *err_msg = nullptr;

	// Load request from socket
	virtual int insert(sslFile *_socket) = 0;

	// Excecute request
	virtual int exec(Database& db) = 0;

protected:
	int _customAppend(std::string &buf, int max);
  int _customAppend(int64_t &buf);
};

// Client upload
class requestUpload : public requestBase {
public:
	std::string company;

  int64_t size;

	int insert(sslFile *_socket);
	int exec(Database& db);
};

// Client download
class requestDownload : public requestBase {
public:
  int64_t idPage;

	std::string company;

	int insert(sslFile *_socket);
	int exec(Database& db);
};

class requestSearch : public requestBase {
public:
	std::string company;

	std::vector<std::string> keywords;

	int insert(sslFile *_socket);
	int exec(Database& db);
};

class requestNewCompany : public requestBase {
public:
  std::string name;

  int insert(sslFile *_socket);
  int exec(Database& db);
};

std::unique_ptr<requestBase> getRequest(sslFile *socket);
#endif
