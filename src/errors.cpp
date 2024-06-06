#include "errors.hpp"
#include "lex.hpp"

using namespace atto;

Error::Error(
  std::string what, const Module& module
) :
  _what(what), _module{module}
{}

Error::~Error() {}

std::string_view Error::what() const
{
  return _what;
}

std::string_view Error::typeName() const
{
  return "Error";
}

const Module& Error::module() const
{
  return _module;
}

// ---------------------------------------------------

SyntaxError::SyntaxError(
  std::string what,
  const Module& module,
  int line, int col
) :
  Error{what, module}, _line{line}, _col{col}
{}

SyntaxError::SyntaxError(
  std::string what,
  const Module& module,
  const Token& tok
) :
  SyntaxError{what, module, tok.line(), tok.col()}
{}

SyntaxError::~SyntaxError() {}

std::string_view SyntaxError::typeName() const
{
  return "SyntaxError";
}

int SyntaxError::line() const
{
  return _line;
}

int SyntaxError::col() const
{
  return _col;
}

// --------------------------------------------------------

ParseError::ParseError(
  std::string what,
  const Module& module,
  int line, int col
) :
    SyntaxError{what, module, line, col}
{}

ParseError::ParseError(
  std::string what,
  const Module& module,
  const Token& tok
) :
  ParseError(what, module, tok.line(), tok.col())
{}

std::string_view ParseError::typeName() const
{
  return "ParseError";
}

// --------------------------------------------------------

FileIOError::FileIOError(
  std::string what, const Module& module,
  std::filesystem::path path):
    Error{what, module}
{}

std::string_view FileIOError::typeName() const
{
  return "FileIOError";
}