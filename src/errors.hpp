#ifndef ATTO_ERRORS_H
#define ATTO_ERRORS_H

#include <string>
#include <string_view>
#include <filesystem>
#include <memory>

namespace atto {

class Module;

/**
 * @brief Base class for all errors
 */
class Error {
protected:
  std::string _what;
  const Module& _module;
public:
  /**
   * @brief Construct a new Error object
   * @param what What happened?
   * @param module In what module?
   */
  Error(std::string what, const Module& module);
  virtual ~Error();

  /// @brief getter for what
  /// @return string of what happened
  std::string_view what() const;
  /// @return std::string_view The name of this error, ie. Error, subclasses reimplement
  virtual std::string_view typeName() const;

  /// @brief getter for module
  /// @return Reference to the module where it happened
  const Module& module() const;
};

/**
 * @brief An error throw when syntax is not understood
 */
class SyntaxError : public Error {
protected:
  int _line, _col;
public:
  /**
   * @brief Construct a new Syntax Error object
   *
   * @param what The message of what happened
   * @param module The module where it happened
   * @param line The line where it happened
   * @param col The column in the line where it happened
   */
  SyntaxError(std::string what, const Module& module,
              int line, int col);
  ~SyntaxError();
  /**
   * @return std::string_view The name of this error, ie. SyntaxError
   */
  virtual std::string_view typeName() const override;

  /// @return The line nr
  int line() const;

  /// @return The column nr
  int col() const;
};

/**
 * @brief A error thrown when parsing source code
 */
class ParseError : public SyntaxError {
public:
  /**
   * @brief Construct a new ParseError object
   *
   * @param what The message, description of what happened
   * @param module The module where it happened
   * @param line The line where it happened
   * @param col The column where it happened
   */
  ParseError(std::string what, const Module& module,
              int line, int col);

  /**
   * @return std::string_view The name of this error, ie. ParseError
   */
  virtual std::string_view typeName() const override;
};

/**
 * @brief Error when Reading writing file
 */
class FileIOError : public Error {
protected:
  std::filesystem::path _path;
public:
  /**
   * @brief Construct a new File I/O Error object
   *
   * @param what The error message
   * @param module The module where it happened
   * @param path The path related to this error
   */
  FileIOError(std::string what, const Module& module,
              std::filesystem::path path);
  /**
   * @return std::string_view The name of this error, ie. FileIOError
   */
  virtual std::string_view typeName() const override;
};

} // namespace atto


#endif // ATTO_ERRORS_H
