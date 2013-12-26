#ifndef DOSSIER_SSL_H
#define DOSSIER_SSL_H

#include <string>
#include <memory>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include "file.h"

struct Client {
  SSL *ssl;

  std::unique_ptr<sslFile> socket;
};

class X509_Free {
public:
  void operator()(X509 *cert) {
    X509_free(cert);
  }
};

class SSL_CTX_Free {
public:
  void operator()(SSL_CTX *ssl_ctx) {
    SSL_CTX_free(ssl_ctx);
  }
};

typedef std::unique_ptr<X509, X509_Free>       Certificate;
typedef std::unique_ptr<SSL_CTX, SSL_CTX_Free> Context;

Context init_ssl(std::string& certPath, std::string& keyPath);

class sockaddr;
Client ssl_accept(SSL_CTX *ctx, int fd, sockaddr *client, uint32_t* addr_size);

std::string getCN(const SSL *ssl);

#endif
