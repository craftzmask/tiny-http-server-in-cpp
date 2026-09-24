#include "request.h"
#include "response.h"
#include "utils.h"
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <sys/socket.h>
#include <unistd.h>
#include <filesystem>
#include <string_view>

namespace fs = std::filesystem;

void handle_client(int socket_fd, const fs::path& dir_path);

int main(int argc, char **argv) {
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  fs::path dir_path;
  if (argc == 3 && std::string_view(argv[1]) == "--directory") {
    dir_path = argv[2];
    fs::create_directory(dir_path);
  }

  const int server_fd = create_server();
  if (server_fd < 0) {
    return -1;
  }

  while (true) {
    const int socket_fd = create_socket(server_fd);
    if (socket_fd < 0) {
      return -1;
    }

    std::jthread client_thread(handle_client, socket_fd, dir_path);
    client_thread.detach();
  }

  close(server_fd);

  return 0;
}

void handle_client(int socket_fd, const fs::path& dir_path) {
  char buffer[1024];
  const int byte_read = read(socket_fd, buffer, 1024);
  std::string request_string(buffer, byte_read);

  Request request;
  parseRequest(request, request_string);

  Response response;
  response.socket_fd = socket_fd;

  const std::string& path = request.path;
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
  } else if (path.contains("/files/")) {
    const std::string s = "/files/";
    size_t index = path.find(s);
    const std::string file_name = path.substr(index + s.size());
    fs::path full_path = dir_path / file_name;
    std::ifstream file(full_path);
    if (!file.is_open()) {
      response.status_code = "404";
      response.status_msg = "Not Found";
      response.send();
    } else {
      std::string content;
      std::getline(file, content);
      response.send(content, "application/octet-stream");
    }
  } else {
    response.status_code = "404";
    response.status_msg = "Not Found";
    response.send();
  }

  close(socket_fd);
}
