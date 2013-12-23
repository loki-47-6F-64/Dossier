#include <fcntl.h>
#include <unistd.h>

#include <thread>
#include "main.h"

#include "file.h"
#include "server.h"
#include "proxy.h"
#include "database.h"
typedef unsigned char byte;

thread_local char err_buf[MAX_ERROR_BUFFER];
#define ROOT_SERVER "/home/loki/keys/server/"
void start_server() {
	Server s;

	if(s.addListener(config::server.port, 20, [&](sslFile &&client) {
    std::unique_ptr<requestBase> req = getRequest(&client);

    if(req.get() == nullptr) {
      log(warning, "Unknown request.");
      return;
    }
    if(req->insert(&client) || 
       req->exec())
    {
      log(warning, req->err_msg);
    }
  }
  ) < 0) {
    strerror_r(errno, err_buf, MAX_ERROR_BUFFER);
		log(error, "Can't set listener: ", err_buf);
	}

  if(s.addListener(8081, 20, [&](sslFile &&client) {
    DEBUG_LOG("Started Copy...");

    ioFile echo { 2, STDOUT_FILENO };

    echo.close_after_delete = false;

    client.copy<ioFile>(echo);

    DEBUG_LOG("Finished copy");
  }
  ) < 0) {
    strerror_r(errno, err_buf, MAX_ERROR_BUFFER);
    log(error, "Can't set listener: ", err_buf);
  }

	s();
}

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
    8080, AF_INET, INADDR_ANY, 200
  };
};

int main() {
  log_open("out.log");
  if(!Server::init(config::server.certPath, config::server.keyPath)) {
    start_server();
  }
  return 0;
}
