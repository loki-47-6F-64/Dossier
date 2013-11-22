#ifndef DOSSIER_DATABASE_H
#define DOSSIER_DATABASE_H

#include <memory>
#include <string>
#include <vector>

#include "proxy.h"
#include "sql_connect.h"

struct meta_doc {
  int64_t id;
  std::string company;
};

class Database {
	SqlConnect _sql;
public:
	const char *err_msg = nullptr;

	Database();
//	~Database();

	std::vector<meta_doc> search(requestSearch *req);
	meta_doc getFile(requestDownload *req);
	int newDocument(requestUpload *req);

	int validateUser(std::string &&username,
									 std::string &&hash);

	int insertUser(std::string &&username,
									std::string &&email,
								  std::string &&hash);
};

#endif
