#ifndef ATTO_COMMON_H
#define ATTO_COMMON_H

#include <string>
#include <string_view>
#include <vector>
#include <filesystem>

namespace atto {

// --------------------------------------------------------
// string stuff
std::string_view rtrim(std::string_view str);
std::string_view ltrim(std::string_view str);
std::string_view trim(std::string_view str);
std::vector<std::string> split(const std::string& str,
                               const std::string& delim);
std::string join(const std::vector<std::string>& parts,
                 const std::string& joiner = ",");
std::string utf8_substr(const std::string& str,
                        std::size_t from = 0,
                        std::size_t ulen = std::string::npos);
std::vector<std::string> utf8_words(const std::string& str);

// ---------------------------------------------------------

enum class LangType {
  // The ordering of these must conform to persers requirements
  // single expression
  List, // first in 1
  Litr, Str, Words,
  Input, Print, Head, Neg,
  Import,
  Tail, // last in 1


  // two things expression
  Fuse, // first in 2
  Pair, Eq, Add,
  Mul, Div, Rem, Less,
  LessEq, // last in 2

  // special case these
  If,

  // epsilon
  Value, // first in epsilon
  Num_litr, True_litr, False_litr, Null_litr,
  Str_litr, // last in epsilon

  // special epsilon
  Ident,

  // only for function definitions
  Fn, Is,
  // only for function call
  Call,

  __Finished,
  __Failure
};

enum class LexTypes {
  Default, Number, String, Ident
};

std::string_view typeName(LangType type);

std::string readFile(std::filesystem::path path, bool& success);

} // namespace atto


#endif // ATTO_COMMON_H
