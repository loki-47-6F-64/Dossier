#include <string>
#include <sstream>

#include "server/main.h"
#include "database/database.h"
Database::Database() {
  if(!_sql.open(
    config::database.host.c_str(),
    config::database.user.c_str(),
    config::database.password.c_str()))
  {
    err_msg = _sql.error();
    return;
  }

  _sql.setdb(config::database.db.c_str());
}


int Database::newUser(std::string &username,
                      std::string &email)
{
  _sql.sanitize(username); _sql.sanitize(email);
  std::ostringstream query;

  query << "INSERT INTO user (username, email, password) VALUES ('";
  query << username.data(); query << "','";
  query << email.data(); query << "',')";

  if(_sql.query(query.str())) {
    err_msg = _sql.error();
    return -1;
  }
  return 0;
}

int Database::removeDocument(int64_t idPage, int64_t idUser) {
  std::ostringstream query;

  query << "DELETE FROM Document WHERE idPage=" << idPage
        << " AND user_idUser=" << idUser;

  if(_sql.query(query.str())) {
    err_msg = _sql.error();
    return -1;
  }
  return 0;
}

int64_t Database::validateUser(std::string& username) {
  _sql.sanitize(username);

  std::ostringstream query;

  query << "SELECT 1 FROM user WHERE username='" << 
  username.data() << "' LIMIT 1";
  //query << "' AND password='";
  //query << hash.data(); query << "' LIMIT 1)";

  if(_sql.query(query.str())) {
    err_msg = _sql.error();
    return -1;
  }

  bool rejected = true;

  int64_t id = 0;
  _sql.eachRow([&](MYSQL_ROW row, unsigned long*) {
    rejected = false;

    id = std::strtol(*row, nullptr, 10);
  });

  if(rejected)
    err_msg = "Authentication failed";
  return id;
}

std::vector<meta_doc> Database::search(int64_t idUser, std::string &company,
  int year, int month, int day,
  std::vector<std::string>& keywords)
{
  _sql.sanitize(company);

  std::ostringstream query;
  query << "SELECT doc.idPage, Company.name, doc.content, doc.created FROM Document AS doc INNER JOIN (Company) ON (Company.idCompany=doc.Company_idCompany) WHERE doc.user_idUser='" << idUser << '\'';
  if(!company.empty()) {
    query << " AND Company.name='" << company << '\'';
  }

  if(day) {
    query << " AND DAY(doc.created)='" << day << '\'';
  }

  if(month) {
    query << " AND MONTH(doc.created)='" << month << '\'';
  }

  if(year) {
    query << " AND YEAR(doc.created)='" << year << '\'';
  }

  if(!keywords.empty()) {
    query << " AND MATCH(doc.content) AGAINST('";
    size_t size = keywords.size();
    for(size_t x = 0; x+1 < size; ++x) {
      query << keywords[x] << ',';
    }
    query << keywords.back() << "')";
  }

  std::vector<meta_doc> result;
  if(_sql.query(query.str())) {
    err_msg = _sql.error();
    return result;
  }

  _sql.eachRow([&](MYSQL_ROW row, unsigned long*lengths) {
    std::string idPage(*row, *lengths);

    int previewLength = lengths[2] > 30 ? 30 : lengths[2];
    meta_doc tmp {
      std::stol(idPage),
      std::string(row[1], lengths[1]), // company
      std::string(row[2], previewLength), // preview of content
      std::string(row[3], lengths[3])  // date
    };

    result.push_back(std::move(tmp));
  });

  return result;
}

meta_doc Database::getFile(int64_t idUser, int64_t idPage) {
  meta_doc result;

  std::ostringstream query;
  query << "SELECT doc.idPage, Company.name FROM Document AS doc INNER JOIN (Company) ON (Company.idCompany=doc.Company_idCompany) WHERE doc.user_idUser='" <<
    idUser << "' AND doc.idPage=" << idPage;

  if(_sql.query(query.str())) {
    err_msg = _sql.error();
    return result;
  }

  _sql.eachRow([&](MYSQL_ROW row, unsigned long*lengths) {
    char *dummy;
  
    std::string idPage(*row, *lengths);

    result.id = std::strtol(idPage.c_str(), &dummy, 10);
    result.company.append(row[1], lengths[1]);
  });

  return result;
}

int64_t Database::newDocument(int64_t idUser, std::string &company) {
  _sql.sanitize(company);

  std::ostringstream query;
  query << "INSERT INTO Document (user_idUser, Company_idCompany) VALUES (" << idUser << ", (SELECT idCompany FROM Company WHERE Company.user_idUser=" << idUser << " AND Company.name='" << company << "' LIMIT 1))";

  if(_sql.query(query.str())) {
    err_msg = _sql.error();
    return 0;
  }
  return _sql.idInserted();
}

int Database::newCompany(std::string &name, int64_t idUser) {
  _sql.sanitize(name);

  std::ostringstream query;

  query << "INSERT INTO Company (user_idUser, name) VALUES ("
        << idUser << ", '"
        << name   << "')";

  if(_sql.query(query.str())) {
    err_msg = _sql.error();
    return -1;
  }
  return 0;
}

int Database::removeCompany(std::string &name, int64_t idUser) {
  _sql.sanitize(name);

  std::ostringstream query;

  query << "DELETE FROM Company WHERE name='" << name << "'"
        << "AND user_idUser='" << idUser << "'";

  if(_sql.query(query.str())) {
    err_msg = _sql.error();
    return -1;
  }

  return 0;
}

std::vector<std::string> Database::listCompany(int64_t idUser) {
  std::ostringstream query;

  query << "SELECT name FROM Company WHERE user_idUser='" << idUser << "'";

  std::vector<std::string> result;
  if(_sql.query(query.str())) {
    err_msg = _sql.error();
    return result;
  }

  _sql.eachRow([&](MYSQL_ROW row, unsigned long*lengths) {
    result.push_back(std::string(*row, *lengths));
  });

  return result;
}

int Database::setDocContent(int64_t idPage, std::string &content) {
  _sql.sanitize(content);

  std::ostringstream query;

  query << "UPDATE Document SET content='" << content << "'"
          " WHERE idPage=" << idPage;

  if(_sql.query(query.str())) {
    err_msg = _sql.error();
    return -1;
  }

  return 0;
}
