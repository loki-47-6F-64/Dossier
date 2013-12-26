#ifndef DOSSIER_SERVER_H
#define DOSSIER_SERVER_H

#include <poll.h>
#include <vector>
#include <mutex>

#include "_ssl.h"
#include "file.h"
class Server {
	// Should the server continue?
	bool _continue;

	std::vector<pollfd> _listenfd;
  std::vector<std::function<void(Client&&)>> _action;

	std::mutex _add_listen;
public:
	Server();
	~Server();

  // Start server
	void operator() ();

	// Returns -1 on failure
	int addListener(uint16_t port, int max_parallel, std::function<void(Client&&)> f);
	void removeListener(int fd);

	void stop();

	inline bool isRunning();

private:
	void _listen();	


private:
  static Context _ssl_ctx;
public:
  // Needs to be called before starting server
  static int init(std::string& certPath, std::string& keyPath) {
    _ssl_ctx = init_ssl(certPath, keyPath);
    return _ssl_ctx.get() == nullptr;
  }
};

#endif
