#include <fcntl.h>
#include <unistd.h>
#include "main.h"
#include "proxy.h"
#include "database.h"

//#include "server.h"
//#include "sql_connect.h"
/*
typedef unsigned char byte;

namespace config {
	// Initialize with default values
	_server server { 8080, AF_INET, INADDR_ANY, 200 };
};

void start_server() {
	Server s;
	if(s.addListener(config::server.port, 20) < 0) {
		FATAL_ERROR("Can't set listener:", errno);
	}
	s();
}

*/


namespace config {
  // Initialize with default values
	_database database {
		"127.0.0.1",
		"root",
		"root",
		"mydb"
	};
};

void _test(std::string &&str) {
  //ioFile f(open("test.txt", O_RDWR | O_CREAT));
  ioFile f;
  f.append(std::move(str));
  f.out();

  std::unique_ptr<requestBase> req(new requestSearch());
  if(req->insert(&f)) {
    FATAL_ERROR(req->err_msg,0);
  }
  if(req->exec()) {
    FATAL_ERROR(req->err_msg,0);
  }
}

int main() {
	Log::open("out.log");

  _test(std::string("Loki\0ING\0Create", sizeof("Loki\0ING\0Create")));

	Log::close();
}
