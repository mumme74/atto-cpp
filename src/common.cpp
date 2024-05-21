#include "common.hpp"
#include "errors.hpp"


namespace atto {

// ----------------------------------
// string stuff

const char* ws = " \t\n\r\f\v";
std::string_view rtrim(std::string_view str) {
  return str.substr(0, str.find_last_not_of(ws));
}
std::string_view ltrim(std::string_view str) {
  return str.substr(str.find_first_not_of(ws));
}
std::string_view trim(std::string_view str) {
  return rtrim(ltrim(str));
}

std::vector<std::string> split(const std::string& str, const std::string& delim) {
  std::vector<std::string> parts;
  std::string::size_type pos = 0;
  std::string::size_type prev = 0;
  while ((pos = str.find(delim, prev)) && pos != std::string::npos) {
    parts.emplace_back(str.substr(prev, pos - prev));
    prev = pos + delim.length();
  }

  parts.emplace_back( str.substr(prev));
  return parts;
}

} // namespace atto
