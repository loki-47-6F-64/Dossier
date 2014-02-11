#ifndef DOSSIER_ERR_H
#define DOSSIER_ERR_H

#include "file/file.h"
/*
 * Thread safe error functions
 */
const char *sys_err();
const char *ssl_err();
const char *server_err(sslFile& server);

void set_err(const char *err);
const char *get_current_err();

template<class File>
void ssl_print_err(File &out) {
  const char *err;
  while((err = ssl_err())) {
    out.append(err).append('\n');
  }

  if(!out.getCache().empty()) {
    out.out();
  }
}

#endif
