#include <fcntl.h>
#include <unistd.h>
#include "main.h"
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

int main() {
	Log::open("out.log");

	Database database;
	if(database.err_msg ||
			database.validateUser("Loki", "Hash")) {
		FATAL_ERROR(database.err_msg, 0);
	}
	else
		Log::Info("Success");

	Log::close();
}
