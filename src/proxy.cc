#include "proxy.h"

ProxyServer::ProxyServer(std::string proxy_port, std::string backend_host,
                         std::string backend_port) {
  proxy_port_ = std::stoi(proxy_port);
  backend_host_ = backend_host;
  backend_port_ = std::stoi(backend_port);
}

ProxyServer::ProxyServer(const ProxyServer& other)
    : proxy_port_(other.proxy_port_),
      backend_host_(other.backend_host_),
      backend_port_(other.backend_port_) {}

ProxyServer::ProxyServer(ProxyServer&& other) noexcept {
  if (this != &other) {
    proxy_port_ = other.proxy_port_;
    backend_host_ = other.backend_host_;
    backend_port_ = other.backend_port_;
    other.proxy_port_ = 0;
    other.backend_host_ = "";
    other.backend_port_ = 0;
  }
}

ProxyServer& ProxyServer::operator=(const ProxyServer& other) {
  proxy_port_ = other.proxy_port_;
  backend_host_ = other.backend_host_;
  backend_port_ = other.backend_port_;
  return *this;
}

ProxyServer& ProxyServer::operator=(ProxyServer&& other) noexcept {
  if (this != &other) {
    proxy_port_ = other.proxy_port_;
    backend_host_ = other.backend_host_;
    backend_port_ = other.backend_port_;
    close(proxy_socket_);
  }
  return *this;
}

ProxyServer::~ProxyServer() {
  proxy_port_ = 0;
  backend_host_ = "";
  backend_port_ = 0;
  close(proxy_socket_);
}

void ProxyServer::StopServerAsync() {
  // Use std::async for asynchronously starting a thread waiting for the server
  // stop command
  stop_server_future_ = std::async(std::launch::async, [this]() {
    std::string userInput;
    // Prompt user input, waiting for the "\\exit" command to stop the server
    while (userInput != "\\exit") {
      std::cout << "Enter '\\exit' to stop the server:\n";
      std::cin >> userInput;
    }
    // Set the server stop flag after receiving the command from the user
    server_state_.stop_server = true;
    std::cout << "Server is stopping..." << std::endl;
  });
}

void ProxyServer::StartServer() {
  CreateAndBindProxySocket();
  ConnectToBackendSocket();

  std::ofstream log_stream(kLogFileName, std::ios::app);
  do {
    HandleClient(log_stream);
  } while (!server_state_.stop_server);
  stop_server_future_.wait();

  log_stream.close();
}

void ProxyServer::CreateAndBindProxySocket() {
  // Create a socket for the proxy server
  proxy_socket_ = socket(AF_INET, SOCK_STREAM, 0);
  if (proxy_socket_ == -1)
    throw std::runtime_error("Error creating proxy socket");

  // Configure the proxy server's address information
  sockaddr_in proxy_addr{};
  proxy_addr.sin_family = AF_INET;
  proxy_addr.sin_addr.s_addr = INADDR_ANY;
  proxy_addr.sin_port = htons(proxy_port_);

  // Bind the proxy socket to the specified address
  if (bind(proxy_socket_, reinterpret_cast<sockaddr*>(&proxy_addr),
           sizeof(proxy_addr)) == -1) {
    close(proxy_socket_);
    throw std::runtime_error("Error binding proxy socket");
  }

  // Set the proxy socket to listen for incoming connections
  if (listen(proxy_socket_, kMaxConnections) == -1) {
    throw std::runtime_error("Error listening on proxy socket");
    close(proxy_socket_);
  }
  // Print a message indicating that the proxy server has started
  std::cout << "Proxy server started on port " << proxy_port_ << std::endl;
}

void ProxyServer::ConnectToBackendSocket() {
  // Create a socket for the backend connection
  int backend_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (backend_socket == -1) {
    close(proxy_socket_);
    throw std::runtime_error("Error creating backend socket");
  }

  // Configure the backend server's address information
  sockaddr_in backend_addr{};
  backend_addr.sin_family = AF_INET;
  backend_addr.sin_addr.s_addr = inet_addr(backend_host_.c_str());
  backend_addr.sin_port = htons(backend_port_);

  // Display a message indicating the attempt to connect to the backend
  std::cout << "Connecting to backend: " << backend_host_ << ":"
            << backend_port_ << std::endl;

  // Connect the backend socket to the specified address
  if (connect(backend_socket, reinterpret_cast<sockaddr*>(&backend_addr),
              sizeof(backend_addr)) == -1) {
    std::cerr << "Error code: " << errno << std::endl;
    std::cerr << "Error message: " << strerror(errno) << std::endl;
    close(proxy_socket_);
    close(backend_socket);
    throw std::runtime_error("Error connecting to backend");
  }
}

