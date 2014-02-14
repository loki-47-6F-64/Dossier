#include "server/main.h"
#include "server/proxy.h"
#include "proc.h"
#include "database/database.h"

#define CHAR(x) static_cast<char>(x)

namespace dossier {
namespace server {
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
    if(req_errno == FileErr::TIMEOUT) {
      print(error, "Timeout occurred");
      return -1;
    } 
    if(req_errno == FileErr::STREAM_ERR) {
      ssl_print_err(error);
      return -1;
    }
    print(*_socket,
      CHAR(_response::CORRUPT_REQUEST),
      err_msg);

    print(error, err_msg);
    return -1;
  }

  _socket->clear();
  std::vector<meta_doc> result = db.search(idUser, company, year, month, day, keywords);
 
  _socket->append(CHAR(_response::OK));
  for(auto& doc : result) {
    _socket->
       append(doc.created).append('\0')
      .append(doc.company).append('\0')
      .append(std::to_string(doc.id)).append('\0');

    if(doc.preview.empty()) {
      doc.preview = "--";
    }

    _socket->append(doc.preview).append('\0');
  }

  int err = _socket->append('\0').out();

  if(err == FileErr::TIMEOUT) {
    print(error, "socket Timeout");
    return -1;
  }
  else if(err) {
    print(error, sys_err(), '\n');
    ssl_print_err(error);
    return -1;
  }
 
  return 0;
}

std::string getDocPath(int64_t idUser, int64_t idPage) {
  std::string path = config::storage.root;
  if(path.back() != '/') {
    path += '/';
  }
  path.append(std::to_string(idUser));
  path += '_';

  path.append(std::to_string(idPage));
  path += ".pdf";

  return path;
}

int requestDownload::exec(Database &db) {
  DEBUG_LOG("Excecute download request");

  if(load(idPage)) {
    if(req_errno == FileErr::TIMEOUT) {
      print(error, "Timeout occurred");
      return -1;
    } 
    if(req_errno == FileErr::STREAM_ERR) {
      ssl_print_err(error);
      return -1;
    }

    print(*_socket,
      CHAR(_response::CORRUPT_REQUEST),
      err_msg);

    print(error, err_msg);
    return -1;
  }

  std::string path = getDocPath(idUser, idPage);

  ioFile page(1024);
  page.access(path, fileStreamRead);

  print(*_socket, CHAR(_response::OK));
  int err = page.copy(*_socket);

  if(err == FileErr::STREAM_ERR) {
    const char *err = sys_err();
    print(error, "Downloading failed: ", err);

    print(*_socket,
      CHAR(_response::INTERNAL_ERROR),
      "Downloading failed: ", err);

    return -1;
  }
  if(err == FileErr::TIMEOUT) {
    print(error, "Downloading failed: Timeout");
    print(*_socket,
      CHAR(_response::INTERNAL_ERROR),
      "Downloading failed: Timeout");

    return -1;
  }
  if(err == FileErr::COPY_STREAM_ERR) {
    const char *err = sys_err();
    print(error, "Downloading failed: ", err);

    print(*_socket,
      CHAR(_response::INTERNAL_ERROR),
      "Downloading failed: ", err);

    return -1;
  }

  return 0;
}

int requestUpload::exec(Database &db) {
  DEBUG_LOG("Execute upload request");

  if(load(company, MAX_COMPANY, size)) {
    if(req_errno == FileErr::TIMEOUT) {
      print(error, "Timeout occurred");
      return -1;
    }
    if(req_errno == FileErr::STREAM_ERR) {
      ssl_print_err(error);
      return -1;
    }

    print(*_socket,
      CHAR(_response::CORRUPT_REQUEST),
      err_msg);

    print(error, err_msg);
    return -1;
  }
  int64_t idPage = db.newDocument(idUser, company);

  if(db.err_msg) {
    err_msg = db.err_msg;

    print(error, db.err_msg);
    print(*_socket,
      CHAR(_response::INTERNAL_ERROR),
      db.err_msg);

    return -1;
  }

  std::string path = getDocPath(idUser, idPage);
  ioFile out(1024);
  out.access(path, fileStreamWrite);

  DEBUG_LOG("Copying file: ", path, " of size: ", size);

  // Attempt to copy file from client
  int err = _socket->copy(out, size);

  if(err == FileErr::STREAM_ERR) {
    const char *err = sys_err();
    print(error, "Uploading failed: ", err);

    print(*_socket, 
      CHAR(_response::INTERNAL_ERROR),
      "Uploading failed: ", err);

    return -1;
  }

  if(err == FileErr::TIMEOUT) {
    print(error, "Downloading failed: Timeout");
    print(*_socket,
      CHAR(_response::INTERNAL_ERROR),
      "Downloading failed: Timeout");
    
    return -1;
  }   
  if(err == FileErr::COPY_STREAM_ERR) { 
    const char *err = sys_err();
    print(error, "Downloading failed: ", err);

    print(*_socket,
      CHAR(_response::INTERNAL_ERROR),
      "Downloading failed: ", err);

    return -1;
  }

  print(*_socket, CHAR(_response::OK));

  _socket->seal();
  const char *args[] = {
    "pdftotext",
    path.c_str(), "-",
    nullptr
  };

  Proc proc = proc_open(*args, args, pipeType::READ);
  if(proc.pid >= 0) {

    std::string content;
    out.access(path, fileStreamRead);

    if(proc.fpipe.eachByte([&](unsigned char ch) {
      content.push_back(ch);
      return 0;
    }))
    {
      print(error, sys_err());
      return -1;
    }

    if(db.setDocContent(idPage, content)) {
      print(error, db.err_msg);
      return -1;
    }
  }

  return 0;
}

