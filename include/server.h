#ifndef DOSSIER_SERVER_H
#define DOSSIER_SERVER_H

#include <poll.h>
#include <vector>
#include <mutex>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include "file.h"
class Server {
	// Should the server continue?
	bool _continue;

	std::vector<pollfd> _listenfd;
  std::vector<std::function<void(sslFile&&)>> _action;

	std::mutex _add_listen;
public:
	Server();
	~Server();

  // Start server
	void operator() ();

	// Returns -1 on failure
	int addListener(uint16_t port, int max_parallel, std::function<void(sslFile&&)> f);
	void removeListener(int fd);

	void stop();

	inline bool isRunning();

private:
	void _listen();	


private:
  static SSL_CTX *_ssl_ctx;
public:
  // Needs to be called before starting server
  static int init(std::string& certPath, std::string& keyPath) {
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    _ssl_ctx = SSL_CTX_new(SSLv3_server_method());

    return _loadCertificates(certPath.c_str(), keyPath.c_str());
  }
private:

  static int  _loadCertificates(const char *certPath, const char *keyPath) {

    if(SSL_CTX_use_certificate_file(_ssl_ctx, certPath, SSL_FILETYPE_PEM) <= 0) {
      return -1;
    }

    if(SSL_CTX_use_PrivateKey_file(_ssl_ctx, keyPath, SSL_FILETYPE_PEM) <= 0) {
      return -1;
    }

    if(!SSL_CTX_check_private_key(_ssl_ctx)) {
      return -1;
    }

    if(!SSL_CTX_load_verify_locations(_ssl_ctx, certPath, NULL)) {
      return -1;
    }

    SSL_CTX_set_verify(_ssl_ctx, SSL_VERIFY_PEER, NULL);
    SSL_CTX_set_verify_depth(_ssl_ctx, 1);

    return 0;
  }
};

#endif