void ProxyServer::HandleClient(std::ofstream& log_stream) {
  sockaddr_in client_addr{};
  socklen_t client_addr_len = sizeof(client_addr);
  int client_socket =
      accept(proxy_socket_, reinterpret_cast<sockaddr*>(&client_addr),
             &client_addr_len);
  if (client_socket == -1) {
    throw std::runtime_error("Error accepting client connection");
    return;
  }

  std::cout << "Accepted connection from " << inet_ntoa(client_addr.sin_addr)
            << ":" << ntohs(client_addr.sin_port) << std::endl;

  if (fork() == 0) {
    close(proxy_socket_);
    HandleClientLogic(client_socket, log_stream);
    std::cout << "Connection closed" << std::endl;
    exit(0);
  }
}

void ProxyServer::HandleClientLogic(int client_socket,
                                    std::ofstream& log_stream) {
  // Buffer to hold data from client and backend
  char buffer[kBufferSize];
  memset(buffer, 0, sizeof(buffer));

  // Create a socket for the backend connection
  int backend_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (backend_socket == -1) {
    close(client_socket);
    throw std::runtime_error("Error creating backend socket");
  }

  // Configure the backend server's address information
  sockaddr_in backend_addr{};
  backend_addr.sin_family = AF_INET;
  backend_addr.sin_addr.s_addr = inet_addr(backend_host_.c_str());
  backend_addr.sin_port = htons(backend_port_);

  // Display a message indicating the attempt to connect to the backend
  std::cout << "Connecting to backend: " << backend_host_ << ":"
            << backend_port_ << std::endl;

  // Connect the backend socket to the specified address
  if (connect(backend_socket, reinterpret_cast<sockaddr*>(&backend_addr),
              sizeof(backend_addr)) == -1) {
    std::cerr << "Error code: " << errno << std::endl;
    std::cerr << "Error message: " << strerror(errno) << std::endl;
    close(client_socket);
    close(backend_socket);
    throw std::runtime_error("Error connecting to backend");
  }

  fd_set read_fds;
  do {
    // Set up file descriptors for select
    FD_ZERO(&read_fds);
    FD_SET(client_socket, &read_fds);
    FD_SET(backend_socket, &read_fds);

    // Determine the maximum file descriptor for select
    int max_fd = std::max(client_socket, backend_socket);
    if (select(max_fd + 1, &read_fds, nullptr, nullptr, nullptr) == -1) {
      perror("Error in select");
      break;
    }

    // Check if there is data to read from the client socket
    if (FD_ISSET(client_socket, &read_fds)) {
      ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
      if (bytes_received <= 0) {
        std::cerr << "Error receiving data from client" << std::endl;
        break;
      }

      // Get information about the client
      struct sockaddr_in client_addr;
      socklen_t client_addr_len = sizeof(client_addr);
      getpeername(client_socket,
                  reinterpret_cast<struct sockaddr*>(&client_addr),
                  &client_addr_len);

      char client_ip[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);

      // Process and log the received SQL query
      std::string sql_query(buffer, bytes_received);
      sql_query.erase(std::remove_if(sql_query.begin(), sql_query.end(),
                                     [](unsigned char c) {
                                       return !std::isprint(c) || c == '\n';
                                     }),
                      sql_query.end());

      if (sql_query.find("Q(") == 0) sql_query.erase(0, 2);
      if (sql_query.find("Q") == 0) sql_query.erase(0, 1);
      if (sql_query.find("SELECT") == 1) sql_query.erase(0, 1);

      if (sql_query.find("SELECT") != std::string::npos ||
          sql_query.find("INSERT") != std::string::npos ||
          sql_query.find("UPDATE") != std::string::npos ||
          sql_query.find("DELETE") != std::string::npos) {
        auto now = std::chrono::system_clock::to_time_t(
            std::chrono::system_clock::now());
        std::cout << "Received from client at " << std::ctime(&now)
                  << "From: " << client_ip << ":" << ntohs(client_addr.sin_port)
                  << "\nSQL-query:\n"
                  << sql_query << "\n\n";
        log_stream << "Received from client at " << std::ctime(&now)
                   << "From: " << client_ip << ":"
                   << ntohs(client_addr.sin_port) << "\nSQL-query:\n"
                   << sql_query << "\n\n";
        log_stream.flush();
      }

      // Forward the data to the backend
      send(backend_socket, buffer, static_cast<size_t>(bytes_received), 0);
      memset(buffer, 0, sizeof(buffer));
    }

    // Check if there is data to read from the backend socket
    if (FD_ISSET(backend_socket, &read_fds)) {
      ssize_t bytes_received = recv(backend_socket, buffer, sizeof(buffer), 0);
      if (bytes_received <= 0) {
        std::cerr << "Error receiving data from backend" << std::endl;
        break;
      }

      // Forward the data to the client
      send(client_socket, buffer, static_cast<size_t>(bytes_received), 0);
      memset(buffer, 0, sizeof(buffer));
    }
  } while (!server_state_.stop_server);

  // Close sockets when the loop exits
  close(client_socket);
  close(backend_socket);
}
