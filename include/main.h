#ifndef MAIN_H
#define MAIN_H

#define _DEBUG_DOSSIER

#ifdef _DEBUG_DOSSIER
	#define ON_DEBUG( x ) x
	#define DEBUG_LOG( x ) Log::Debug(__FILE__, __LINE__, x)
#else
	#define ON_DEBUG( x )
	#define DEBUG_LOG( x ) do {} while(0)
#endif

#define FATAL_ERROR( x, y ) \
	Log::Error(x,y); \
	DEBUG_LOG("Exit due to fatal error");\
	exit(1);

#include <fstream>
#include <string>
#include <mutex>

#define LOG_BUFFER_SIZE 25
#define DATE_BUFFER_SIZE 21 + 1 // Full string plus '\0'

#include <arpa/inet.h>
namespace config {
	struct _server {
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

	extern _database database;
	extern _server server;
};

namespace Log {
	// Called one time only.
	void open(const char *path);
	void close();


	void Error(const char *msg, int err);
	void Warning(const char *msg, int err);
	void Info(const char *msg, int err);

	void Error(const char* msg);
	void Warning(const char *msg);
	void Info(const char *msg);

 	void Debug(const char *file_name, int line, const char *msg);
  void Debug(const char *file_name, int line, int64_t msg);
	
	void _scanDate();
};
#endif
