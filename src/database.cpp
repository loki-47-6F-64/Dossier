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


int Database::insertUser(std::string &&username,
                  		std::string &&email,
                  		std::string &&hash)
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

int Database::validateUser(std::string &&username, std::string &&hash) {
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

#include <iostream>
std::vector<meta_doc> Database::search(requestSearch *req) {
  std::ostringstream query;
  query << "SELECT Company.name FROM Document AS doc INNER JOIN (user, Company) ON (user.idUser=doc.user_idUser AND Company.idCompany=doc.Company_idCompany) WHERE user.username='" << req->username << '\'';
  if(!req->company.empty()) {
    query << " AND Company.name='" << req->company << '\'';
  }

  _sql.query(query.str());

  std::vector<meta_doc> result;
  _sql.eachRow([&](MYSQL_ROW row, uint64_t *lengths) {
    meta_doc tmp { std::string(*row, lengths[0]) };
    tmp.company += '\0';

    result.push_back(std::move(tmp));
  });

  return result;
}
