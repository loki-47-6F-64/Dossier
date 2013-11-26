#include "main.h"
#include "proxy.h"

#include "authenticate.h"

// Limit file _cache buffer size
const int REQ_BUFFER_SIZE = 1;

std::unique_ptr<requestBase> getRequest(ioFile *socket) {
	if(socket->load(REQ_BUFFER_SIZE) < 0) {
		FATAL_ERROR("Parse request", errno);
	}

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
    case _req_code::AUTHENTICATE:
      _req = std::unique_ptr<requestAuthenticate>(new requestAuthenticate());
    break;
	}
	_req->insert(socket);

  return _req;
}

int requestBase::_customAppend(int64_t &buf) {
  for(int x = 0; x < sizeof(int64_t); ++x) {
    if(_socket->eof()) {
      err_msg = "Not full int64_t supplied";
      return -1;
    }
    buf |= (int64_t)_socket->next() << x*8;
  }
  if(_socket->eof()) {
    err_msg = "No full int64_t supplied";
    return -1;
  }

  return 0;
}

int requestBase::_customAppend(std::string &buf, int max) {
	int pos = 0;

	// Make sure eof has not been reached yet.
	char ch = _socket->next();
	if(_socket->eof())
		return 0;

	buf.push_back(ch);

	while((ch = _socket->next())) {
		if(pos++ >= max) {
			err_msg = "string to large";
			return -1;
		}

		if(_socket->eof()) {
			err_msg = "reached eof before \\0";
			return -1;
		}
		buf.push_back(ch);
	}

	return 0;
}

int requestAuthenticate::insert(ioFile *_socket) {
  DEBUG_LOG("Parse authenticate request");

  this->_socket = _socket;

  if(_customAppend(username, MAX_USERNAME) ||
     _customAppend(password,  MAX_PASSWORD))
  {
    return -1;
  }

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

		if(_socket->eof())
			break;

		keywords.push_back(std::move(tmp));
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

int requestAuthenticate::exec() {
  DEBUG_LOG("Execute authenticate request");

  Database db;
  Authenticater auth(&db);

  std::string token = auth.validate(username, password);

  if(!auth.err_msg) {
    // Return token

    DEBUG_LOG(token.c_str());
    return 0;
  }

  DEBUG_LOG(username.c_str());
  DEBUG_LOG(password.c_str());

  err_msg = auth.err_msg;
  return -1;
}

int requestSearch::exec() {
  DEBUG_LOG("Execute search request");

  Database db;
  Authenticater auth(&db);

  int64_t id = auth.validate(token);

  if(!auth.err_msg) {
    std::vector<meta_doc> result = db.search(this);
 
    return 0;
  }

  err_msg = "user validation failed.";

  return -1;
}

int requestDownload::exec() {
  DEBUG_LOG("Excecute download requests");

  Database db;
  Authenticater auth(&db);

  int64_t id = auth.validate(token);

  if(!auth.err_msg) {
    meta_doc result = db.getFile(this);
   
    DEBUG_LOG(result.company.c_str());
    DEBUG_LOG(result.id);

    return 0;
  }

  err_msg = "user validation failed.";

	return -1;
}

int requestUpload::exec() {
  DEBUG_LOG("Execute search request");

  Database db;
  Authenticater auth(&db);

  int64_t id = auth.validate(token);

  if(!auth.err_msg) {
    db.newDocument(this);

    return 0;
  }

  err_msg = "user validation failed.";

	return -1;
}
