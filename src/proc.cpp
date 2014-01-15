#define INT(x) static_cast<int>(x)
#include "proc.h"
Proc proc_open(const char *path, const char *argv[], pipeType type) {
  Proc proc;

  // 0 == read, 1 == write
  int fd[2];
  if(pipe(&fd[0])) {
    proc.pid = -1;
    return proc;
  }

  if((proc.pid = fork()) > 0) {
    proc.fpipe = ioFile(1024, -1, fd[INT(type)]);

    close(fd[1 - INT(type)]);
  }
  else if(!proc.pid) {
    dup2(fd[1 - INT(type)], 1 - INT(type));

    close(fd[INT(type)]);

    execvp(path, (char* const*)argv);

    close(fd[1 - INT(type)]);

    exit(-1);
  }

  return proc;
}
