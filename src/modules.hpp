#ifndef ATTO_MODULES_H
#define ATTO_MODULES_H

#include <filesystem>
#include <string>
#include <vector>
#include <unordered_map>
#include "lex.hpp"
#include "ast.hpp"

namespace atto {

/**
 * @brief A Module is a source file loaded by the engine.
 * All different scripts are loaded as a module
 */
class Module {
private:
  std::filesystem::path _path;
  std::string _code;
  std::vector<Token> _tokens;
  std::vector<std::string> _imported;
  std::unordered_map<std::string, FuncDef> _funcs;
  bool _parsed;

  static
  std::unordered_map<std::string, Module> _allModules;
public:
  /**
   * @brief Construct a new Module object
   *
   * @param path Path to this module
   * @param code The code for this module
   */
  Module(std::filesystem::path path, const std::string& code);
  /**
   * @brief Construct a new Module object, code is implicitly loaded
   *
   * @param path Path to this module
   */
  Module(std::filesystem::path path);
  /**
   * @brief Construct a new Module object using code.
   * Path is assumed to be empty, such as in a REPL
   *
   * @param code
   */
  Module(const std::string& code);

  Module();
  Module(const Module& other) = delete;
  Module(Module&& rhs);
  ~Module();

  Module& operator=(const Module& other) = delete;
  Module& operator=(Module&& rhs);

  /// @return true if module has successfully been parsed
  bool isParsed() const;
  /// @return The path to this module
  const std::filesystem::path& path() const;
  /// @brief Get the code for this module as
  /// @return String view for the code in this module
  std::string_view code() const;
  /// @brief Append code to module, used by repl
  /// @param code The code to append
  void appendCode(const std::string code); // for REPL
  /// @brief Add a token to the tokens in this module, module takes ownership
  void addToken(const Token &tok);
  /// @brief Get all tokens stored in this module
  const std::vector<Token>& tokens() const;
  /// @brief Get all functions in this module
  const FuncMap& funcs() const;
  /// @brief Get the function fn in this module, throws if not found
  const AstFunc& func(const std::string& fn) const;
  /// @brief Get the function def as a writable ref from this module, throws if not found
  FuncDef& funcDef(const std::string& fn);
  /// @brief Get function params
  const FuncParams& funcParams(const std::string& fn) const;
  /// @brief Find out if fn exists in this module
  bool hasFunc(const std::string& fn) const;
  /// @brief add a function to this module, used by parser
  void addFunc(const std::string& fn, FuncDef& def);
  /// @brief import path into this module, loads and parse if necessary
  void import(std::filesystem::path path);
  /// @brief All modules currently imported to this module.
  /// use as key to allModules to retrieve the actual module.
  /// @return All module names currently imported to this module
  const std::vector<std::string>& imported() const;



  /// @brief AllModules loaded to VM at this time
  /// @return vector with names of all modules
  static
 std::vector<std::string> allModuleNames();

  static
  void parse(Module& mod, std::string modName = "");

  static
  Module& module(const std::string& name,
                const std::filesystem::path path = "");
};

} // namespace atto

#endif // ATTO_MODULES_H
