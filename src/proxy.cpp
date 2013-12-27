#include "main.h"
#include "proxy.h"

#include "file.h"
#include "database.h"

std::unique_ptr<requestBase> getRequest(sslFile *socket) {
  std::unique_ptr<requestBase> _req;
	switch (socket->next()) {
		case _req_code::SEARCH:
			_req = std::unique_ptr<requestSearch>(new requestSearch());
      _req->_socket = socket;
			break;
    case _req_code::LIST_COMPANIES:
      _req = std::unique_ptr<requestListCompanies>(new requestListCompanies());
      _req->_socket = socket;
      break;
		case _req_code::DOWNLOAD:
			_req = std::unique_ptr<requestDownload>(new requestDownload());
      _req->_socket = socket;
			break;
		case _req_code::UPLOAD:
			_req = std::unique_ptr<requestUpload>(new requestUpload());
      _req->_socket = socket;
			break;
    case _req_code::NEW_COMPANY:
      _req = std::unique_ptr<requestNewCompany>(new requestNewCompany());
      _req->_socket = socket;
      break;
	}
  return _req;
}

int requestSearch::exec(Database &db) {
  DEBUG_LOG("Execute search request");

  if(load
      //<std::vector<std::string>&, int, int>
      (company, MAX_COMPANY, keywords, MAX_PARAMETERS, MAX_KEYWORD)) {
    return -1;
  }

  _socket->clear();
  std::vector<meta_doc> result = db.search(idUser, company);
 
  _socket->append(_response::OK);
  for(auto& doc : result) {
    _socket->
       append(doc.created).append('\0')
      .append(doc.company).append('\0')
      .append(std::to_string(doc.id)).append('\0');
  }

  _socket->
    append('\0').out();
 
  return 0;
}

int requestDownload::exec(Database &db) {
  DEBUG_LOG("Excecute download requests");

  if(load(idPage, company, MAX_COMPANY)) {
    return -1;
  }
  meta_doc result = db.getFile(idUser, idPage);
  
  DEBUG_LOG(result.company.c_str());
  DEBUG_LOG(result.id);

  return 0;
}

int requestUpload::exec(Database &db) {
  DEBUG_LOG("Execute upload request");

  load(company, MAX_COMPANY, size);
  int64_t idPage = db.newDocument(idUser, company);

  if(db.err_msg) {
    _socket->clear();

    err_msg = db.err_msg;

    print(error, db.err_msg);
    print(*_socket,
      _response::INTERNAL_ERROR,
      db.err_msg);
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

  DEBUG_LOG("Copying file: ", path, " of size: ", size);

  // Attempt to copy file from client
  if(_socket->copy(out, size)) {
    _socket->clear();

    strerror_r(errno, err_buf, MAX_ERROR_BUFFER);
    print(error, "Uploading failed: ", err_buf);

    print(*_socket, 
      _response::INTERNAL_ERROR,
      "Uploading failed: ", err_buf);

    return -1;
  }

  _socket->
    clear().append(_response::OK).out();

  return 0;
}

int requestNewCompany::exec(Database &db) {
  DEBUG_LOG("Execute new Company request");

  if(load(name, MAX_COMPANY)) {
    return -1;
  }

  _socket->clear();
  if(db.newCompany(name, idUser)) {
    _socket->
      append(_response::INTERNAL_ERROR)
      .append(db.err_msg)
      .out();

    print(*_socket,
      _response::INTERNAL_ERROR,
      db.err_msg);

    print(error, db.err_msg);
    return -1;
  }

  _socket->
    append(_response::OK).out();

  return 0;
}

int requestListCompanies::exec(Database &db) {
  DEBUG_LOG("Execute list Company request");

  _socket->clear();
  std::vector<std::string> companies = db.listCompany(idUser);

  if(db.err_msg) {
    print(error, db.err_msg);
    print(*_socket,
      _response::INTERNAL_ERROR,
      db.err_msg);

    return -1;
  }

  _socket->append(_response::OK);

  for(auto& company : companies) {
    _socket->
      append(company).append('\0');
  }

  _socket->
    append('\0').out();

  return 0;
}
