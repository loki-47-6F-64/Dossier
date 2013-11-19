#ifndef DOSSIER_SQL_CONNECT_H
#define DOSSIER_SQL_CONNECT_H

#include <functional>
#include <string>

#include <mysql/my_global.h>
#include <mysql/mysql.h>

// #include <vector>
// #include <functional>

class SqlConnect {
	MYSQL _con;

	// Result of last query
	MYSQL_RES* _res;
public:
	SqlConnect();
	~SqlConnect();

	int query(std::string&& sql);

	// Returned MYSQL struct should not be used
	MYSQL* open(const char *host, const char *user, const char *passwd);

	void eachRow(std::function<void(MYSQL_ROW)> f);
	void eachField(MYSQL_ROW row, std::function<void(char*, uint64_t)> f);

	void setdb(const char *db);

	unsigned int totalFields();
	unsigned int totalRows();

	const char *error();

	void close();	
};

#endif
