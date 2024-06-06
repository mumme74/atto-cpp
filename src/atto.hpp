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
#include "vm.hpp"

namespace atto {



// ----------------------------------------------------

class Atto
{
protected:
  std::filesystem::path _replHistoryPath;
  Vm vm;
public:
  Atto(std::filesystem::path replHistoryPath = ".replHistory");
  ~Atto();

  const Value execFile(std::filesystem::path path, std::string modName = "__main__");
  void repl();
};

} // namespace atto

#endif // ATTO_ATTO_H
