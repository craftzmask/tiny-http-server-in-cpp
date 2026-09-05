#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

struct sockaddr_in server_addr;
struct sockaddr_in client_addr;

int create_server();
int create_socket(int server_fd);

int main(int argc, char **argv) {
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  const int server_fd = create_server();
  if (server_fd < 0) {
    return -1;
  }

  const int socket_fd = create_socket(server_fd);
  if (socket_fd < 0) {
    return -1;
  }
  
  const std::string response = "HTTP/1.1 200 OK\r\n\r\n";
  write(socket_fd, response.data(), response.size());
  
  close(server_fd);

  return 0;
}

int create_server() {
  const int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    std::cerr << "Failed to create server socket\n";
    return -1;
  }
  
  // Since the tester restarts your program quite often, setting SO_REUSEADDR
  // ensures that we don't run into 'Address already in use' errors
  const int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
    std::cerr << "setsockopt failed\n";
    return -1;
  }
  
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(4221);
  
  if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
    std::cerr << "Failed to bind to port 4221\n";
    return -1;
  }
  
  const int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) != 0) {
    std::cerr << "listen failed\n";
    return -1;
  }

  return server_fd;
}

int create_socket(int server_fd) {
  struct sockaddr_in client_addr;
  const int client_addr_len = sizeof(client_addr);
  
  std::cout << "Waiting for a client to connect...\n";
  
  const int socket_fd = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);
  if (socket_fd < 0) {
    std::cerr << "Failed to create client socket\n";
    return -1;
  }

  std::cout << "Client connected\n";

  return socket_fd;
}