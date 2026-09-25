#pragma once
#include <string>
#include <unordered_map>

struct Request {
  std::string method;
  std::string path;
  std::string http_version;
  std::unordered_map<std::string, std::string> headers;

  std::string get_echo() {
    if (path.contains("/echo/")) {
      const std::string s = "/echo/";
      size_t index = path.find(s);
      return path.substr(index + s.size());
    } else {
      return "";
    }
  }
};

void parseRequest(Request &request, std::string_view request_string);