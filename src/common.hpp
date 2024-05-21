#ifndef ATTO_COMMON_H
#define ATTO_COMMON_H

#include <string>
#include <string_view>
#include <vector>

namespace atto {

// --------------------------------------------------------
// string stuff
std::string_view rtrim(std::string_view str);
std::string_view ltrim(std::string_view str);
std::string_view trim(std::string_view str);
std::vector<std::string> split(const std::string& str,
                              const std::string& delim);

// ---------------------------------------------------------

enum class LangType {
  // The ordering of these must conform to persers requirements
  // single expression
  Num, // first in 1
  List, Litr, Words,
  Input, Print, Head, Neg,
  Tail, // last in 1


  // two things expression
  Fuse, // first in 2
  Pair, Eq, Add,
  Mul, Div, Rem, Less,
  LessEq, // last in 2

  // special case these
  If,

  // epsilon
  Value, Bool, Null, Str,
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

} // namespace atto


#endif // ATTO_COMMON_H
