#include <string>
#include <unordered_map>

struct Response {
  std::string http_version;
  std::string status_code;
  std::string status_msg;
  std::unordered_map<std::string, std::string> headers;
};