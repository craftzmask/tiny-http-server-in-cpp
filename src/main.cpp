#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unordered_map>
#include <ranges>
#include <string_view>
#include <vector>

struct Request {
  std::string method;
  std::string path;
  std::string http_version;
  std::unordered_map<std::string, std::string> headers;
};

struct Response {
  std::string http_version;
  std::string status_code;
  std::string status_msg;
  std::unordered_map<std::string, std::string> headers;
};

struct sockaddr_in server_addr;
struct sockaddr_in client_addr;

int create_server();
int create_socket(int server_fd);

void parseRequest(Request& request, std::string_view requestString);

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

  char buffer[1024];
  const int byteRead = read(socket_fd, buffer, 1024);
  std::string requestString(buffer, byteRead);

  Request request;
  parseRequest(request, requestString);

  std::string response;
  const std::string path = request.path;
  if (path == "/") {
    response = "HTTP/1.1 200 OK\r\n\r\n";
  } else if (path.contains("/echo/")) {
    const std::string s = "/echo/";
    size_t index = path.find(s);
    const std::string content = path.substr(index + s.size());
    response = std::format("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: {}\r\n\r\n{}", content.size(), content);
  } else if (path.contains("/user-agent")) {
    const std::string content = request.headers["User-Agent"];
    response = std::format("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: {}\r\n\r\n{}", content.size(), content);
  } else {
    response = "HTTP/1.1 404 Not Found\r\n\r\n";
  }
  
  write(socket_fd, response.data(), response.size());
  
  close(server_fd);

  return 0;
}

auto split_string(std::string_view text, std::string_view delimiter) {
    return std::views::split(text, delimiter)
         | std::ranges::to<std::vector<std::string>>();
}

void parseRequest(Request& request, std::string_view requestString) {
  auto lines = split_string(requestString, "\r\n");
  auto request_line_parts = split_string(lines[0], " ");
  request.method = request_line_parts[0];
  request.path = request_line_parts[1];
  request.http_version = request_line_parts[2];

  for (int i = 1; i < lines.size(); i++) {
    auto parts = split_string(lines[i], ": ");
    if (parts.size() == 2) {
      request.headers[parts[0]] = parts[1];
    }
  }
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