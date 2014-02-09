#include <gtest/gtest.h>

#include <thread>
#include <future>

#include "server.h"
#include "proxy.h"
#include "main.h"
#include "err.h"

TEST(server, TestServer) {
  ASSERT_EQ(
    Server::init(
      config::server.certPath,
      config::server.keyPath
    ),
    0
  );

  Server test_server;
  std::thread t([&]() {
    start_server(test_server, config::server.port);
  });

  std::string caPath   = "./src/test/test_server.crt";
  std::string certPath = "./src/test/test_client.crt";
  std::string keyPath  = "./src/test/test_client.key";

  Context client_ctx = init_ctx_client(caPath, certPath, keyPath);
  ioFile fout(1024, -1, dup(1));

  ASSERT_NE(client_ctx.get(), nullptr);

  sslFile file = ssl_connect(
    client_ctx, "localhost", std::to_string(config::server.port).c_str()
  );

  ASSERT_EQ(file.is_open(), true);

  test_server.stop();
  t.join();
}

int main(int argc, char *argv[]) {
  config::file("./src/test/dossier.conf");
  log_open(config::storage.log.c_str());

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();  
}
