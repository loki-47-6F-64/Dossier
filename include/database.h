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
  std::string created;
};

class Database {
	SqlConnect _sql;
public:
	const char *err_msg = nullptr;

	Database();
//	~Database();

	std::vector<meta_doc> search(int64_t idUser, std::string &company,
    std::string &year, std::string &month, std::string &day);

	meta_doc getFile(int64_t idUser, int64_t idPage);

  // Create new meta_doc and return the id.
 	int64_t newDocument(int64_t idUser, std::string &company);
  int removeDocument(int64_t idPage, int64_t idUser);

  // Return id user
	int64_t validateUser(std::string& username);

	int newUser(std::string &username,
									std::string &email,
								  std::string &hash);

  int newCompany(std::string &name, int64_t idUser);
  int removeCompany(std::string &name, int64_t idUser);

  std::vector<std::string> listCompany(int64_t idUser);
};

#endif
