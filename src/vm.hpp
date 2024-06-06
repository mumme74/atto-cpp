#ifndef ATTO_VM_H
#define ATTO_VM_H

#include <memory>
#include <vector>
#include <unordered_map>
#include "modules.hpp"
#include "parser.hpp"
#include "values.hpp"

namespace atto {

class Vm {
  void print(std::string_view msg) const;
  Value input(std::string_view msg) const;
  void import(Module& mod, std::filesystem::path path) const;
public:
  Vm();
  ~Vm();

  std::shared_ptr<const Value> eval(
    const AstBase& expr,
    const FuncMap& funcs,
    const std::vector<std::shared_ptr<const Value>>& args);
};

} // namespace atto

#endif // ATTO_VM_H
