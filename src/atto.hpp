#ifndef ATTO_ATTO_H
#define ATTO_ATTO_H

#include <string>
#include <string_view>
#include <vector>
#include <variant>
#include <filesystem>
#include <iostream>
#include <memory>
#include "common.hpp"
#include "modules.hpp"
#include "values.hpp"

namespace atto {



// ----------------------------------------------------

class Atto
{
protected:
  std::filesystem::path _replHistoryPath;
  std::istream &_cin;
  std::ostream &_cout, &_cerr;
  std::string _core_code;
  std::vector<std::shared_ptr<Module>> _modules;
public:
  Atto(std::filesystem::path replHistoryPath = ".replHistory",
       std::istream &cin = std::cin,
       std::ostream &cout = std::cout,
       std::ostream &cerr = std::cerr);
  virtual ~Atto();

  void print(std::string_view msg) const;
  Value input(std::string_view msg) const;


  bool execFile(std::filesystem::path path);
  void repl();
protected:
  std::string readFile(std::filesystem::path, bool & success);
};

} // namespace atto

#endif // ATTO_ATTO_H
