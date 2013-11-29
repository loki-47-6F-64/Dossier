#include "main.h"
#include "server.h"

#include <unistd.h>
#include <thread>
#include <algorithm>
void Server::operator() () {
	if(isRunning())
		return;

	_continue = true;
	_listen();
}

int Server::addListener(uint16_t port, int max_parallel, std::function<void(ioFile&)> f) {
	sockaddr_in server {
		config::server.inet,
		htons(port),
		{ config::server.allowed_inaddr }
	};

	pollfd pfd { 
		socket(config::server.inet, SOCK_STREAM, 0),
		POLLIN,
		0 
	};

	if(pfd.fd == -1)
		return -1;

	// Allow reuse of local addresses
  setsockopt(pfd.fd, SOL_SOCKET, SO_REUSEADDR, &pfd.fd, sizeof(pfd.fd));

	if(bind(pfd.fd, (sockaddr*)&server, sizeof(sockaddr_in)) < 0) {
		return -1;
  }

	listen(pfd.fd, max_parallel);

	/* lock */ {
		std::lock_guard<std::mutex> lg(_add_listen);
		_listenfd.push_back(pfd);
    _action.push_back(f);
	} // unlock

	return pfd.fd;
}

void Server::removeListener(int fd) {
	/* lock */ {
		std::lock_guard<std::mutex> lg(_add_listen);
    for(int x = 0; x < _listenfd.size(); ++x) {
      if(_listenfd[x].fd == fd) {
        _listenfd.erase(_listenfd.begin() + x);
        _action  .erase(_action  .begin() + x);

        break;
      }
    }
  } // unlock

	close(fd);
}

void Server::stop() {
	_continue = false;
}	

Server::Server() : _continue(false) {}
Server::~Server() { 
	stop();
}

inline bool Server::isRunning() {
	return _continue;
}

void Server::_listen() {
	sockaddr_in client;
	int addr_size { sizeof(sockaddr_in) };

	int result;

	while(_continue) {
		std::unique_lock<std::mutex> ul(_add_listen);
    if((result = poll(_listenfd.data(), _listenfd.size(), config::server.poll_timeout)) > 0) 
		{
			for(int x = 0; x < _listenfd.size(); ++x) {
        auto& poll = _listenfd[x];

				if(poll.revents == POLLIN) {
					DEBUG_LOG("Accepting client");
	
      		// Accept new client
      		ioFile client_sock { 1024, accept(poll.fd, (sockaddr*)&client, (socklen_t*)&addr_size)};

          // Call user function
          _action[x](client_sock); 

          DEBUG_LOG("client closed...");
				}
			}
			
    }
    else if(result == -1) {
      FATAL_ERROR("Cannot poll socket", errno);
    }
  }

	// Cleanup
	for(auto& poll : _listenfd) {
		close(poll.fd);
	}
}
