#include "main.h"
#include "proxy.h"

#include "file.h"
#include "database.h"

std::unique_ptr<requestBase> getRequest(sslFile *socket) {
  std::unique_ptr<requestBase> _req;
	switch (socket->next()) {
		case _req_code::SEARCH:
			_req = std::unique_ptr<requestSearch>(new requestSearch());
			break;
    case _req_code::LIST_COMPANIES:
      _req = std::unique_ptr<requestListCompanies>(new requestListCompanies());
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

int requestListCompanies::insert(sslFile *_socket) {
  this->_socket = _socket;

  return 0;
}

int requestSearch::insert(sslFile *_socket) {
	DEBUG_LOG("Parse search request");

	this->_socket = _socket;

	if(_customAppend(company,  MAX_COMPANY))
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

  DEBUG_LOG(company.c_str());

  for(auto& keyword : keywords) {
    DEBUG_LOG(keyword.c_str());
  }
  return 0;
}

int requestDownload::insert(sslFile *_socket) {
	DEBUG_LOG("Parse download request");

	this->_socket = _socket;

  if(_customAppend(idPage) ||
     _customAppend(company,  MAX_COMPANY))
  {
    return -1;
  }

  DEBUG_LOG(idPage);
  DEBUG_LOG(company.c_str());

	return 0;
}

int requestUpload::insert(sslFile *_socket) {
	DEBUG_LOG("Parse upload request");

	this->_socket = _socket;

  if(_customAppend(company,  MAX_COMPANY) ||
     _customAppend(size))
  {
    return -1;
  }

  return 0;
}

int requestNewCompany::insert(sslFile *_socket) {
  DEBUG_LOG("Parse new Company request");

  this->_socket = _socket;

  if(_customAppend(name,  MAX_COMPANY))
  { 
    return -1;
  }

  DEBUG_LOG(name.c_str());

  return 0;
}

int requestSearch::exec(Database &db) {
  DEBUG_LOG("Execute search request");

  _socket->getCache().clear();
  std::vector<meta_doc> result = db.search(idUser, company);
 
  _socket->append(_response::OK);
  for(auto& doc : result) {
    _socket->append(doc.created);
    _socket->append('\0');

    _socket->append(doc.company);
    _socket->append('\0');

    _socket->append(std::to_string(doc.id));
    _socket->append('\0');
  }

  _socket->out();
 
  return 0;
}

int requestDownload::exec(Database &db) {
  DEBUG_LOG("Excecute download requests");

  meta_doc result = db.getFile(idUser, idPage);
  
  DEBUG_LOG(result.company.c_str());
  DEBUG_LOG(result.id);

  return 0;
}

int requestUpload::exec(Database &db) {
  DEBUG_LOG("Execute upload request");

  int64_t idPage = db.newDocument(idUser, company);

  if(db.err_msg) {
    err_msg = db.err_msg;

    _socket->getCache().clear();

    _socket->append(_response::INTERNAL_ERROR);
    _socket->append(err_msg);
    _socket->out();
  }
   
  /* root/idUser/ */
  std::string path = config::storage.root;
  if(path.back() != '/') {
    path += '/';
  }
  path.append(std::to_string(idUser));
  path += '_';

  path.append(std::to_string(idPage));
  path += ".txt";

  ioFile out(1);
  out.access(path);

  _socket->copy<ioFile>(out, size);


  _socket->getCache().clear();

  _socket->append(_response::OK);
  _socket->out();

  return 0;
}

int requestNewCompany::exec(Database &db) {
  DEBUG_LOG("Execute new Company request");

  _socket->getCache().clear();
  if(db.newCompany(name, idUser)) {
    err_msg = db.err_msg;
    _socket->append(_response::INTERNAL_ERROR);
    _socket->append(err_msg);

    _socket->out();

    return -1;
  }

  _socket->append(_response::OK);
  _socket->out();

  return 0;
}

int requestListCompanies::exec(Database &db) {
  DEBUG_LOG("Execute list Company request");

  _socket->getCache().clear();
  std::vector<std::string> companies = db.listCompany(idUser);

  if(db.err_msg) {
    err_msg = db.err_msg;
    _socket->append(_response::INTERNAL_ERROR);
    _socket->append(err_msg);

    _socket->out();

    return -1;
  }

  _socket->append(_response::OK);

  for(auto& company : companies) {
    _socket->append(company);
    _socket->append('\0');
  }

  _socket->out();

  return 0;
}
