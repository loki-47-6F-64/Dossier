#include <gtest/gtest.h>

#include <future>
#include <vector>

#include "server/server.h"
#include "server/proxy.h"
#include "server/main.h"
#include "database/database.h"
#include "err.h"
#include "proc.h"
#include "thread_t.h"

#include "client/args.h"
namespace dossier {


#define TEST_DIR "./src/test/"
#define HOST "localhost", "8088"

std::vector<client::s_args> _s_args {
  { "","","", "ING", {}, HOST }
};
std::vector<client::up_args> _up_args {
  { TEST_DIR "test_up.pdf", "ING", HOST }
};
std::vector<client::down_args> _down_args {
  { "4", TEST_DIR "test_down.pdf", HOST }
};
std::vector<client::del_args> _del_args {
  { "4", HOST }
};
std::vector<client::list_args> _list_args {
  { HOST }
};
std::vector<client::mod_company_args> _mod_company_args {
  { "ING", HOST }
};

TEST(ExternalProcess, TestExternalProcess) {
  const char *argv[] {
    "cat",
    TEST_DIR "test_proc.txt",
    nullptr
  };
  Proc proc = proc_open(*argv, argv, pipeType::READ);

  ASSERT_GT(proc.pid, 0);

  std::string file, fpipe;
  proc.fpipe.eachByte([&](unsigned char ch) {
    fpipe.push_back(ch);
    return 0;
  });

  ioFile f { 1 };
  ASSERT_EQ(f.access(TEST_DIR "test_proc.txt", fileStreamRead), 0);

  f.eachByte([&](unsigned char ch) {
    file.push_back(ch);
    return 0;
  });

  EXPECT_EQ(file, fpipe);
}

// Test sql_connect and prepare the test database at the same time.
TEST(mysql, TestMysql) {
  SqlConnect sql_client;
 
  ASSERT_NE(sql_client.open(
    config::database.host.c_str(),
    config::database.user.c_str(),
    config::database.password.c_str()
  ), nullptr) << "Database error: " << sql_client.error();

  ioFile f { 1 };
  ASSERT_EQ(f.access(TEST_DIR "test.sql", fileStreamRead), 0);

  std::string sql;
  std::vector<std::string> statements;
  f.eachByte([&](unsigned char ch) {
    if(ch == '\n') return 0;
    if(ch == ';') {
      statements.push_back(std::move(sql));
      return 0;
    }
    sql.push_back(ch);
    return 0;
  });

  for(auto &statement : statements) {
    EXPECT_EQ(sql_client.query(std::move(statement)), 0) << "Database error: " << sql_client.error();
  }
}

TEST(server, TestServer) {
  ASSERT_EQ(0,
    server::Server::init(
      config::server.certPath,
      config::server.keyPath
    )
  );

  server::Server test_server;
  thread_t t([&]() {
      server::start_server(test_server, config::server.port);
  });

  std::string caPath   = TEST_DIR "test_server.crt";
  std::string certPath = TEST_DIR "test_client.crt";
  std::string keyPath  = TEST_DIR "test_client.key";

  Context client_ctx = init_ctx_client(caPath, certPath, keyPath);
  ioFile fout(1024, -1, dup(1));

  ASSERT_NE(nullptr, client_ctx.get()) << ssl_err();

  sslFile file = ssl_connect(
    client_ctx, "localhost", std::to_string(config::server.port).c_str()
  );

  ASSERT_EQ(true, file.is_open()) << get_current_err();
  file.seal();

  for(auto &args : _s_args) {
    EXPECT_EQ(0, client::perform_search(client_ctx, args))  << "Search failed: " << get_current_err();
  }
  for(auto &args : _up_args) {
    EXPECT_EQ(0, client::perform_upload(client_ctx, args)) << "Upload failed: " << get_current_err();
  }
  for(auto &args : _down_args) {
    EXPECT_EQ(0, client::perform_download(client_ctx, args)) << "Download failed: " << get_current_err();
  }
  for(auto &args : _del_args) {
    EXPECT_EQ(0, client::perform_remove(client_ctx, args)) << "Remove failed: " << get_current_err();
  }
  for(auto &args : _list_args) {
    EXPECT_EQ(0, client::perform_list_company(client_ctx, args)) << "List company failed: " << get_current_err();
  }
  for(auto &args : _mod_company_args) {
    EXPECT_EQ(0, client::perform_add_company(client_ctx, args)) << "Add company failed: " << get_current_err();
  }
  for(auto &args : _mod_company_args) {
    EXPECT_EQ(0, client::perform_del_company(client_ctx, args)) << "Delete company failed: " << get_current_err();
  }

  test_server.stop();
  t.join();
}

};
int main(int argc, char *argv[]) {
  dossier::config::file(TEST_DIR "dossier.conf");
  log_open(dossier::config::storage.log.c_str());

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();  
}
