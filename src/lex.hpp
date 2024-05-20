#ifndef ATTO_LEX_H
#define ATTO_LEX_H

#include <string>
#include <string_view>
#include <vector>
#include <memory>

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

enum class BuiltinTypes {
  // The ordering of these must conform to persers requirements
  // single expression
  Num, Str, List, Litr, Words,
  Input, Print, Head, Tail,


  // two things expression
  Fuse, Pair, Eq, Add,  Neg,
  Mul, Div, Rem, Less, LessEq,

  // secial case these
  If,

  // epsilon
  Value, Ident,

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

class Token {
  int _line, _col;
  BuiltinTypes _tokType;
  LexTypes _lexType;
  std::string _ident;
public:
  Token(LexTypes type, std::string_view ident,
        int line = -1, int col = -1);
  int line() const;
  int col() const;
  BuiltinTypes type() const;
  std::string_view ident() const;
  std::string_view value() const;
};

class SyntaxError {
public:
  int line, col;
  std::string what;
  SyntaxError(std::string what, int line, int col);
};

std::vector<std::shared_ptr<Token>> lex(std::string_view code);



#endif // ATTO_LEX_H
