#ifndef DOSSIER_AUTHENTICATOR_H
#define DOSSIER_AUTHENTICATOR_H

#include <map>
#include <mutex>

#include "database.h"
class Authenticater {
	static std::map<std::string, int64_t> _active;
  static std::mutex _mutex;

  Database *_db;
public:
  Authenticater(Database *db);
  const char *err_msg = nullptr;

  int64_t validate(std::string &token);

  // Validate user and create token for quick subsequent authentication
  std::string validate(std::string &username, std::string &hash);


  void logOut(std::string &token);
};

#endif
