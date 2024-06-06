#include "common.hpp"
#include "errors.hpp"
#include <sstream>
#include <iostream>
#include <fstream>


namespace atto {

namespace fs = std::filesystem;

std::string_view typeName(LangType type)
{
  switch (type) {
  case LangType::List:   return "List";
  case LangType::Litr:   return "Litr";
  case LangType::Words:  return "Words";
  case LangType::Input:  return "Input";
  case LangType::Print:  return "Print";
  case LangType::Head:   return "Head";
  case LangType::Neg:    return "Neg";
  case LangType::Import: return "Import";
  case LangType::Tail:   return "Tail";
  case LangType::Fuse:   return "Fuse";
  case LangType::Pair:   return "Pair";
  case LangType::Eq:     return "Eq";
  case LangType::Add:    return "Add";
  case LangType::Mul:    return "Mul";
  case LangType::Div:    return "Div";
  case LangType::Rem:    return "Rem";
  case LangType::Less:   return "Less";
  case LangType::LessEq: return "LessEq";
  case LangType::If:     return "If";
  case LangType::Value:  return "Value";
  case LangType::Num_litr:    return "Num_litr";
  case LangType::True_litr:   return "True_litr";
  case LangType::False_litr:  return "False_litr";
  case LangType::Null_litr:   return "Null_litr";
  case LangType::Str_litr:    return "Str_litr";
  case LangType::Str:    return "Str";
  case LangType::Ident:  return "Ident";
  case LangType::Fn:     return "Fn";
  case LangType::Is:     return "Is";
  case LangType::Call:   return "call";
  case LangType::__Failure:  return "__Failure";
  case LangType::__Finished: return "__Finished";
  }
  return "_unhandled_LangType";
}

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

std::vector<std::string> split(
  const std::string& str,
  const std::string& delim
) {
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

std::string join(
  const std::vector<std::string>& parts,
  const std::string& joiner)
{
  if (parts.size() == 0)
    return "";

  std::stringstream ss;
  auto it = parts.begin();
  ss << *it++;
  for (; it != parts.end(); ++it)
    ss << joiner << *it;

  return ss.str();
}

std::string utf8_substr(const std::string& str,
                        std::size_t from,
                        std::size_t len)
{
    std::size_t ufrom = 0, ulen = 0;
    const char *start = str.c_str(),
               *cp = start,
               *end = start + str.size();

    for (; cp < end; cp++) {
        if((*cp & 0x80) != 0) {
            if (from+ufrom > static_cast<std::size_t>(cp-start))
              ufrom += 1;
            else
              ulen += 1;
        }
        if(len+ulen >= static_cast<std::size_t>(cp-start))
            break;
    }

    return str.substr(from+ufrom, len+ulen);
}

std::vector<std::string> utf8_words(const std::string& str)
{
  std::vector<std::string> words;
  std::size_t len = str.size(), pos = 0, n = len;
  bool multibyte = false;
  const char *start = str.c_str(),
             *cp = str.c_str(),
             *end = cp + len;

  for (; cp < end; ++cp) {
    if (!multibyte && isspace(*cp)) {
      n = static_cast<std::size_t>(cp-start) - pos;
      if (n > 0)
        words.emplace_back(str.substr(pos, n));
      pos += n+1;
    }
    multibyte = (*cp & 0x80) != 0;
  }

  if (static_cast<std::size_t>(cp-start)>pos)
    words.emplace_back(str.substr(pos));
  return words;
}

std::string readFile(std::filesystem::path path, bool& success)
{
  success = false;
  auto filestat = fs::status(path);
  if (!fs::exists(path)) {
    std::cerr << "File: " << path << " does not exist.\n";
    return "";
  }
  if (!fs::is_regular_file(filestat)) {
    std::cerr << "File: " << path << " is not a regular file.\n";
    return "";
  }

  auto perms = filestat.permissions();
  if ((perms & fs::perms::owner_read) == fs::perms::none ||
      (perms & fs::perms::group_read) == fs::perms::none ||
      (perms & fs::perms::others_read) == fs::perms::none)
  {
    std::cerr << "Insufficient privileges to access file: " << path << ".\n";
    return "";
  }

  if (fs::is_empty(path)) {
    std::cerr << "File: " << path << " is empty.";
    return "";
  }

  std::ifstream file;
  file.open(path, std::ios::in);
  if (file.is_open()) {
    std::stringstream ss;
    ss << file.rdbuf();
    file.close();
    success = true;
    return ss.str();
  }
  std::cerr << "Failed to open file " << path << '\n';
  return "";
}

} // namespace atto
