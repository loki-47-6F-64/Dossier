#include "client/args.h"

#include "err.h"

namespace dossier {
namespace client {
int setArgs(down_args& args, int argc, char *argv[]) {
  if(argc != 4) {
    return -1;
  }

  args.idPage  = *argv++;
  args.outFile = *argv++;
  args.host    = *argv++;
  args.port    = *argv++;

  return 0;
}

int download_doc(Context &ctx, int argc, char *argv[]) {
  down_args args;
  if(setArgs(args, argc -1, argv +1)) {
    print_help(
      "   idPage: document to download\n"
      "   FILE: file to download to\n"
      "   HOST PORT\n"
    );
    return -1;
  }
  return perform_download(ctx, args);
}

int perform_download(Context &ctx, down_args &args) {
  sslFile server = ssl_connect(ctx, args.host.c_str(), args.port.c_str());

  if(!server.is_open()) {
    return -1;
  }

  print(server,
    static_cast<char>(_req_code::DOWNLOAD),
    args.idPage, '\0'
  );

  if(server.next()) {
//    print(ferr, "Server error: ", server_err(server), '\n');
    server_err(server);
    return -1;
  }

  ioFile outfile { -1 };
  if(outfile.access(args.outFile, fileStreamWrite)) {
    set_err(("Couldn't open file " + std::move(args.outFile) + ": " + sys_err()).c_str());
  }

  if(server.copy(outfile)) {
    return -1;
  }

  return 0;
}
};
};
