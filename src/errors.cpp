#include "errors.hpp"

using namespace atto;

Error::Error(
  std::string what, std::shared_ptr<const Module> module
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

std::shared_ptr<const Module> Error::module() const
{
  return _module;
}

// ---------------------------------------------------

SyntaxError::SyntaxError(
  std::string what,
  std::shared_ptr<const Module> module,
  int line, int col
) :
  Error{what, module}, _line{line}, _col{col}
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
