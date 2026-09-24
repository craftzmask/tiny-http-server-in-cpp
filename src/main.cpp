#include "request.h"
#include "response.h"
#include "utils.h"
#include <iostream>
#include <string>
#include <thread>
#include <sys/socket.h>
#include <unistd.h>

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
  const int byte_read = read(socket_fd, buffer, 1024);
  std::string request_string(buffer, byte_read);

  Request request;
  parseRequest(request, request_string);

  Response response;
  response.socket_fd = socket_fd;

  const std::string path = request.path;
  if (path == "/") {
    response.send();
  } else if (path.contains("/echo/")) {
    const std::string s = "/echo/";
    size_t index = path.find(s);
    const std::string content = path.substr(index + s.size());
    response.send(content);
  } else if (path.contains("/user-agent")) {
    const std::string content = request.headers["User-Agent"];
    response.send(content);
  } else {
    response.status_code = "404";
    response.status_msg = "Not Found";
    response.send();
  }

  close(socket_fd);
}
