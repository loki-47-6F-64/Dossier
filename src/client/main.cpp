#include <string>
#include <cstring>
#include <unordered_map>
#include <functional>

#include "client/args.h"
#include "file/_ssl.h"
#include "err.h"

#define ROOT_SERVER "/home/loki/keys/server/"
#define ROOT_CLIENT "/home/loki/keys/client/"

constexpr int buffer_size = 1024;

#define STDOUT_FILENO 1
#define STDIN_FILENO  0
#define STDERR_FILENO 2

ioFile fout(buffer_size, -1, STDOUT_FILENO);
ioFile fin (buffer_size, -1, STDIN_FILENO);
ioFile ferr(buffer_size, -1, STDERR_FILENO);

std::string
    caPath(ROOT_SERVER "root.crt"),
    certPath(ROOT_CLIENT "ssl_client.crt"),
    keyPath(ROOT_CLIENT "ssl_client.key");


std::unordered_map<std::string,std::function<int(Context &, int argc, char *argv[])>> taskToFunc = {
  { "download", &download_doc },
  { "remove", &remove_doc },
  { "search", &search_doc },
  { "upload", &upload_doc },
  { "add-company", &add_company },
  { "del-company", &del_company },
  { "list-companies", &list_company }
};

void print_usage();
void print_help(const char *task) {
  print(fout, "Usage: \n", task);
}
int perform_task(int argc, char *argv[]) {
  if(argc < 2) {
    print_usage();
    return -1;
  }

  Context ctx = init_ctx_client(caPath, certPath, keyPath);

  if(ctx == nullptr) {
    return -1;
  }

  const char *task = *argv;

  if(!std::strcmp(task, "help")) {
    print_help(argv[1]);

    return 0;
  }

  try {
    return taskToFunc.at(task)(ctx, argc, argv);
  } catch(std::out_of_range &err) {
    print_usage();

    return -1;
  }
  return 0;
}

void print_usage() {
  print(fout,
    "Usage: client.out TASK... [OPTIONS] HOST PORT\n"
    "   help TASK...   --- print details on task\n"
    "   search         --- search for specific documents\n"
    "   upload         --- upload a new document\n"
    "   download       --- download an existing document\n"
    "   remove         --- remove an existing document\n"
    "   list-companies --- List all companies\n"
    "   add-company    --- create new company\n"
    "   del-company    --- delete a company\n"
  );
}


int main(int argc, char *argv[]) {
  init_ssl();
  return perform_task(argc -1, argv + 1);

/*
  Context ctx = init_ssl(caPath, certPath, keyPath);

  if(ctx.get() == nullptr) {
    return -1;
  }

  sslFile server = ssl_connect(ctx, "localhost", "8081");

  if(!server.is_open()) {
    return -1;
  }

  print(server, "Hello world!");
*/
}
