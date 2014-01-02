#include <errno.h>
#include <string.h>

#include <openssl/err.h>
#include "err.h"

constexpr int MAX_ERROR_BUFFER = 120;
thread_local char err_buf[MAX_ERROR_BUFFER];

const char *sys_err() {
  return strerror_r(errno, err_buf, MAX_ERROR_BUFFER);
}

const char *ssl_err() {
  int err = ERR_get_error();
  if(err) {
    ERR_error_string_n(err, err_buf, MAX_ERROR_BUFFER);
    return err_buf;
  }

  return nullptr;
}
