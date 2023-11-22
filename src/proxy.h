#ifndef PROXY_H_
#define PROXY_H_

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <ctime>
#include <fstream>
#include <future>
#include <iostream>

#define kMaxConnections 100
#define kBufferSize 8192
#define kLogFileName "sql_queries.log"

class ProxyServer {
 public:
  ProxyServer()
      : proxy_port_(4444), backend_host_("localhost"), backend_port_(22) {}
  ProxyServer(std::string proxy_port, std::string backend_host,
              std::string backend_port);
  ProxyServer(const ProxyServer &other);
  ProxyServer(ProxyServer &&other) noexcept;
  ProxyServer &operator=(const ProxyServer &other);
  ProxyServer &operator=(ProxyServer &&other) noexcept;
  ~ProxyServer();

  void StopServerAsync();
  void StartServer();
  void StopServer();

 private:
  void CreateAndBindProxySocket();
  void ConnectToBackendSocket();
  void HandleClient(std::ofstream &log_stream);
  void HandleClientLogic(int client_socket, std::ofstream &log_stream);

  struct ServerState {
    bool stop_server = false;
  };

  ServerState server_state_;
  std::future<void> stop_server_future_;

  int proxy_port_;
  std::string backend_host_;
  int backend_port_;
  int proxy_socket_;
};

#endif  // PROXY_H_
