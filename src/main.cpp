#include "main.h"

#include "file.h"
#include "proxy.h"

#include <unistd.h>

int main(int args, char *argv[]) {
  const char *conf_path = "./dossier.conf";
  if(args > 1) {
    conf_path = argv[1];
  }

  ioFile io_out(1, -1, dup(STDOUT_FILENO));
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

  Server server;
  start_server(server, config::server.port);
  return 0;
}
