#include "lex.hpp"
#include "common.hpp"
#include "errors.hpp"
#include "modules.hpp"

namespace atto {

Token::Token(
  LexTypes type,
  std::shared_ptr<const Module> module,
  std::string_view ident,
  int line, int col
):
  _line{line}, _col{col},
  _ident{ident}
{
  switch (type) {
  case LexTypes::Number:
    _tokType = LangType::Num_litr; break;
  case LexTypes::String:
    _tokType = LangType::Str_litr; break;
  case LexTypes::Ident: {
    auto twoFirst = ident.substr(0,2);
    if (ident == "true")       _tokType = LangType::True_litr;
    else if (ident == "false") _tokType = LangType::False_litr;
    else if (ident == "null")  _tokType = LangType::Null_litr;
    else if (ident == "fn")    _tokType = LangType::Fn;
    else if (ident == "is")    _tokType = LangType::Is;
    else if (ident == "if")    _tokType = LangType::If;
    else if (twoFirst == "__") {
      auto name = ident.substr(2);
      if (name == "head")        _tokType = LangType::Head;
      else if (name == "tail")   _tokType = LangType::Tail;
      else if (name == "fuse")   _tokType = LangType::Fuse;
      else if (name == "pair")   _tokType = LangType::Pair;
      else if (name == "litr")   _tokType = LangType::Litr;
      else if (name == "str")    _tokType = LangType::Str;
      else if (name == "words")  _tokType = LangType::Words;
      else if (name == "input")  _tokType = LangType::Input;
      else if (name == "print")  _tokType = LangType::Print;
      else if (name == "eq")     _tokType = LangType::Eq;
      else if (name == "add")    _tokType = LangType::Add;
      else if (name == "neg")    _tokType = LangType::Neg;
      else if (name == "mul")    _tokType = LangType::Mul;
      else if (name == "div")    _tokType = LangType::Div;
      else if (name == "rem")    _tokType = LangType::Rem;
      else if (name == "less")   _tokType = LangType::Less;
      else if (name == "lesseq") _tokType = LangType::LessEq;
      else if (name == "import") _tokType = LangType::Import;
      else _tokType = LangType::Ident;
    } else _tokType = LangType::Ident;
  } break;
  case LexTypes::Default: [[fallthrough]];
  default:
    throw SyntaxError(
      std::string("Literal not valid: ") + std::string(ident),
      module,
      line, col);
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

LangType Token::type() const
{
  return _tokType;
}

std::string_view Token::ident() const
{
  return _ident;
}

std::string_view Token::value() const
{
  return _tokType == LangType::Str ?
    _ident.substr(1,_ident.length() -1) : _ident;
}

bool Token::operator==(const Token& other) const
{
  return _line == other._line && _col == other._col &&
         _ident == other._ident && _tokType == other._tokType;
}

// ----------------------------------------------------

void lex(std::shared_ptr<Module> module, std::size_t from) {

  LexTypes state{LexTypes::Default};
  int lineNr = 1;
  auto code = module->code().substr(from);
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
    auto tok = std::make_shared<const Token>(state, module, ident, lineNr, col);
    module->addToken(tok);
    tokBegin = nullptr;
    state = LexTypes::Default;
  };
  auto syntaxError = [&](const std::string &msg){
    int col = static_cast<int>(cp - lineBegin);
    return SyntaxError(msg, module, lineNr, col);
  };

  bool incr = true, escaped = false;

  for(cp = code.begin(); cp != code.end(); incr && ++cp) {
    incr = true;
    if (*cp == '\r' && *(cp+1) == '\n')
      continue; // make sure it is not past end()
    if (*cp == '\n' && state != LexTypes::String) {
      if (tokBegin) endToken();
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
      else if (*cp == '"') {
        ++cp;
        startToken(LexTypes::String);
        if (*cp == '"') endToken(); // special case empty string
      }
      else if (isdigit(*cp)) startToken(LexTypes::Number);
      else // must be a ident
        startToken(LexTypes::Ident);
      break;
    case LexTypes::Number:
      if (isspace(*cp)) {
        incr = false;
        endToken();
      }
      else if (!isdigit(*cp) || *cp == '.')
        throw syntaxError("Unexpected char in numbers literal");
      break;
    case LexTypes::String:
      if (!escaped && *cp == '"')
        endToken();
      else if (*cp == '\0')
        throw syntaxError("Invalid null char in string");
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
      else if (!isalnum(*cp) && *cp != '_' && *cp != '-')
        throw syntaxError("Invalid char in identifier literal");
      break;
    }
  }

  if (tokBegin != nullptr) endToken();
}

} // namespace atto
