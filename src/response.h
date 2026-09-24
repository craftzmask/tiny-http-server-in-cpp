#pragma once
#include <string>
#include <format>
#include <unordered_map>
#include <unistd.h>

struct Response {
  int socket_fd;
  std::string http_version = "HTTP/1.1";
  std::string status_code = "200";
  std::string status_msg = "OK";
  std::unordered_map<std::string, std::string> headers;

  std::string to_string() {
    return std::format("{} {} {}\r\n\r\n", http_version, status_code, status_msg);
  }

  std::string to_string(const std::string& content, const std::string& content_type = "text/plain") {
    return std::format("{} {} {}\r\nContent-Type: {}\r\nContent-Length: {}\r\n\r\n{}",
                           http_version, status_code, status_msg, content_type, content.size(), content);
  }

  void send() {
    const std::string s = to_string();
    write(socket_fd, s.data(), s.size());
  }

  void send(const std::string& content, const std::string& content_type = "text/plain") {
    const std::string s = to_string(content, content_type);
    write(socket_fd, s.data(), s.size());
  }
};