#include <thread>
#include <unordered_map>

#include "main.h"

#include "file.h"
#include "server.h"
#include "proxy.h"
#include "database.h"

typedef unsigned char byte;

thread_local char err_buf[MAX_ERROR_BUFFER];
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
		print(error, "Can't set listener: ", strerror_r(errno, err_buf, MAX_ERROR_BUFFER));

    return;
	}

	s();
}

int main() {
  ioFile io_out(1, dup(STDOUT_FILENO));

  if(config::file("./dossier.conf")) {
    io_out.append(config::err_msg).append("\n").out();
    return -1;
  }

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
