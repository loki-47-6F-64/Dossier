#include <vector>
#include "file/file.h"
#include "thread_t.h"
using namespace dossier::client;
int print_hello(const char *world) {
  print(fout, "hello ", world);
  return 0;
}

int main(int argc, char *argv[]) {
  std::vector<thread_t> _thread(5);

  for(auto &t : _thread) {
    t = thread_t(&print_hello, "world!\n");
  }

  for(auto &t : _thread) {
    t.join();
  }
  return 0;
}
