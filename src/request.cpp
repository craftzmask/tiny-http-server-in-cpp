#include "request.h"
#include <ranges>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

auto split_string(std::string_view text, std::string_view delimiter) {
  return std::views::split(text, delimiter) |
         std::ranges::to<std::vector<std::string>>();
}

void parseRequest(Request &request, std::string_view requestString) {
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