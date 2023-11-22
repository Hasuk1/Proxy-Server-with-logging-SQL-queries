#include <thread>

#include "proxy.h"

int main(int argc, char* argv[]) {
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0]
              << " <proxy_port> <backend_host> <backend_port>" << std::endl;
    return 1;
  }

  ProxyServer my_server(argv[1], argv[2], argv[3]);
  std::thread asyncThread(&ProxyServer::StopServerAsync, &my_server);
  my_server.StartServer();

  asyncThread.join();
  return 0;
}
