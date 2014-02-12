#include "client/args.h"

#include "err.h"

constexpr char
  DASH = '-',

  S_YEAR  = 'y',
  S_MONTH = 'm',
  S_DAY   = 'd',

  S_KEYWORD = 'k',
  S_COMPANY = 'c';

int setArgs(s_args& args, int argc, char *argv[]) {
  for(int x = 0; x < argc; ++x) {
    char *arg = argv[x];

    if(*arg != DASH) {
      if(args.host.empty()) {
        args.host = arg;
        continue;
      }
      else if(args.port.empty()) {
        args.port = arg;
        continue;
      }
      else {
        return -1;
      }
    }

    if(x + 1 >= argc) {
      return -1;
    }

    while(*++arg) {
      switch(*arg) {
        case S_COMPANY:
          args.company = argv[++x];
          break;
        case S_KEYWORD:
          args.keywords.push_back(argv[++x]);
          break;
        case S_YEAR:
          args.year = argv[++x];
          break;
        case S_MONTH:
          args.month = argv[++x];
          break;
        case S_DAY:
          args.day = argv[++x];
          break;
        default:
          return -1;
      }
    }
  }

  return 0;
}

int search_doc(Context &ctx, int argc, char *argv[]) {
  s_args args;
  if(setArgs(args, argc -1, argv +1)) {
    print_help(
      "   -c  company name to search by\n"
      "   -y  year to search by\n"
      "   -m  month to search by\n"
      "   -d  day to search by\n"
    );
    return -1;
  }

  if(args.host.empty()) {
    print_help(
      "   -c  company name to search by\n"
      "   -y  year to search by\n"
      "   -m  month to search by\n"
      "   -d  day to search by\n"
    );
    return -1;
  }

  return perform_search(ctx, args);
}

int perform_search(Context &ctx, s_args &args) {
  sslFile server = ssl_connect(ctx, args.host.c_str(), args.port.c_str());

  if(!server.is_open()) {
    return -1;
  }

  server.append(static_cast<char>(_req_code::SEARCH))
        .append(args.company).append('\0')
        .append(args.year).append('\0')
        .append(args.month).append('\0')
        .append(args.day).append('\0');


  for(auto& keyword : args.keywords) {
    server.append(keyword).append('\0');
  }

  server.append('\0').out();

  if(server.next()) {
    print(ferr, "Server error: ", server_err(server), '\n');
    return -1;
  }

  int x = 0;
  /*server.eachByte([&](unsigned char ch) {
    if(!ch) {
      if(x++ == 3) {
        x = 0;
        ch = '\n';
      }
      else if(!fout.getCache().empty() && fout.getCache().back() == '\n') {
        return static_cast<int>(FileErr::BREAK);
      }
      else {
        ch = ',';
      }
    }

    fout.append(ch);
    return 0;
  });

  return fout.out();*/
  return server.copy(fout);
}
