#include <errno.h>
#include <string.h>

#include <openssl/err.h>
#include "err.h"

#ifdef __clang__
#if __clang_major__ *100 + __clang_minor__ *10 < 330
#define thread_local __thread
#endif
#endif

constexpr int MAX_ERROR_BUFFER = 120;
thread_local char err_buf[MAX_ERROR_BUFFER];

const char *sys_err() {
  return strerror_r(errno, err_buf, MAX_ERROR_BUFFER);
}

void set_err(const char *src) {
  int x;
  for(x = 0; src[x] && x < MAX_ERROR_BUFFER -1; ++x) {
    err_buf[x] = src[x];
  }
  err_buf[x] = '\0';
}

const char *ssl_err() {
  int err = ERR_get_error();
  if(err) {
    ERR_error_string_n(err, err_buf, MAX_ERROR_BUFFER);
    return err_buf;
  }

  return nullptr;
}

const char *get_current_err() {
  return err_buf;
}

const char *server_err(sslFile& server) {
  char *pos = err_buf;
  server.eachByte([&](unsigned char ch) {
    *pos++ = ch;
    return (pos - err_buf < MAX_ERROR_BUFFER - 1) ? 0 : 1;
  });
  *pos = '\0';

  return err_buf;
}
