#include "main.h"
#include "proxy.h"

#include "authenticate.h"

enum _req_code {
  SEARCH,
  DOWNLOAD,
  UPLOAD,
  NEW_COMPANY,
  AUTHENTICATE
};

enum _response {
  OK,
  INTERNAL_ERROR,
  CORRUPT_REQUEST,
  UNAUTHORIZED
};

std::unique_ptr<requestBase> getRequest(ioFile *socket) {
  std::unique_ptr<requestBase> _req;
	switch (socket->next()) {
		case _req_code::SEARCH:
			_req = std::unique_ptr<requestSearch>(new requestSearch());
			break;
		case _req_code::DOWNLOAD:
			_req = std::unique_ptr<requestDownload>(new requestDownload());
			break;
		case _req_code::UPLOAD:
			_req = std::unique_ptr<requestUpload>(new requestUpload());
			break;
    case _req_code::NEW_COMPANY:
      _req = std::unique_ptr<requestNewCompany>(new requestNewCompany());
      break;
    case _req_code::AUTHENTICATE:
      _req = std::unique_ptr<requestAuthenticate>(new requestAuthenticate());
    break;
	}
  return _req;
}

int requestBase::_customAppend(int64_t &buf) {
  constexpr int max_digits = 10;

  std::string str;
  if(_customAppend(str, max_digits)) {
    return -1;
  }

  buf = std::stol(str);
  return 0;
}

int requestBase::_customAppend(std::string &buf, int max) {
  bool cont = true;
  int result = 0;

  while(cont) {
    if(_socket->eof()) {
      err_msg = "Reached EOF before \\0";
      return -1;
    }

    _socket->eachLoaded([&](unsigned char ch) {
      if(buf.size() > max) {
        result = -1;
        err_msg = "String to large";

        return false;
      }

      if(!ch) {
        return cont = false;
      }

      buf.push_back(ch);
      return true;
    });
  }
	return result;
}

int requestAuthenticate::insert(ioFile *_socket) {
  DEBUG_LOG("Parse authenticate request");

  this->_socket = _socket;

  if(_customAppend(username, MAX_USERNAME) ||
     _customAppend(password,  MAX_PASSWORD))
  {
    return -1;
  }

  DEBUG_LOG(username.c_str());
  DEBUG_LOG(password.c_str());

  return 0;
}

int requestSearch::insert(ioFile *_socket) {
	DEBUG_LOG("Parse search request");

	this->_socket = _socket;

	if(_customAppend(token, MAX_TOKEN) ||
		 _customAppend(company,  MAX_COMPANY))
  {
    return -1;
  }

	for(int x = 0;x < MAX_PARAMETERS; x++) {
		std::string tmp;

		if(_customAppend(tmp, MAX_KEYWORD)) {
			DEBUG_LOG(err_msg);
			return -1;
		}

    // No more keywords
		if(tmp.empty())
			break;

		keywords.push_back(std::move(tmp));
	}

  DEBUG_LOG(token.c_str());
  DEBUG_LOG(company.c_str());

  for(auto& keyword : keywords) {
    DEBUG_LOG(keyword.c_str());
  }
  return 0;
}

int requestDownload::insert(ioFile *_socket) {
	DEBUG_LOG("Parse download request");

	this->_socket = _socket;

	if(_customAppend(token, MAX_TOKEN) ||
     _customAppend(idPage)                 ||
     _customAppend(company,  MAX_COMPANY))
  {
    return -1;
  }

  DEBUG_LOG(token.c_str());
  DEBUG_LOG(idPage);
  DEBUG_LOG(company.c_str());

	return 0;
}

int requestUpload::insert(ioFile *_socket) {
	DEBUG_LOG("Parse upload request");

	this->_socket = _socket;

	if(_customAppend(token, MAX_TOKEN) ||
     _customAppend(company,  MAX_COMPANY))
  {
    return -1;
  }

  return 0;
}

int requestNewCompany::insert(ioFile *_socket) {
  DEBUG_LOG("Parse new Company request");

  this->_socket = _socket;

  if(_customAppend(token, MAX_TOKEN) ||
     _customAppend(name,  MAX_COMPANY))
  { 
    return -1;
  }

  DEBUG_LOG(token.c_str());
  DEBUG_LOG(name.c_str());

  return 0;
}

int requestAuthenticate::exec() {
  DEBUG_LOG("Execute authenticate request");

  Database db;
  Authenticater auth(&db);

  std::string token = auth.validate(username, password);

  _socket->getCache().clear();
  if(!auth.err_msg) {
    // Return token
    _socket->append(_response::OK);
    _socket->append(token);
    _socket->out();

    return 0;
  }

  _socket->append(_response::INTERNAL_ERROR);
  _socket->append("Username/Password is incorrect");
  _socket->out();

  err_msg = auth.err_msg;
  return -1;
}

int requestSearch::exec() {
  DEBUG_LOG("Execute search request");

  Database db;
  if(db.err_msg) {
    err_msg = db.err_msg;
    return -1;
  }

  Authenticater auth(&db);

  int64_t id = auth.validate(token);

  _socket->getCache().clear();
  if(!auth.err_msg) {
    std::vector<meta_doc> result = db.search(id, company);
 
    _socket->append(_response::OK);
    for(auto& doc : result) {
      _socket->append(doc.company);
      _socket->append('\0');

      _socket->append(std::to_string(doc.id));
      _socket->append('\0');
    }

    _socket->out();
       
    return 0;
  }

  _socket->append(_response::UNAUTHORIZED);
  _socket->append("Unauthorized access");
  _socket->out();

  err_msg = "user validation failed.";

  return -1;
}

int requestDownload::exec() {
  DEBUG_LOG("Excecute download requests");

  Database db;
  Authenticater auth(&db);

  int64_t id = auth.validate(token);

  if(!auth.err_msg) {
    meta_doc result = db.getFile(id, idPage);
   
    DEBUG_LOG(result.company.c_str());
    DEBUG_LOG(result.id);

    return 0;
  }

  _socket->append(_response::UNAUTHORIZED);
  _socket->append("Unauthorized access");
  _socket->out();

  err_msg = "user validation failed.";

	return -1;
}

int requestUpload::exec() {
  DEBUG_LOG("Execute upload request");

  Database db;
  Authenticater auth(&db);

  int64_t idUser = auth.validate(token);

  if(!auth.err_msg) {
    int64_t idPage = db.newDocument(idUser, company);

        
    /* root/idUser/ */
    std::string path = config::storage.root;
    if(path.back() != '/') {
      path += '/';
    }
    path.append(std::to_string(idUser));
    path += '/';

    path.append(std::to_string(idPage));
    path += ".txt";

    ioFile out(1);
    out.access(path);

    _socket->copy(out);
    return 0;
  }

  _socket->append(_response::UNAUTHORIZED);
  _socket->append(auth.err_msg);
  _socket->append('\0');
  _socket->out();

  err_msg = "user validation failed.";

	return -1;
}

int requestNewCompany::exec() {
  DEBUG_LOG("Execute new Company request");
  Database db;

  Authenticater auth(&db);

  int64_t idUser = auth.validate(token);

  _socket->getCache().clear();
  if(!auth.err_msg) {
    db.newCompany(name, idUser);
    _socket->append(_response::OK);
    _socket->out();

    return 0;
  }

  _socket->append(_response::UNAUTHORIZED);
  _socket->append(auth.err_msg);
  _socket->append('\0');
  _socket->out();

  return -1;
}
