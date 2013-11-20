#include "main.h"
#include "proxy.h"
#include "database.h"

// Limit file _cache buffer size
const int REQ_BUFFER_SIZE = 1;

Proxy::Proxy(ioFile *socket) : _socket(socket) {}

int Proxy::readRequest() {
	if(_socket->load(REQ_BUFFER_SIZE) < 0) {
		FATAL_ERROR("Parse request", errno);
	}

	switch (_socket->next()) {
		case _req_code::SEARCH:
			_req = std::unique_ptr<requestSearch>(new requestSearch());
			break;
		case _req_code::DOWNLOAD:
			_req = std::unique_ptr<requestDownload>(new requestDownload());
			break;
		case _req_code::UPLOAD:
			_req = std::unique_ptr<requestUpload>(new requestUpload());
			break;
		default:
			return -1;
	}
	return _req->insert(_socket);
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

int requestSearch::insert(ioFile *_socket) {
	DEBUG_LOG("Parse search request");

	this->_socket = _socket;

	if(_customAppend(username, MAX_USERNAME) ||
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

	if(_customAppend(username, MAX_USERNAME) ||
     _customAppend(company,  MAX_COMPANY))
  {
    return -1;
  }

	return 0;
}

int requestUpload::insert(ioFile *_socket) {
	DEBUG_LOG("Parse upload request");

	this->_socket = _socket;

	if(_customAppend(username, MAX_USERNAME) ||
     _customAppend(company,  MAX_COMPANY))
  {
    return -1;
  }

  return 0;
}

int requestSearch::exec() {
  Database db;
  if(!db.validateUser(username.c_str(), "")) {
    std::vector<meta_doc> result = db.search(this);

    DEBUG_LOG(result[0].company.c_str());
    return 0;
  }

  err_msg = "user validation failed.";

  return -1;
}

int requestDownload::exec() {
	return 0;
}

int requestUpload::exec() {
	return 0;
}
