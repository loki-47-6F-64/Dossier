#include <mutex>

#include <unistd.h>
#include <sys/socket.h>

#include <netdb.h>

#include "err.h"
#include "file/_ssl.h"

std::unique_ptr<std::mutex[]> lock;
void crypto_lock(int mode, int n, const char *file, int line) {
  if(mode & CRYPTO_LOCK) {
    lock[n].lock();
  } else {
    lock[n].unlock();
  }
}

int loadCertificates(Context& ctx, const char *caPath, const char *certPath, const char *keyPath) {
  if(SSL_CTX_use_certificate_file(ctx.get(), certPath, SSL_FILETYPE_PEM) != 1) {
    return -1;
  }

  if(SSL_CTX_use_PrivateKey_file(ctx.get(), keyPath, SSL_FILETYPE_PEM) != 1) {
    return -1;
  }

  if(SSL_CTX_check_private_key(ctx.get()) != 1) {
    return -1;
  }

  if(SSL_CTX_load_verify_locations(ctx.get(), caPath, NULL) != 1) {
    return -1;
  }

  SSL_CTX_set_verify(ctx.get(), SSL_VERIFY_PEER, NULL);
  SSL_CTX_set_verify_depth(ctx.get(), 1);

  return 0;
}

std::string getCN(const SSL *ssl) {
  std::string cn;

  Certificate cert(SSL_get_peer_certificate(ssl));
  if(!cert.get()) {
    return cn;
  }

  char *pos, *ch = cert->name;

  while(*ch) {
    if(*ch == '/')
      pos = ch;
    ++ch;
  }

  // TODO: check if it is legal to use '/' in CN
  cn = ch - pos > 4 ? pos+4 : "";

  return cn;
}

Context init_ctx_server(std::string &caPath, std::string& certPath, std::string& keyPath) {
  Context ctx(SSL_CTX_new(SSLv3_server_method()));

  if(loadCertificates(ctx, caPath.c_str(), certPath.c_str(), keyPath.c_str())) {
    ctx.release();
  }

  return ctx;
}

Context init_ctx_client(std::string &caPath, std::string& certPath, std::string& keyPath) {
  Context ctx(SSL_CTX_new(SSLv3_client_method()));

  if(loadCertificates(ctx, caPath.c_str(), certPath.c_str(), keyPath.c_str())) {
    ctx.release();
  }

  return ctx;
}

void init_ssl() {
  SSL_library_init();
  OpenSSL_add_all_algorithms();
  SSL_load_error_strings();

  lock = std::unique_ptr<std::mutex[]>(new std::mutex[CRYPTO_num_locks()]);
  CRYPTO_set_locking_callback(crypto_lock);
}
sslFile ssl_connect(Context &ctx, const char *hostname, const char* port) {
  constexpr long buffer_size = 1024, timeout = -1;

  sslFile sslFd(buffer_size);
  int serverFd = socket(AF_INET, SOCK_STREAM, 0);

  addrinfo hints;
  addrinfo *server;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  int err;
  if((err = getaddrinfo(hostname, port, &hints, &server))) {
    set_err(gai_strerror(err));
    return sslFd;
  }


  if(connect(serverFd, server->ai_addr, server->ai_addrlen)) {
    freeaddrinfo(server);

    set_err(sys_err());
    return sslFd;
  }

  freeaddrinfo(server);
  SSL *ssl = SSL_new(ctx.get());

  if(ssl == nullptr) {
    set_err((std::string("SSL struct is NULL: ") + ssl_err()).c_str());
  }

  sslFile ssl_tmp(buffer_size, timeout, ssl);
  SSL_set_fd(ssl, serverFd);

  if(SSL_connect(ssl) != 1) {
    ssl_err();
    return sslFd;
  }

  sslFd = std::move(ssl_tmp);

  return sslFd;
}

Client ssl_accept(SSL_CTX *ctx, int fd, sockaddr *client_addr, uint32_t* addr_size) {
  constexpr long buffer_size = 1024, timeout = 3000000;
  int clientFd = accept(fd, client_addr, addr_size);

  SSL *ssl = SSL_new(ctx);
  SSL_set_fd(ssl, clientFd);

  Client client { ssl, std::unique_ptr<sslFile>(new sslFile(buffer_size, timeout, ssl)) };
  if(SSL_accept(ssl) != 1) {
    client.ssl = nullptr;
    return client;
  }

  return client;
}
