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

enum class Intrinsic {
    // Flow control
    If,
    // Arithmetic
    Add, Neg, Mul, Inv, Rem,
    // Logical
    Eq, Less,
    // List manipulation
    Head, Tail, Pair, Fuse,
    // String manipulation
    Litr, Str, Words,
    // I/O
    In, Out,
};

class Token {
  int _line, _col;
  LangType _tokType;
  std::string_view _ident;
public:
  Token(LexTypes type,
        std::shared_ptr<const Module>,
        std::string_view ident,
        int line = -1, int col = -1);
  int line() const;
  int col() const;
  LangType type() const;
  std::string_view ident() const;
  std::string_view value() const;
};


void lex(std::shared_ptr<Module> module, std::size_t from = 0);

} // namespace atto


#endif // ATTO_LEX_H
