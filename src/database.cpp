#include <string>
#include <sstream>

#include "proxy.h"

#include "main.h"
#include "database.h"
Database::Database() {
	if(!_sql.open(
		config::database.host,
		config::database.user,
		config::database.password))
	{
		err_msg = "Failed to connect to database";
		return;
	}

	_sql.setdb(config::database.db);
}


int Database::insertUser(std::string &username,
                  		std::string &email,
                  		std::string &hash)
{
	std::ostringstream query;

	query << "INSERT INTO user (username, email, password) VALUES ('";
	query << username.data(); query << "','";
	query << email.data(); query << "','";
	query << hash.data(); query << "')";

	if(_sql.query(query.str())) {
		err_msg = _sql.error();
		return -1;
	}
	return 0;
}

int64_t Database::validateUser(std::string &username, std::string &hash) {
	std::ostringstream query;

	query << "SELECT EXISTS(SELECT 1 FROM user WHERE username='" << 
  username.data() << "' LIMIT 1)";
  //query << "' AND password='";
	//query << hash.data(); query << "' LIMIT 1)";

	if(_sql.query(query.str())) {
    err_msg = _sql.error();
    return -1;
  }

  int rejected;
	_sql.eachRow([&](MYSQL_ROW row, uint64_t*) {
    rejected = **row == '0';
	});

  if(rejected)
    err_msg = "Authentication failed";
  return rejected;
}

std::vector<meta_doc> Database::search(int64_t idUser, std::string &company) {
  std::ostringstream query;
  query << "SELECT doc.idPage, Company.name FROM Document AS doc INNER JOIN (Company) ON (Company.idCompany=doc.Company_idCompany) WHERE doc.idUser='" << idUser << '\'';
  if(!company.empty()) {
    query << " AND Company.name='" << company << '\'';
  }

  _sql.query(query.str());

  std::vector<meta_doc> result;
  _sql.eachRow([&](MYSQL_ROW row, uint64_t *lengths) {
    char *dummy;

    std::string idPage(*row, *lengths);
    idPage += '\0';

    meta_doc tmp { std::strtol(idPage.c_str(), &dummy, 10), std::string(row[1], lengths[1]) };
    tmp.company += '\0';

    result.push_back(std::move(tmp));
  });

  return result;
}

meta_doc Database::getFile(int64_t idUser, int64_t idPage) {
  std::ostringstream query;
  query << "SELECT doc.idPage, Company.name FROM Document AS doc INNER JOIN (Company) ON (Company.idCompany=doc.Company_idCompany) WHERE doc.user_idUser='" <<
    idUser << "' AND doc.idPage=" << idPage;

  _sql.query(query.str());

  meta_doc result;
  _sql.eachRow([&](MYSQL_ROW row, uint64_t *lengths) {
    char *dummy;
  
    std::string idPage(*row, *lengths);
    idPage += '\0';

    result.id = std::strtol(idPage.c_str(), &dummy, 10);
    result.company.append(row[1], lengths[1]);
    result.company += '\0';
  });

  return result;
}

int Database::newDocument(int64_t idUser, std::string &company) {
  std::ostringstream query;
  query << "INSERT INTO Document (user_idUser, Company_idCompany) VALUES (" << idUser << ", (SELECT idCompany FROM Company WHERE Company.user_idUser=" << idUser << " AND Company.name='" << company << "' LIMIT 1))";

  if(_sql.query(query.str())) {
    err_msg = _sql.error();
    return -1;
  }
  return 0;
}
