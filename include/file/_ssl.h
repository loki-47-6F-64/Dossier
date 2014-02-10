#ifndef DOSSIER_SSL_H
#define DOSSIER_SSL_H

#include <string>
#include <memory>

#include <openssl/ssl.h>

#include "file.h"

struct Client {
  SSL *ssl;

  std::unique_ptr<sslFile> socket;
};

// Deallocation functor for std::unique_ptr
class X509_Free {
public:
  void operator()(X509 *cert) {
    X509_free(cert);
  }
};

// Deallocation functor for std::unique_ptr
class SSL_CTX_Free {
public:
  void operator()(SSL_CTX *ssl_ctx) {
    SSL_CTX_free(ssl_ctx);
  }
};

typedef std::unique_ptr<X509, X509_Free>       Certificate;
typedef std::unique_ptr<SSL_CTX, SSL_CTX_Free> Context;

// On failure Context.get() returns nullptr
Context init_ctx_server(std::string &caPath, std::string& certPath, std::string& keyPath);
Context init_ctx_client(std::string &caPath, std::string& certPath, std::string& keyPath);

void init_ssl();
// On failure Client.get() returns nullptr
class sockaddr;
Client ssl_accept(SSL_CTX *ctx, int fd, sockaddr *client, uint32_t* addr_size);
sslFile ssl_connect(Context &ctx, const char *hostname, const char* port);

std::string getCN(const SSL *ssl);

#endif
