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

	if(s.addListener(config::server.port, 20, [&](Client &&client) {
    Database db;

    if(db.err_msg) {
      print(*client.socket, 
        _response::INTERNAL_ERROR,
        "Could not connect to database.");
      print(error, db.err_msg);

      return;
    }

    std::string cn = getCN(client.ssl);
    int64_t idUser = db.validateUser(cn);

    if(db.err_msg) {
      print(*client.socket,
        _response::INTERNAL_ERROR,
        "User ", cn, " not found.");

      print(error, "User ", cn, " not found.");
      return;
    }

    std::unique_ptr<requestBase> req = getRequest(client.socket.get());

    if(req.get() == nullptr) {
      print(warning, "Unknown request.");
      return;
    }
//    req->insert(client.socket.get());

    req->idUser = idUser;
    req->exec(db);
  }
  ) < 0) {
    strerror_r(errno, err_buf, MAX_ERROR_BUFFER);
		print(error, "Can't set listener: ", err_buf);
	}

  if(s.addListener(8081, 20, [&](Client &&client) {
    DEBUG_LOG("Started Copy...");

    ioFile echo { 2, STDOUT_FILENO };

    echo.close_after_delete = false;

    client.socket->copy(echo);

    DEBUG_LOG("Finished copy");
  }
  ) < 0) {
    strerror_r(errno, err_buf, MAX_ERROR_BUFFER);
    print(error, "Can't set listener: ", err_buf);
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
  if(Server::init(config::server.certPath, config::server.keyPath)) {
    int err;

    // Empty error queue
    while((err = ERR_get_error())) {
      ERR_error_string_n(err, err_buf, MAX_ERROR_BUFFER);
      print(error, "Failed to init ssl:", err_buf);
    }
    return -1;
  }

  start_server();
  return 0;
}
