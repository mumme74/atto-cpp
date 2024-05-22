#ifndef ATTO_ERRORS_H
#define ATTO_ERRORS_H

#include <string>
#include <string_view>
#include <memory>

namespace atto {

class Module;

class Error {
protected:
  std::string _what;
  std::shared_ptr<const Module> _module;
public:
  Error(std::string what, std::shared_ptr<const Module> module);
  virtual ~Error();
  std::string_view what() const;
  virtual std::string_view typeName() const;
  std::shared_ptr<const Module> module() const;
};

class SyntaxError : public Error {
protected:
  int _line, _col;
public:
  SyntaxError(std::string what, std::shared_ptr<const Module> module,
              int line, int col);
  ~SyntaxError();
  virtual std::string_view typeName() const;
  int line() const;
  int col() const;
};

class ParseError : public SyntaxError {
public:
  ParseError(std::string what, std::shared_ptr<const Module> module,
              int line, int col);
  virtual std::string_view typeName() const;
};

} // namespace atto


#endif // ATTO_ERRORS_H
