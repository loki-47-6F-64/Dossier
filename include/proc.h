#ifndef DOSSIER_PROC_H
#define DOSSIER_PROC_H

#include <unistd.h>
#include <thread>
#include <future>

#include "file.h"
#include "main.h"

enum class pipeType {
  READ,
  WRITE
};

struct Proc {
  pid_t pid;

  ioFile fpipe { -1,-1 };
};

/*
  Argv must end with a nullptr
  fd must be an array of size 4
  On success: Proc::pid < 0
*/
Proc proc_open(const char *path, const char *argv[], pipeType);
#endif
