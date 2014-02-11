#include <string>
#include <cstring>

#include <unistd.h>
#include <sys/stat.h>

#include "client/main.h"
#include "file/_ssl.h"
#include "err.h"

#define ROOT_SERVER "/home/loki/keys/server/"
#define ROOT_CLIENT "/home/loki/keys/client/"

constexpr int buffer_size = 1024;

ioFile fout(buffer_size, -1, STDOUT_FILENO);
ioFile fin (buffer_size, -1, STDIN_FILENO);
ioFile ferr(buffer_size, -1, STDERR_FILENO);

std::string
    caPath(ROOT_SERVER "root.crt"),
    certPath(ROOT_CLIENT "ssl_client.crt"),
    keyPath(ROOT_CLIENT "ssl_client.key");

constexpr char
  DASH = '-',

  S_YEAR  = 'y',
  S_MONTH = 'm',
  S_DAY   = 'd',

  S_KEYWORD = 'k',
  S_COMPANY = 'c';


struct s_args {
  std::string year, month, day;
  std::string company;

  std::vector<const char*> keywords;
  std::string host, port;
};

struct del_args {
  std::string idPage;

  std::string host, port;
};

struct down_args {
  std::string idPage;
  ioFile outfile {-1};

  std::string host, port;
};

struct up_args {
  std::string infile;
  std::string company;

  std::string host, port;
};

struct list_args {
  std::string host, port;
};

struct mod_company_args {
  std::string company;

  std::string host, port;
};

namespace _req_code {
  enum : char {
    SEARCH,
    LIST_COMPANIES,
    DOWNLOAD,
    UPLOAD,
    NEW_COMPANY,
    REMOVE_COMPANY,
    REMOVE_DOCUMENT
  };
};

namespace _response {
  enum : char {
    OK,
    INTERNAL_ERROR,
    CORRUPT_REQUEST
  };
};

int delArgs(del_args &args, int argc, char *argv[]) {
  if(argc != 3) {
    return -1;
  }

  args.idPage = *argv++;

  args.host = *argv++;
  args.port = *argv++;

  return 0;
}

int listArgs(list_args &args, int argc, char *argv[]) {
  if(argc != 2) {
    return -1;
  }

  args.host = *argv++;
  args.port = *argv++;

  return 0;
}

int modCompanyArgs(mod_company_args &args, int argc, char *argv[]) {
  if(argc != 3) {
    return -1;
  }

  args.company = *argv++;
  args.host = *argv++;
  args.port = *argv++;

  return 0;
}

int uploadArgs(up_args& args, int argc, char *argv[]) {
  if(argc != 4) {
    return -1;
  }

  args.company = *argv++;
  args.infile = *argv++;
  args.host = *argv++;
  args.port = *argv++;

  return 0;
}

int downArgs(down_args& args, int argc, char *argv[]) {
  if(argc != 4) {
    return -1;
  }

  args.idPage = *argv++;
  args.outfile.access(*argv++, fileStreamRead);
  args.host = *argv++;
  args.port = *argv++; 

  return 0;
}

int searchArgs(s_args& args, int argc, char *argv[]) {
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

void print_usage();
void print_help(const char *task);
int perform_task(int argc, char *argv[]) {
  if(argc < 2) {
    print_usage();
    return -1;
  }

  Context ctx = init_ctx_client(caPath, certPath, keyPath);

  if(ctx.get() == nullptr) {
    return -1;
  }

  const char *task = *argv;
  if(!std::strcmp(task, "search")) {
    s_args args;
    if(searchArgs(args, argc -1, argv +1)) {
      print_help(task);
      return -1;
    }

    if(args.host.empty()) {
      print_help(task);
      return -1;
    }

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
  else if(!std::strcmp(task, "upload")) {
    up_args args;
    if(uploadArgs(args, argc -1, argv +1)) {
      print_help(task);
      return -1;
    }

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
  }
  else if(!std::strcmp(task, "download")) {
    down_args args;
    if(downArgs(args, argc -1, argv +1)) {
      print_help(task);
      return -1;
    }

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
  }
  else if(!std::strcmp(task, "remove")) {
    del_args args;
    if(delArgs(args, argc -1, argv +1)) {
      print_help(task);
      return -1;
    }

    sslFile server = ssl_connect(ctx, args.host.c_str(), args.port.c_str());

    if(!server.is_open()) {
      return -1;
    }

    print(server,
      static_cast<char>(_req_code::REMOVE_DOCUMENT),
      args.idPage.c_str(), '\0'
    );

    if(server.next()) {
      print(ferr, "Server error: ", server_err(server), '\n');
      return -1;
    }
  }
  else if(!std::strcmp(task, "list-companies")) {
    list_args args;
    if(listArgs(args, argc -1, argv +1)) {
      print_help(task);
      return -1;
    }

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
  }
  else if(!std::strcmp(task, "add-company")) {
    mod_company_args args;
    if(modCompanyArgs(args, argc -1, argv +1)) {
      print_help(task);
      return -1;
    } 
    
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
  }
  else if(!std::strcmp(task, "del-company")) {
    mod_company_args args;
    if(modCompanyArgs(args, argc -1, argv +1)) {
      print_help(task);
      return -1;
    } 
    
    sslFile server = ssl_connect(ctx, args.host.c_str(), args.port.c_str());

    if(!server.is_open()) {
      return -1;
    } 
    
    print(server,
      static_cast<char>(_req_code::REMOVE_COMPANY),
      args.company.c_str(), '\0'
    );

    if(server.next()) {
      print(ferr, "Server error: ", server_err(server), '\n');
      return -1;
    }
  }
  else if(!std::strcmp(task, "help")) {
    print_help(argv[1]);
  }
  else {
    print_usage();
  }
  return 0;
}

void print_help(const char *task) {
  if(!std::strcmp(task, "search")) {
    print(fout,
      "Usage:\n"
      "   -c  company name to search by\n"
      "   -y  year to search by\n"
      "   -m  month to search by\n"
      "   -d  day to search by\n"
    );
  }
  else if(!std::strcmp(task, "upload")) {
    print(fout,
      "Usage:\n"
      "   Company\n"
      "   FILE: file to upload\n"
      "   HOST PORT\n"
    );
  }
  else if(!std::strcmp(task, "download")) {
    print(fout,
      "Usage:\n"
      "   idPage: document to download\n"
      "   FILE: file to download to\n"
      "   HOST PORT\n"
    );
  }
  else if(!std::strcmp(task, "remove")) {
    print(fout,
      "Usage:\n"
      "   idPage: document to remove\n"
      "   HOST PORT\n"
    );
  }
  else if(!std::strcmp(task, "list-companies")) {
    print(fout,
      "Usage:\n"
      "   It simply lists all companies\n"
      "   HOST PORT\n"
    );
  }
  else if(!std::strcmp(task, "add-company")) {
    print(fout,
      "Usage:\n"
      "   NAME: name of the company\n"
      "   HOST PORT\n"
    );
  }
  else if(!std::strcmp(task, "del-company")) {
    print(fout,
      "Usage:\n"
      "   NAME: name of the company\n"
      "   HOST PORT\n"
    );
  }
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
  perform_task(argc -1, argv + 1);

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
  return 0;
}
