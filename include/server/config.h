#ifndef DOSSIER_CONFIG_H
#define DOSSIER_CONFIG_H

#include <string>
#include <arpa/inet.h>
namespace dossier {
namespace config {

  struct _server {
    std::string certPath, keyPath;

    uint16_t port;
    sa_family_t inet;         // IPv4 or IPv6

    int poll_timeout;         // Time in milliseconds before timeout
  };

  struct _database {
    std::string host;
    std::string user;
    std::string password;
    std::string db;
  };

  struct _storage {
    std::string root;
    std::string log;
  };

  extern _database database;
  extern _server server;
  extern _storage storage;


  /*
   * Read config file at path.
   * Returns 0 on success
   * Returns non-zero on failiure and sets err_msg
   */
  int file(const char *path);

  extern const char *err_msg;
};
};

#endif
