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
    ".",
    "./out.log"
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
  
    
    if(file.access(path, fileStreamRead)) {
      return -1;
    }
  
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
  
  void setConfig(std::string &&configName, std::string &dest) {
    auto key_pair = config_map.find(configName);
    if(key_pair != config_map.cend()) {
      dest = std::move((*key_pair).second);
    }
  }

  void setConfig(std::string &&configName, uint16_t &dest) {
    auto key_pair = config_map.find(configName);
    if(key_pair != config_map.cend()) {
      dest = std::stoi((*key_pair).second);
    }
  }

  void setConfig(std::string &&configName, sa_family_t &dest, bool is_inet_family) {
    auto key_pair = config_map.find(configName);
    if(key_pair != config_map.cend()) {
      if((*key_pair).second.back() == '4') {
        dest = AF_INET;
      }
      else {
        dest = AF_INET6;
      }
    }
  }

  int file(const char *path) {
    if(readConfig(path)) {
      return -1;
    }

    setConfig("certificate_path", config::server.certPath);
    setConfig("key_path", config::server.keyPath);
    setConfig("ip_family", config::server.inet, true);
    setConfig("port", config::server.port);
    setConfig("host", config::database.host);
    setConfig("password", config::database.password);
    setConfig("db", config::database.db);
    setConfig("user", config::database.user);
    setConfig("log", config::storage.log);
    setConfig("root_document", config::storage.root);
  
    return 0;
  }
};
