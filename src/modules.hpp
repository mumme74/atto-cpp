#ifndef ATTO_MODULES_H
#define ATTO_MODULES_H

#include <filesystem>
#include <string>
#include <vector>
#include <unordered_map>
#include "lex.hpp"
#include "parser.hpp"

namespace atto {

/**
 * @brief All different scripts are loaded as a module
 */
class Module {
public:
  using FuncDef = std::pair<
    std::shared_ptr<Func>, std::vector<std::string>>;
private:
  std::filesystem::path _path;
  std::string _code;
  std::vector<std::shared_ptr<const Token>> _tokens;
  std::unordered_map<std::string,
    std::pair<std::shared_ptr<Func>,
    std::vector<std::string>>> _funcs;
public:
  Module(std::filesystem::path path, std::string code);
  ~Module();

  const std::filesystem::path& path() const;
  std::string_view code() const;
  void appendCode(const std::string code); // for REPL
  void addToken(std::shared_ptr<const Token> &tok);
  std::vector<std::shared_ptr<const Token>>& tokens();
  std::unordered_map<std::string, FuncDef>& funcs();

  static
  std::unordered_map<std::string, std::shared_ptr<Module>> allModules;
};

} // namespace atto

#endif // ATTO_MODULES_H
