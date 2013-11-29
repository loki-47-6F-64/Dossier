#include "sql_connect.h"

SqlConnect::SqlConnect() : _res(nullptr) {
	mysql_init(&_con);
}

SqlConnect::~SqlConnect() {
	close();
}

int SqlConnect::query(std::string&& sql) {
	if(_res != nullptr) {
		mysql_free_result(_res);
		_res = nullptr;
	}

	mysql_real_query(&_con, sql.c_str(), sql.size());
	_res = mysql_store_result(&_con);

	return mysql_errno(&_con) != 0;
}

MYSQL* SqlConnect::open(const char *host, const char *user, const char *passwd) {
	return mysql_real_connect(&_con, host, user, passwd, nullptr, 0, nullptr, 0);
}

void SqlConnect::setdb(const char *db) {
	mysql_select_db(&_con, db);
}

unsigned int SqlConnect::totalFields() {
	return mysql_num_fields(_res);
}

void SqlConnect::eachRow(std::function<void(MYSQL_ROW, uint64_t*)> f) {
	MYSQL_ROW row;
	while((row = mysql_fetch_row(_res))) {
		f(row, mysql_fetch_lengths(_res));
  }
}

const char *SqlConnect::error() { return mysql_error(&_con); }

unsigned int SqlConnect::totalRows() { return mysql_affected_rows(&_con); }

void SqlConnect::close() {
	if(_res != nullptr) {
		mysql_free_result(_res);
		_res = nullptr;
	}

	mysql_close(&_con);
}

int64_t SqlConnect::idInserted() { return mysql_insert_id(&_con); }
