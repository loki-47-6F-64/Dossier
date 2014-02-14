#ifndef DOSSIER_CLIENT_MAIN_H
#define DOSSIER_CLIENT_MAIN_H

#include "file/file.h"
namespace dossier {
namespace client {
extern ioFile fout;
extern ioFile fin;
extern ioFile ferr;

void print_help(const char *);
};
};
#endif
