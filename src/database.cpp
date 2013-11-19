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
	_sql.eachRow([&](MYSQL_ROW row) {
		_sql.eachField(row, [&](char *field, uint64_t length) {
      // Field is '0' authentication fail.
      rejected = *field == '0';
    });
	});
  if(rejected)
    err_msg = "Authentication failed";
  return rejected;
}

int Database::search(requestSearch *req) {

}
