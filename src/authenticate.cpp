#include "authenticate.h"

std::mutex Authenticater::_mutex;
std::map<std::string, int64_t> Authenticater::_active;

Authenticater::Authenticater(Database *db) : _db(db) {}

std::string Authenticater::validate(std::string &username, std::string& hash) {
  std::string token;

  int64_t id = _db->validateUser(username, hash);
  if(_db->err_msg) {

    err_msg = _db->err_msg;
    return token;
  }

  token = username + hash;

  std::lock_guard<std::mutex> lg(_mutex);
  _active[token] = id;

  return token;
}

int64_t Authenticater::validate(std::string &token) {
  std::lock_guard<std::mutex> lg(_mutex);

  return _active[token];
}


void Authenticater::logOut(std::string &token) {
  std::lock_guard<std::mutex> lg(_mutex);

  _active.erase(token);
}
