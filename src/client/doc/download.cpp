#include "client/args.h"

#include "err.h"

int setArgs(down_args& args, int argc, char *argv[]) {
  if(argc != 4) {
    return -1;
  }

  args.idPage = *argv++;
  args.outfile.access(*argv++, fileStreamRead);
  args.host = *argv++;
  args.port = *argv++;

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
    args.idPage.c_str(), '\0'
  );

  if(server.next()) {
    print(ferr, "Server error: ", server_err(server), '\n');
    return -1;
  }

  if(server.copy(args.outfile)) {
    return -1;
  }

  return 0;
}
