#include "lex.hpp"

Token::Token(LexTypes type, std::string_view ident,
        int line, int col):
  _line{line}, _col{col},
  _lexType{type},
  _ident{ident}
{
  switch (_lexType) {
  case LexTypes::Number:
    _tokType = BuiltinTypes::Num; break;
  case LexTypes::String:
    _tokType = BuiltinTypes::Str; break;
  case LexTypes::Ident: {
    auto twoFirst = ident.substr(0,2);
    if (twoFirst == "fn") _tokType = BuiltinTypes::Fn;
    else if (twoFirst == "is") _tokType = BuiltinTypes::Is;
    else if (twoFirst == "if") _tokType = BuiltinTypes::If;
    else if (twoFirst == "__") {
      auto name = ident.substr(2);
      if (name == "head")        _tokType = BuiltinTypes::Head;
      else if (name == "tail")   _tokType = BuiltinTypes::Tail;
      else if (name == "fuse")   _tokType = BuiltinTypes::Fuse;
      else if (name == "pair")   _tokType = BuiltinTypes::Pair;
      else if (name == "litr")   _tokType = BuiltinTypes::Litr;
      else if (name == "str")    _tokType = BuiltinTypes::Str;
      else if (name == "words")  _tokType = BuiltinTypes::Words;
      else if (name == "input")  _tokType = BuiltinTypes::Input;
      else if (name == "print")  _tokType = BuiltinTypes::Print;
      else if (name == "eq")     _tokType = BuiltinTypes::Eq;
      else if (name == "add")    _tokType = BuiltinTypes::Add;
      else if (name == "neg")    _tokType = BuiltinTypes::Neg;
      else if (name == "mul")    _tokType = BuiltinTypes::Mul;
      else if (name == "div")    _tokType = BuiltinTypes::Div;
      else if (name == "rem")    _tokType = BuiltinTypes::Rem;
      else if (name == "less")   _tokType = BuiltinTypes::Less;
      else if (name == "lesseq") _tokType = BuiltinTypes::LessEq;
      else _tokType = BuiltinTypes::Ident;
    } else _tokType = BuiltinTypes::Ident;
  } break;
  case LexTypes::Default: [[fallthrough]];
  default:
    throw SyntaxError(
      std::string("Literal not valid: ") + std::string(ident), line, col);
  }

}

int Token::col() const
{
  return _col;
}

int Token::line() const
{
  return _line;
}

BuiltinTypes Token::type() const
{
  return _tokType;
}

std::string_view Token::ident() const
{
  return _ident;
}

std::string_view Token::value() const
{
  return _tokType == BuiltinTypes::Str ?
    _ident.substr(1,_ident.length() -1) : _ident;
}

// ---------------------------------------------------

SyntaxError::SyntaxError(std::string what, int line, int col) :
  what{what}, line{line}, col{col}
{}

// ----------------------------------------------------

std::vector<std::shared_ptr<Token>> lex(std::string_view code)
{

  std::vector<std::shared_ptr<Token>> tokens;
  LexTypes state{LexTypes::Default};
  int lineNr = 1;
  auto lineBegin = code.begin();
  const char *tokBegin = nullptr,
             *cp = nullptr;

  auto startToken = [&](LexTypes s) {
    tokBegin = cp;
    state = s;
  };
  auto endToken = [&]() {
    auto ident = code.substr(tokBegin - code.begin(), cp - tokBegin);
    int col = static_cast<int>(tokBegin - lineBegin);
    auto tok = std::make_shared<Token>(state, ident, lineNr, col);
    tokens.emplace_back(tok);
    tokBegin = nullptr;
    state = LexTypes::Default;
  };
  auto syntaxError = [&](const std::string &msg){
    return SyntaxError(msg, lineNr, static_cast<int>(cp - lineBegin));
  };

  bool incr = true, escaped = false;

  for(cp = code.begin(); cp != code.end(); incr && ++cp) {
    incr = true;
    if (*cp == '\r' && *(cp+1) == '\n')
      continue; // make sure it is not past end()
    if (*cp == '\n') {
      lineNr++; lineBegin = cp+1;
      continue; // make sure it is not past end()
    }

    switch (state) {
    case LexTypes::Default:
      if (*cp == '\0') {
        cp = code.end(); // bail out
        incr = false;
      }
      else if (isspace(*cp)) ; // intentional nothing
      else if (*cp == '"') startToken(LexTypes::String);
      else if (isdigit(*cp)) startToken(LexTypes::Number);
      else // must be a ident
        startToken(LexTypes::Ident);
      break;
    case LexTypes::Number:
      if (isspace(*cp)) {
        incr = false;
        endToken();
      }
      else if (!isalnum(*cp) || *cp == '.')
        throw syntaxError("Unexpected char in numbers litteral");
      break;
    case LexTypes::String:
      if (!escaped && *cp == '"') endToken();
      else if (*cp == '\0') throw syntaxError("Invalid null char in string");
      else if (*cp == '\\') {
        escaped = true;
        continue;
      }
      else if (escaped && *cp != 'n')
        throw syntaxError("Invalid escape sequence");
      escaped = false;
      break;
    case LexTypes::Ident:
      if (isspace(*cp)) endToken();
      else if (!isalpha(*cp) && *cp != '_' && *cp != '-')
        throw syntaxError("Invalid char in identifer literal");
      break;
    }
  }

  if (tokBegin != nullptr) endToken();

  return tokens;
}
