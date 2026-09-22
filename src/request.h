#pragma once
#include <string>
#include <unordered_map>

struct Request {
  std::string method;
  std::string path;
  std::string http_version;
  std::unordered_map<std::string, std::string> headers;
};

void parseRequest(Request &request, std::string_view requestString);