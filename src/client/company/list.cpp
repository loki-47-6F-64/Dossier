#include "client/main.h"
#include "file/_ssl.h"

#include "client/args.h"
#include "err.h"

namespace dossier {
namespace client {
int setArgs(list_args &args, int argc, char *argv[]) {
  if(argc != 2) {
    return -1;
  }

  args.host = *argv++;
  args.port = *argv++;

  return 0;
}

int list_company(Context &ctx, int argc, char *argv[]) {
  list_args args;
  if(setArgs(args, argc -1, argv +1)) {
    print_help(
      "   It simply lists all companies\n"
      "   HOST PORT\n"
    );
    return -1;
  }

  return perform_list_company(ctx, args);
}

int perform_list_company(Context &ctx, list_args &args) {
  sslFile server = ssl_connect(ctx, args.host.c_str(), args.port.c_str());

  if(!server.is_open()) {
    return -1;
  }

  print(server,
    static_cast<char>(_req_code::LIST_COMPANIES)
  );

  if(server.next()) {
    print(ferr, "Server error: ", server_err(server), '\n');
    return -1;
  }

  server.eachByte([&](unsigned char ch) {
    if(!ch) {
      if(fout.getCache().empty()) {
        return 0;
      }

      if(fout.getCache().back() == '\n') {
        return static_cast<int>(FileErr::BREAK);
      }

      ch = '\n';
    }

    fout.append(ch);

    return 0;
  });

  fout.out();
  return 0;
}

};
};