int requestNewCompany::exec(Database &db) {
  DEBUG_LOG("Execute new Company request");

  if(load(name, MAX_COMPANY)) {
    if(req_errno == FileErr::TIMEOUT) {
      print(error, "Timeout occurred");
      return -1;
    }
    if(req_errno == FileErr::STREAM_ERR) {
      ssl_print_err(error);
      return -1;
    }

    print(*_socket,
      CHAR(_response::CORRUPT_REQUEST),
      err_msg);

    print(error, err_msg);
    return -1;
  }

  if(db.newCompany(name, idUser)) {
    print(*_socket,
      CHAR(_response::INTERNAL_ERROR),
      db.err_msg);

    print(error, db.err_msg);
    return -1;
  }

  print(*_socket, CHAR(_response::OK));

  return 0;
}

int requestListCompanies::exec(Database &db) {
  DEBUG_LOG("Execute list Company request");

  std::vector<std::string> companies = db.listCompany(idUser);

  if(db.err_msg) {
    print(error, db.err_msg);
    print(*_socket,
      CHAR(_response::INTERNAL_ERROR),
      db.err_msg);

    return -1;
  }

  _socket->clear().append(CHAR(_response::OK));

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
    if(req_errno == FileErr::TIMEOUT) {
      print(error, "Timeout occurred");
      return -1;
    }
    if(req_errno == FileErr::STREAM_ERR) {
      ssl_print_err(error);
      return -1;
    }

    print(*_socket,
      CHAR(_response::CORRUPT_REQUEST),
      err_msg);

    print(error, err_msg);
    return -1;
  }

  if(db.removeCompany(company, idUser)) {
    print(*_socket,
      CHAR(_response::INTERNAL_ERROR),
      db.err_msg);

    print(error, db.err_msg);
    return -1;
  }

  print(*_socket, CHAR(_response::OK));
  return 0;
}

int requestRemoveDocument::exec(Database &db) {
  DEBUG_LOG("Execute remove document");

  if(load(idPage)) {
    if(req_errno == FileErr::TIMEOUT) {
      print(error, "Timeout occurred");
      return -1;
    }
    if(req_errno == FileErr::STREAM_ERR) {
      ssl_print_err(error);
      return -1;
    }

    print(*_socket,
      CHAR(_response::INTERNAL_ERROR),
      db.err_msg);

    print(error, db.err_msg);
    return -1;
  }

  DEBUG_LOG("Removing idPage:", idPage);
  if(db.removeDocument(idPage, idUser)) {
    print(*_socket,
      CHAR(_response::INTERNAL_ERROR),
      db.err_msg);

    print(error, db.err_msg);
    return -1;
  }

  if(std::remove(getDocPath(idUser, idPage).c_str())) {
    const char *err = sys_err();

    print(*_socket, CHAR(_response::INTERNAL_ERROR), err);
    print(error, err);

    return -1;
  }

  print(*_socket, CHAR(_response::OK));
  return 0;
}

void start_server(Server &s, uint16_t port) {
  if(s.addListener(port, 20, [&](Client &&client) {
    Database db;

    if(db.err_msg) {
      print(*client.socket,
        CHAR(_response::INTERNAL_ERROR),
        "Could not connect to database.");
      print(error, db.err_msg);
      return;
    }

    std::string cn = getCN(client.ssl);
    int64_t idUser = db.validateUser(cn);

    DEBUG_LOG("Accepted client: ", cn);
    if(db.err_msg) {
      print(*client.socket,
        CHAR(_response::INTERNAL_ERROR),
        "User ", cn, " not found.");

      print(error, "User ", cn, " not found.");
      return;
    }

    std::unique_ptr<requestBase> req = getRequest(client.socket.get());

    std::unique_ptr<int> s;
    if(req.get() == nullptr) {
      print(warning, "Unknown request.");
      return;
    }

    req->idUser = idUser;
    req->exec(db);
  }
  ) < 0) {
    print(error, "Can't set listener: ", sys_err());

    return;
  }
  s();
}
};
};
