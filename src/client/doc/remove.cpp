#include "client/main.h"
#include "client/args.h"

#include "file/_ssl.h"

#include "err.h"

namespace dossier {
namespace client {
int setArgs(del_args &args, int argc, char *argv[]) {
  if(argc != 3) {
    return -1;
  }

  args.idPage = *argv++;

  args.host = *argv++;
  args.port = *argv++;

  return 0;
}

int remove_doc(Context &ctx, int argc, char *argv[]) {
  del_args args;
  if(setArgs(args, argc -1, argv +1)) {
    print_help(
      "   idPage: document to remove\n"
      "   HOST PORT\n"
    );
    return -1;
  }
  return perform_remove(ctx, args);
}

int perform_remove(Context &ctx, del_args &args) {
  sslFile server = ssl_connect(ctx, args.host.c_str(), args.port.c_str());

  if(!server.is_open()) {
    return -1;
  }

  print(server,
    static_cast<char>(_req_code::REMOVE_DOCUMENT),
    args.idPage.c_str(), '\0'
  );

  if(server.next()) {
    server_err(server);
//    print(ferr, "Server error: ", server_err(server), '\n');
    return -1;
  }
  return 0;
}

};
};
