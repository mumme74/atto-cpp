#ifndef ATTO_LEX_H
#define ATTO_LEX_H

#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include "common.hpp"
//#include "modules.hpp"

namespace atto {

class Module;

/**
 * @brief A token in the source code
 */
class Token {
  int _line, _col;
  LangType _tokType;
  std::string_view _ident;
public:
  /**
   * @brief Construct a new Token object
   *
   * @param type What LexType this token is
   * @param module In what module is token create
   * @param ident The source string for this token
   * @param line What line is the token located
   * @param col What column is this token located in this line
   */
  Token(LexTypes type,
        const Module& module,
        std::string_view ident,
        int line = -1, int col = -1);
  /// @brief Get the line
  /// @return The line nr
  int line() const;
  /// @brief Get the column
  /// @brief The column nr
  int col() const;
  bool operator==(const Token& other) const;
  /// @brief get the type form this token
  /// @return LangType for this token
  LangType type() const;
  /// @brief Get the source string for this token
  std::string_view ident() const;
  /// @brief Get the value for this token, ie no '"' for this string
  std::string_view value() const;
};

/**
 * @brief Lex (Tokenize) the source code
 *
 * @param module in what module the source is found
 * @param from From what position to begin lex
 */
void lex(const Module &module, std::size_t from = 0);

} // namespace atto


#endif // ATTO_LEX_H
