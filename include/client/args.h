#ifndef DOSSIER_CLIENT_ARGS_H
#define DOSSIER_CLIENT_ARGS_H

#include "main.h"
#include "file/_ssl.h"

namespace _response {
  enum : char { 
    OK,
    INTERNAL_ERROR,
    CORRUPT_REQUEST
  };
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
struct mod_company_args {
  std::string company;

  std::string host, port;
};

struct list_args {
  std::string host, port;
};

struct down_args {
  std::string idPage;
  ioFile outfile {-1};

  std::string host, port;
};

struct del_args {
  std::string idPage;

  std::string host, port;
};

struct up_args {
  std::string infile;
  std::string company;

  std::string host, port;
};

struct s_args {
  std::string year, month, day;
  std::string company;

  std::vector<const char*> keywords;
  std::string host, port;
};

/*
int setArgs(mod_company_args &args, int argc, char *argv[]);
int setArgs(list_args &args, int argc, char *argv[]);
int setArgs(down_args& args, int argc, char *argv[]);
int setArgs(del_args &args, int argc, char *argv[]);
int setArgs(up_args& args, int argc, char *argv[]);
int setArgs(s_args& args, int argc, char *argv[]);
*/

int download_doc(Context &ctx, int argc, char *argv[]);
int remove_doc(Context &ctx, int argc, char *argv[]);
int search_doc(Context &ctx, int argc, char *argv[]);
int upload_doc(Context &ctx, int argc, char *argv[]);
int add_company(Context &ctx, int argc, char *argv[]);
int del_company(Context &ctx, int argc, char *argv[]);
int list_company(Context &ctx, int argc, char *argv[]);

int perform_download(Context &ctx, down_args &args);
int perform_remove(Context &ctx, del_args &args);
int perform_search(Context &ctx, s_args &args);
int perform_upload(Context &ctx, up_args &args);
int perform_add_company(Context &ctx, mod_company_args &args);
int perform_del_company(Context &ctx, mod_company_args &args);
int perform_list_company(Context &ctx, list_args &args);

#endif
