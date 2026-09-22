#include "request.h"
#include "response.h"
#include "utils.h"
#include <iostream>
#include <string>
#include <thread>

void handle_client(int socket_fd);

int main(int argc, char **argv) {
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  const int server_fd = create_server();
  if (server_fd < 0) {
    return -1;
  }

  while (true) {
    const int socket_fd = create_socket(server_fd);
    if (socket_fd < 0) {
      return -1;
    }

    std::jthread client_thread(handle_client, socket_fd);
    client_thread.detach();
  }

  close(server_fd);

  return 0;
}

void handle_client(int socket_fd) {
  char buffer[1024];
  const int byteRead = read(socket_fd, buffer, 1024);
  std::string request_string(buffer, byteRead);

  Request request;
  parseRequest(request, request_string);

  std::string response;
  const std::string path = request.path;
  if (path == "/") {
    response = "HTTP/1.1 200 OK\r\n\r\n";
  } else if (path.contains("/echo/")) {
    const std::string s = "/echo/";
    size_t index = path.find(s);
    const std::string content = path.substr(index + s.size());
    response = std::format("HTTP/1.1 200 OK\r\nContent-Type: "
                           "text/plain\r\nContent-Length: {}\r\n\r\n{}",
                           content.size(), content);
  } else if (path.contains("/user-agent")) {
    const std::string content = request.headers["User-Agent"];
    response = std::format("HTTP/1.1 200 OK\r\nContent-Type: "
                           "text/plain\r\nContent-Length: {}\r\n\r\n{}",
                           content.size(), content);
  } else {
    response = "HTTP/1.1 404 Not Found\r\n\r\n";
  }

  write(socket_fd, response.data(), response.size());
  close(socket_fd);
}
