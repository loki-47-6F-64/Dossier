#include <sys/stat.h>

#include "client/args.h"
#include "file/_ssl.h"

#include "err.h"

constexpr int buffer_size = 1024;

int setArgs(up_args& args, int argc, char *argv[]) {
  if(argc != 4) {
    return -1;
  }

  args.company = *argv++;
  args.infile = *argv++;
  args.host = *argv++;
  args.port = *argv++;

  return 0;
}

int upload_doc(Context &ctx, int argc, char *argv[]) {
  up_args args;
  if(setArgs(args, argc -1, argv +1)) {
    print_help(
      "   FILE: file to upload\n"
      "   HOST PORT\n"
    );
    return -1;
  }

  return perform_upload(ctx, args);
}

int perform_upload(Context &ctx, up_args &args) {
  sslFile server = ssl_connect(ctx, args.host.c_str(), args.port.c_str());

  if(!server.is_open()) {
    return -1;
  }

  struct stat status;

  stat(args.infile.c_str(), &status);

  ioFile in(buffer_size, -1);

  in.access(args.infile.c_str(), fileStreamRead);
  print(server,
    static_cast<char>(_req_code::UPLOAD),
    args.company, '\0',
    status.st_size, '\0'
  );

  if(in.copy(server)) {
    return -1;
  }

  if(server.next()) {
    print(ferr, "Server error: ", server_err(server), '\n');
    return -1;
  }
  return 0;
}
