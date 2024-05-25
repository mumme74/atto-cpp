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
public:
  Vm();
  ~Vm();

  std::shared_ptr<const Value> eval(
    const Expr& expr,
    const std::unordered_map<std::string, Module::FuncDef>& funcs,
    const std::vector<std::shared_ptr<const Value>>& args);
};

} // namespace atto

#endif // ATTO_VM_H
