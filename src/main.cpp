#include <fcntl.h>
#include <unistd.h>

#include <thread>
#include "main.h"

#include "file.h"
#include "server.h"
#include "proxy.h"
#include "database.h"
typedef unsigned char byte;

#define ROOT_SERVER "/home/loki/keys/server/"
void start_server() {
	Server s;

	if(s.addListener(config::server.port, 20, [&](sslFile &&client) {
    std::unique_ptr<requestBase> req = getRequest(&client);

    if(req.get() == nullptr) {
      Log::Warning("Unknown request.");
      return;
    }
    if(req->insert(&client) || 
       req->exec())
    {
      Log::Warning(req->err_msg);
    }
  }
  ) < 0) {
		FATAL_ERROR("Can't set listener:", errno);
	}

  if(s.addListener(8081, 20, [&](sslFile &&client) {
    DEBUG_LOG("Started Copy...");

    ioFile echo { 2, STDOUT_FILENO };

    echo.close_after_delete = false;

    client.copy<ioFile>(echo);

    DEBUG_LOG("Finished copy");
  }
  ) < 0) {
    FATAL_ERROR("Can't set listener:", errno);
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
	Log::open("out.log");

  Server::init(config::server.certPath, config::server.keyPath);
  start_server();

	Log::close();

  return 0;
}
