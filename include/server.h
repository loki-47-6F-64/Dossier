#ifndef DOSSIER_SERVER_H
#define DOSSIER_SERVER_H

#include <poll.h>
#include <vector>
#include <mutex>

class Server {
	// Should the server continue?
	bool _continue;

	std::vector<pollfd> _listenfd;
	std::mutex _add_listen;
public:
	Server();
	~Server();
	void operator() ();

	// Returns -1 on failure
	int addListener(uint16_t port, int max_parallel);
	void removeListener(int fd);

	void stop();

	inline bool isRunning();
private:
	void _listen();	
};

#endif
