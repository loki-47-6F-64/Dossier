#include "file/_ssl.h"

#include "client/args.h"
#include "err.h"

namespace dossier {
namespace client {
int setArgs(mod_company_args &args, int argc, char *argv[]) {
  if(argc != 3) {
    return -1;
  }

  args.company = *argv++;
  args.host = *argv++;
  args.port = *argv++;

  return 0;
}

int add_company(Context &ctx, int argc, char *argv[]) {
  mod_company_args args;
  if(setArgs(args, argc -1, argv +1)) {
    print_help(
      "   NAME: name of the company\n"
      "   HOST PORT\n"
    );
    return -1;
  }

  return perform_add_company(ctx, args);
}

int perform_add_company(Context &ctx, mod_company_args &args) {
  sslFile server = ssl_connect(ctx, args.host.c_str(), args.port.c_str());

  if(!server.is_open()) {
    return -1;
  }

  print(server,
    static_cast<char>(_req_code::NEW_COMPANY),
    args.company.c_str(), '\0'
  );

  if(server.next()) {
    print(ferr, "Server error: ", server_err(server), '\n');
    return -1;
  }
  return 0;
}
};
};
