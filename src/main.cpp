#include <fcntl.h>
#include <unistd.h>

#include <thread>
#include "main.h"

#include "server.h"
#include "database.h"

typedef unsigned char byte;

void start_server() {
	Server s;
	if(s.addListener(config::server.port, 20, [&](int fd) {
    ioFile f(1, fd);
    ioFile echo (1, STDOUT_FILENO);

    echo.close_after_delete = false;

    f.load(1024);
    copy<ioFile, ioFile>(f, echo);
  }) < 0) {
		FATAL_ERROR("Can't set listener:", errno);
	}

  if(s.addListener(8081, 20, [&](int fd) {
    ioFile f(1, fd);
    ioFile echo (1, STDOUT_FILENO);

    echo.close_after_delete = false;

    f.load(2);
    f.out();
  }) < 0) {
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

  _server server { 8080, AF_INET, INADDR_ANY, 200 };
};

int main() {
	Log::open("out.log");

  start_server();

	Log::close();

  return 0;
}
