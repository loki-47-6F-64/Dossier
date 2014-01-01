#include <cstdio>
#include "main.h"
#include "proxy.h"

#include "file.h"
#include "database.h"

std::unique_ptr<requestBase> getRequest(sslFile *socket) {
  std::unique_ptr<requestBase> _req;
	switch (socket->next()) {
		case _req_code::SEARCH:
			_req = std::unique_ptr<requestSearch>(new requestSearch(socket));
			break;
    case _req_code::LIST_COMPANIES:
      _req = std::unique_ptr<requestListCompanies>(new requestListCompanies(socket));
      break;
		case _req_code::DOWNLOAD:
			_req = std::unique_ptr<requestDownload>(new requestDownload(socket));
			break;
		case _req_code::UPLOAD:
			_req = std::unique_ptr<requestUpload>(new requestUpload(socket));
			break;
    case _req_code::NEW_COMPANY:
      _req = std::unique_ptr<requestNewCompany>(new requestNewCompany(socket));
      break;
    case _req_code::REMOVE_COMPANY:
      _req = std::unique_ptr<requestRemoveCompany>(new requestRemoveCompany(socket));
      break;
    case _req_code::REMOVE_DOCUMENT:
      _req = std::unique_ptr<requestRemoveDocument>(new requestRemoveDocument(socket));
      break;
	}
  return _req;
}

int requestSearch::exec(Database &db) {
  DEBUG_LOG("Execute search request");

  if(load(
      company, MAX_COMPANY,
      year, month, day,
      keywords, MAX_PARAMETERS, MAX_KEYWORD))
  {
    _socket->clear();

    print(*_socket,
      _response::CORRUPT_REQUEST,
      err_msg);

    print(error, err_msg);
    return -1;
  }

  _socket->clear();
  std::vector<meta_doc> result = db.search(idUser, company, year, month, day);
 
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
  DEBUG_LOG("Excecute download request");

  if(load(idPage, company, MAX_COMPANY)) {
    _socket->clear();

    print(*_socket,
      _response::CORRUPT_REQUEST,
      err_msg);

    print(error, err_msg);
    return -1;
  }
  meta_doc result = db.getFile(idUser, idPage);
  
  DEBUG_LOG(result.company.c_str());
  DEBUG_LOG(result.id);

  return 0;
}

int requestUpload::exec(Database &db) {
  DEBUG_LOG("Execute upload request");

  if(load(company, MAX_COMPANY, size)) {
    _socket->clear();

    print(*_socket,
      _response::CORRUPT_REQUEST,
      err_msg);

    print(error, err_msg);
    return -1;
  }
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
  int err = _socket->copy(out, size);

  if(err == FileErr::SYS_ERROR) {
    strerror_r(errno, err_buf, MAX_ERROR_BUFFER);
    print(error, "Uploading failed: ", err_buf);

    print(*_socket, 
      _response::INTERNAL_ERROR,
      "Uploading failed: ", err_buf);

    return -1;
  }
  if(err == FileErr::TIMEOUT) {
    print(error, "Uploading failed: Timeout");
    print(*_socket,
      _response::INTERNAL_ERROR,
      "Uploading failed: Timeout");

    return -1;
  }

  print(*_socket, _response::OK);

  return 0;
}

int requestNewCompany::exec(Database &db) {
  DEBUG_LOG("Execute new Company request");

  if(load(name, MAX_COMPANY)) {
    print(*_socket,
      _response::CORRUPT_REQUEST,
      err_msg);

    print(error, err_msg);
    return -1;
  }

  if(db.newCompany(name, idUser)) {
    print(*_socket,
      _response::INTERNAL_ERROR,
      db.err_msg);

    print(error, db.err_msg);
    return -1;
  }

  print(*_socket, _response::OK);

  return 0;
}

int requestListCompanies::exec(Database &db) {
  DEBUG_LOG("Execute list Company request");

  std::vector<std::string> companies = db.listCompany(idUser);

  if(db.err_msg) {
    print(error, db.err_msg);
    print(*_socket,
      _response::INTERNAL_ERROR,
      db.err_msg);

    return -1;
  }

  _socket->clear().append(_response::OK);

  for(auto& company : companies) {
    _socket->
      append(company).append('\0');
  }

  _socket->
    append('\0').out();

  return 0;
}

int requestRemoveCompany::exec(Database &db) {
  DEBUG_LOG("Execute remove company");
  if(load(company, MAX_COMPANY)) {
    print(*_socket,
      _response::CORRUPT_REQUEST,
      err_msg);

    print(error, err_msg);
    return -1;
  }

  if(db.removeCompany(company, idUser)) {
    print(*_socket,
      _response::INTERNAL_ERROR,
      db.err_msg);

    print(error, db.err_msg);
    return -1;
  }

  print(*_socket, _response::OK);
  return 0;
}

int requestRemoveDocument::exec(Database &db) {
  DEBUG_LOG("Execute remove document");

  if(load(idPage)) {
    print(*_socket,
      _response::INTERNAL_ERROR,
      db.err_msg);

    print(error, db.err_msg);
    return -1;
  }

  DEBUG_LOG("Removing idPage:", idPage);
  if(db.removeDocument(idPage, idUser)) {
    print(*_socket,
      _response::INTERNAL_ERROR,
      db.err_msg);

    print(error, db.err_msg);
    return -1;
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

  if(std::remove(path.c_str())) {
    strerror_r(errno, err_buf, MAX_ERROR_BUFFER);

    print(*_socket, _response::INTERNAL_ERROR, err_buf);
    print(error, err_buf);

    return -1;
  }

  print(*_socket, _response::OK);
  return 0;
}
