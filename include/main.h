#ifndef MAIN_H
#define MAIN_H

#define _DEBUG_DOSSIER

#ifdef _DEBUG_DOSSIER
	#define ON_DEBUG( x ) x
	#define DEBUG_LOG( ... ) print(debug, __FILE__, ':', __LINE__,':', __VA_ARGS__)
#else
	#define ON_DEBUG( ... )
	#define DEBUG_LOG( x ) do {} while(0)
#endif

#include <string>

#include <arpa/inet.h>

#include "log.h"

constexpr int MAX_ERROR_BUFFER = 120;
thread_local extern char err_buf[MAX_ERROR_BUFFER];
namespace config {
	struct _server {
    std::string certPath, keyPath;

		uint16_t port;
		sa_family_t inet; 				// IPv4 or IPv6
		in_addr_t allowed_inaddr; // Allowed incoming IP addresses

		int poll_timeout;					// Time in milliseconds before timeout
	};

	struct _database {
		const char *host;
		const char *user;
		const char *password;
		const char *db;
	};

  struct _storage {
    const char *root;
  };

	extern _database database;
	extern _server server;
  extern _storage storage;
};
#endif
