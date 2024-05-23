#include "vm.hpp"
#include "common.hpp"
#include "lib/linenoise.hpp"
#include <iostream>

namespace atto {

Vm::Vm() {}
Vm::~Vm() {}


void Vm::print(std::string_view msg) const
{
  std::cout << msg;
}

Value Vm::input(std::string_view msg) const
{
  auto str = linenoise::Readline(msg.begin());
  linenoise::AddHistory(str.c_str());
  return Value(str);
}

std::shared_ptr<Value> Vm::eval(
  const Expr& expr,
  const std::unordered_map<std::string, Module::FuncDef>& funcs,
  const std::vector<Value>& args
) {
  switch (expr.type()) {
  case LangType::If:
    if (eval(*expr.exprs()[0], funcs, args))
      return eval(*expr.exprs()[1], funcs, args);
    else
      return eval(*expr.exprs()[2], funcs, args);
  case LangType::Eq:
    return std::make_shared<Value>(
      eval(*expr.exprs()[0], funcs, args) ==
      eval(*expr.exprs()[1], funcs, args));
  case LangType::Add:
    return std::make_shared<Value>(
      *eval(*expr.exprs()[0], funcs, args) +
      *eval(*expr.exprs()[1], funcs, args));
  case LangType::Neg:
    return std::make_shared<Value>(
      !(*eval(*expr.exprs()[0], funcs, args)));
  case LangType::Mul:
    return std::make_shared<Value>(
      *eval(*expr.exprs()[0], funcs, args) *
      *eval(*expr.exprs()[1], funcs, args));
  case LangType::Div:
    return std::make_shared<Value>(
      *eval(*expr.exprs()[0], funcs, args) /
      *eval(*expr.exprs()[1], funcs, args));
  case LangType::Rem:
    return std::make_shared<Value>(
      *eval(*expr.exprs()[0], funcs, args) %
      *eval(*expr.exprs()[0], funcs, args));
  case LangType::Less:
    return std::make_shared<Value>(
      *eval(*expr.exprs()[1], funcs, args) >
      *eval(*expr.exprs()[0], funcs, args));
  case LangType::LessEq:
    return std::make_shared<Value>(
      *eval(*expr.exprs()[1], funcs, args) >=
      *eval(*expr.exprs()[0], funcs, args));
  /*case LangType::Head:
    return std::make_shared<Value>(
      *eval
    );*/

  case LangType::Litr: {
    auto v = eval(*expr.exprs()[0], funcs, args);
    return (v->type() == ValueTypes::Str) ? v :
      std::make_shared<Value>();
  }
  case LangType::Input: {
    auto e = eval(*expr.exprs()[0], funcs, args);
    return std::make_shared<Value>(input(e->asStr()));
  }
  case LangType::Print: {
    auto e = eval(*expr.exprs()[0], funcs, args);
    print(e->asStr());
    return Value::Null_ptr;
  }

  default:
    std::cerr <<
      "unhandled expr.type():" <<
      static_cast<int>(expr.type()) << '\n';
  }
}

} // namespace atto
