#ifndef ATTO_COMMON_H
#define ATTO_COMMON_H

#include <string>
#include <string_view>
#include <vector>
#include <filesystem>

namespace atto {

// --------------------------------------------------------
// string stuff

/// trim whitespace at end
std::string_view rtrim(std::string_view str);
/// trim whitespace from beginning
std::string_view ltrim(std::string_view str);
/// trim whitespace from both ends
std::string_view trim(std::string_view str);
/// @brief split a string into parts using delim as splitter
/// @param str the string to split
/// @param delim split on this substring
/// @return Vector with the parts
std::vector<std::string> split(const std::string& str,
                               const std::string& delim);
/// @brief Join string parts
/// @param parts all the string parts to join
/// @param joiner Join string on these parts
/// @return The joined string
std::string join(const std::vector<std::string>& parts,
                 const std::string& joiner = ",");

/// @brief Take a substring from str, but utf8_aware
/// @param str The string to substring from, default from start
/// @param from position to start at
/// @param ulen how many utf8 letters to use in substring, default to end of string
/// @return The new substring
std::string utf8_substr(const std::string& str,
                        std::size_t from = 0,
                        std::size_t ulen = std::string::npos);
/// @brief Split string in all it words
/// @param str The source text to get the words from
/// @return Vector with all words
std::vector<std::string> utf8_words(const std::string& str);

// ---------------------------------------------------------

/// @brief The different types a AST node (or token) can have
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


/// @brief Get the name of type as a string_view
///  useful for debug print what type it is
/// @param type Type LangType
/// @return The typename as a string_view
std::string_view typeName(LangType type);

/// @brief Read (text)file at path
/// @param path the path to the file to read
/// @param success get set to true if read was successful
/// @return All read bytes in this file
std::string readFile(std::filesystem::path path, bool& success);

} // namespace atto


#endif // ATTO_COMMON_H
