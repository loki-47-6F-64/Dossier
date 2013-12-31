#include <unordered_map>
#include "config.h"
#include "file.h"

#define ROOT_SERVER "/home/loki/keys/server/"
namespace config {
  // Initialize with default values
  _database database {
    "127.0.0.1",
    "root",
    "root",
    "mydb"
  };

  _storage storage {
    "."
  };

  _server server {
    std::string(ROOT_SERVER "root.crt"),
    std::string(ROOT_SERVER "ssl_server.key"),
    8080, AF_INET, 200
  };


  std::unordered_map<std::string, std::string> config_map(24);
  const char *err_msg;
  void skip_comment(ioFile &f) {
    char ch;
    while((ch = f.next())) {
      if(ch == '\n') {
        return;
      }
    }
  }
  
  bool is_whitespace(unsigned char ch) {
    return ch == ' ' || ch == '\t' || ch == '\n';
  }
  
  int readValue(ioFile &file, std::string &value) {
    unsigned char ch;
    while((ch = file.next())) {
      if(ch == '\n' || ch == '#') {
        if(value.empty()) {
          err_msg = "Missing value.";
          return -1;
        }
  
        if(ch == '#')
          skip_comment(file);
  
        return 0;
      }
  
      if(is_whitespace(ch))
        continue;
  
      value.push_back(ch);
    }
  
    err_msg = "Reached EOF before new line.";
    return -1;
  }
  
  int readConfig(const char *path) {
    ioFile file(1024);
  
    file.access(path);
  
    std::string name;
  
    unsigned char ch;
    while((ch = file.next())) {
      if(is_whitespace(ch))
        continue;
  
      if(ch == '#') {
        skip_comment(file);
      }
      else if(ch == '=') {
        std::string value;
        if(readValue(file, value)) {
          return -1;
        }
  
        config_map[name] = std::move(value);
        name.clear();
      }
      else {
        name.push_back(ch);
      }
    }
  
    err_msg = "Reached EOF before equal sign.";
    return !name.empty();
  }
  
  in_addr_t stoaddr(const char *str) {
    in_addr_t result;
  
    char *pos = (char*)str;
  
    for(int x = 0; x < 4; ++x) {
      result |= ((in_addr_t)std::strtol(str, &pos, 10)) << x;
    }
  
    return result;
  }
  
  int file(const char *path) {
    if(readConfig(path)) {
      return -1;
    }

    std::string name = "certificate_path";
    auto key_pair = config_map.find(name);
  
    if(key_pair != config_map.cend()) {
      config::server.certPath = (*key_pair).second;
    }
  
    name = "key_path";
    key_pair = config_map.find(name);
    if(key_pair != config_map.cend()) {
      config::server.keyPath = (*key_pair).second;
    }
  
    name = "ip_family";
    key_pair = config_map.find(name);
    if(key_pair != config_map.cend()) {
      if((*key_pair).second.back() == '4') {
        config::server.inet = AF_INET;
      }
      else {
        config::server.inet = AF_INET6;
      }
    }
  
    name = "port";
    key_pair = config_map.find(name);
    if(key_pair != config_map.cend()) {
      config::server.port = std::stoi((*key_pair).second);
    }

    name = "host";
    key_pair = config_map.find(name);
    if(key_pair != config_map.cend()) {
      config::database.host = (*key_pair).second;
    }
  
    name = "user";
    key_pair = config_map.find(name);
    if(key_pair != config_map.cend()) {
      config::database.user = (*key_pair).second;
    }
  
    name = "password";
    key_pair = config_map.find(name);
    if(key_pair != config_map.cend()) {
      config::database.password = (*key_pair).second;
    }
  
    name = "db";
    key_pair = config_map.find(name);
    if(key_pair != config_map.cend()) {
      config::database.db = (*key_pair).second;
    }
  
    name = "root_document";
    key_pair = config_map.find(name);
    if(key_pair != config_map.cend()) {
      config::storage.root = (*key_pair).second;
    }
  
    return 0;
  }
};
