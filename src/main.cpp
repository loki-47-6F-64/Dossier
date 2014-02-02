#include <thread>
#include <unordered_map>

#include "main.h"

#include "file.h"
#include "server.h"
#include "proxy.h"
#include "database.h"
#include "proc.h"

typedef unsigned char byte;

#define CHAR(x) static_cast<char>(x)
void start_server() {
	Server s;

	if(s.addListener(config::server.port, 20, [&](Client &&client) {
    Database db;

    if(db.err_msg) {
      print(*client.socket, 
        CHAR(_response::INTERNAL_ERROR),
        "Could not connect to database.");
      print(error, db.err_msg);

      return;
    }

    std::string cn = getCN(client.ssl);
    int64_t idUser = db.validateUser(cn);

    DEBUG_LOG("Accepted client: ", cn);
    if(db.err_msg) {
      print(*client.socket,
        CHAR(_response::INTERNAL_ERROR),
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
		print(error, "Can't set listener: ", sys_err());

    return;
	}

  if(s.addListener(8081, 20, [&](Client &&client) {
    ioFile out(1024, -1, dup(STDOUT_FILENO));

    client.socket->copy(out);
  }) <= 0) {
    print(error, "Can't set listener: ", sys_err());

    return;
  }
	s();
}

int main(int args, char *argv[]) {
  const char *conf_path = "./dossier.conf";
  if(args > 1) {
    conf_path = argv[1];
  }

  ioFile io_out(1, dup(STDOUT_FILENO));

  if(config::file(conf_path)) {
    io_out.append(config::err_msg).append("\n").out();
    return -1;
  }

  log_open(config::storage.log.c_str());
  if(Server::init(config::server.certPath, config::server.keyPath)) {
    const char* err;

    // Empty error queue
    while((err = ssl_err())) {
      print(error, "Failed to init ssl:", err);
    }
    return -1;
  }

  start_server();

  return 0;
}
