#include "server/main.h"
#include "server/server.h"

#include <unistd.h>
#include "thread_t.h"

namespace dossier {
namespace server {
Context Server::_ssl_ctx;
void Server::operator() () {
  if(isRunning())
    return;

  _continue = true;
  _listen();
}

int Server::addListener(uint16_t port, int max_parallel, std::function<void(Client&&)> f) {
  sockaddr_in server {
    config::server.inet,
    htons(port),
    { INADDR_ANY }
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

  _listenfd.push_back(pfd);
  _action.push_back(f);

  return pfd.fd;
}

void Server::removeListener(int fd) {
  for(int x = 0; x < _listenfd.size(); ++x) {
    if(_listenfd[x].fd == fd) {
      _listenfd.erase(_listenfd.begin() + x);
      _action  .erase(_action  .begin() + x);

      break;
    }
  }

  close(fd);
}

void Server::stop() {
  _continue = false;
  _RAII_lock lg(_server_stop_lock);
} 

Server::Server() : _continue(false), _task(5) {}
Server::~Server() { 
  stop();
}

inline bool Server::isRunning() {
  return _continue;
}

void Server::_listen() {
  _RAII_lock lg(_server_stop_lock);

  sockaddr_in client; //TODO: Stack overflow on raspberry pi?
  int addr_size { sizeof(sockaddr_in) };

  int result;
  while(_continue) {
    if((result = poll(_listenfd.data(), _listenfd.size(), config::server.poll_timeout)) > 0) 
    {
      for(int x = 0; x < _listenfd.size(); ++x) {
        auto& poll = _listenfd[x];

        if(poll.revents == POLLIN) {
          DEBUG_LOG("Accepting client");
  
          Client client = ssl_accept(Server::_ssl_ctx.get(), poll.fd, (sockaddr*)&client, (socklen_t*)&addr_size);
          if(client.socket.get() == nullptr) {
            continue;
          }

          MoveByCopy<Client> me(std::move(client));
          _task.push(
            decltype(_task)::task_type(std::bind(_action[x], me))
          );
        }
      }
      
    }
    else if(result == -1) {
      print(error, "Cannot poll socket: ", sys_err());

      exit(EXIT_FAILURE);
    }
  }

  // Cleanup
  for(auto& poll : _listenfd) {
    close(poll.fd);
  }
}
};
};
