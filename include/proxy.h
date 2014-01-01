#ifndef DOSSIER_PROXY_H
#define DOSSIER_PROXY_H

#include <vector>
#include <string>

#include <memory>

#include "file.h"
// Limited by database schema
constexpr int MAX_NAME     = 45;
constexpr int MAX_CATEGORY = 45;
constexpr int MAX_COMPANY  = 45;
constexpr int MAX_USERNAME = 16;

// Don't want to read client indefinetely
constexpr int MAX_KEYWORD    = 55;
constexpr int MAX_PARAMETERS = 10;

class Database;

enum _req_code {
  SEARCH,
  LIST_COMPANIES,
  DOWNLOAD,
  UPLOAD,
  NEW_COMPANY,
  REMOVE_COMPANY,
  REMOVE_DOCUMENT
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
  requestBase(sslFile *socket) : _socket(socket){}

  int64_t idUser;

	const char *err_msg = nullptr;

  /*
   * All values in the request are send in a null-terminated byte string
   * On failure a non-zero value is returned and err_msg is set.
   */
  int load() { return 0; }
  template<class... Args>
  int load(std::string &buf, int max, Args&&... params) {
    bool cont = true;
    int result = 0;

    while(cont) {
      if(_socket->eof()) {
        err_msg = "Reached EOF before \\0";
        return -1;
      }

      _socket->eachLoaded([&](unsigned char ch) {
        if(buf.size() > max) {
          result = -1;
          err_msg = "String to large";

          return false;
        }

        if(!ch) {
          return cont = false;
        }

        buf.push_back(ch);
        return true;
      });
    }

    if(result) {
      return -1;
    }
    return load(std::forward<Args>(params)...);
  }

  template<class... Args>
  int load(int64_t& buf, Args&&... params) {
    int max_digits = 10;

    std::string str;
    if(load(str, max_digits)) {
      return -1;
    }

    buf = std::atol(str.c_str());
    return load(std::forward<Args>(params)...);
  }

  template<class... Args>
  int load(int& buf, Args&&... params) {
    int max_digits = 10;

    std::string str;
    if(load(str, max_digits)) {
      return -1;
    }

    buf = std::atoi(str.c_str());
    return load(std::forward<Args>(params)...);
  }

  template<class... Args>
  int load(std::vector<std::string>& vs, int max_params, const int max_size, Args&&... params) {
    for(int x = 0; x < max_params; ++x) {
      std::string tmp;
      if(load(tmp, max_size)) {
        return -1;
      }

      if(tmp.empty()) {
        break;
      }

      vs.push_back(std::move(tmp));
    }

    return load(std::forward<Args>(params)...);
  }

	// Excecute request
	virtual int exec(Database& db) = 0;
};

class requestUpload : public requestBase {
public:
  requestUpload(sslFile *socket) : requestBase(socket){}

	std::string company;

  int64_t size;

	int exec(Database& db);
};

class requestDownload : public requestBase {
public:
  requestDownload(sslFile *socket) : requestBase(socket){}

  int64_t idPage;

	std::string company;

	int exec(Database& db);
};

class requestSearch : public requestBase {
public:
  requestSearch(sslFile *socket) : requestBase(socket){}
	std::string company;
  int year, month, day;

	std::vector<std::string> keywords;

	int exec(Database& db);
};

class requestNewCompany : public requestBase {
public:
  requestNewCompany(sslFile *socket) : requestBase(socket){}
  std::string name;

  int exec(Database& db);
};

class requestListCompanies : public requestBase {
public:
  requestListCompanies(sslFile *socket) : requestBase(socket){}
  int exec(Database& db);
};

class requestRemoveCompany : public requestBase {
public:
  requestRemoveCompany(sslFile *socket) : requestBase(socket){}
  std::string company;
  int exec(Database &db);
};

class requestRemoveDocument : public requestBase {
public:
  requestRemoveDocument(sslFile *socket) : requestBase(socket){}
  int64_t idPage;
  int exec(Database &db);
};
std::unique_ptr<requestBase> getRequest(sslFile *socket);
#endif
