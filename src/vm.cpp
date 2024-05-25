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
  const std::vector<std::shared_ptr<Value>>& args
) {
  switch (expr.type()) {
  case LangType::If:
    if (eval(*expr.exprs()[0], funcs, args)->asBool())
      return eval(*expr.exprs()[1], funcs, args);
    else
      return eval(*expr.exprs()[2], funcs, args);
  case LangType::Eq:{
    auto l = eval(*expr.exprs()[0], funcs, args);
    auto r = eval(*expr.exprs()[1], funcs, args);
    return std::make_shared<Value>(*l == *r);
  }
  case LangType::Add:{
    auto l = eval(*expr.exprs()[0], funcs, args);
    auto r = eval(*expr.exprs()[1], funcs, args);
    return std::make_shared<Value>(*l + *r);
  }
  case LangType::Neg:{
    auto v = eval(*expr.exprs()[0], funcs, args);
    return std::make_shared<Value>(v->neg());
  }
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
  case LangType::Head: {
    auto v = eval(*expr.exprs()[0], funcs, args);
    if (v->isList())
      return std::make_shared<Value>(v->asList().front().clone());
    else if (v->isStr()) {
      return std::make_shared<Value>(v->asStr().substr(0,1));
    }
    return std::make_shared<Value>(*v);
  }
  case LangType::Tail: {
    auto v = eval(*expr.exprs()[0], funcs, args);
    if (v->isList()) {
      const auto& l = v->asList();
      std::vector<Value> list; list.reserve(l.size());
      for (const auto& v : l)
        list.emplace_back(v.clone());
      return std::make_shared<Value>(std::move(list));
    } else if (v->isStr()) {
      return std::make_shared<Value>(v->asStr().substr(1));
    }
    return std::make_shared<Value>(*v);
  }
  case LangType::Fuse: {
    std::vector<Value> list;
    auto fill = [&](const Value& from){
      if (from.isList()) {
        auto flist = from.asList();
        list.insert(list.end(), flist.begin(), flist.end());
      } else
        list.emplace_back(from);
    };
    fill(eval(*expr.exprs()[0], funcs, args)->clone());
    fill(eval(*expr.exprs()[1], funcs, args)->clone());
    return std::make_shared<Value>(list);
  }
  case LangType::Pair: {
    std::vector<Value> list; list.reserve(2);
    list.emplace_back(*eval(*expr.exprs()[0], funcs, args));
    list.emplace_back(*eval(*expr.exprs()[1], funcs, args));
    return std::make_shared<Value>(list);
  }
  /*case LangType::Words: {

  }*/
  case LangType::Litr: {
    auto v = eval(*expr.exprs()[0], funcs, args);
    return std::make_shared<Value>(Value::from_str(v->asStr()));
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
  case LangType::Call: {
    auto call = static_cast<const Call*>(&expr);
    std::string fnName = std::string(call->fnName());
    const auto& fn = call->module()->funcs()[fnName].first;
    if (fn) {
      std::vector<std::shared_ptr<Value>> params;
      params.reserve(args.size());
      for (const auto& e : expr.exprs())
        params.emplace_back(eval(*e, funcs, args));
      return eval(*fn, fn->module()->funcs(), params);
    }
    return Value::Null_ptr;
  }
  case LangType::Fn: {
    auto fn = static_cast<const Func*>(&expr);
    std::shared_ptr<Value> last;
    std::cout << "Entering fn " << fn->fnName()
              << " " << join(fn->args(), ", ") << "\n";
    for (const auto& e : fn->exprs())
      last = eval(*e, funcs, args);
    std::cout << "Leave fn " << fn->fnName() << " with value "
              << last->asStr() << "\n";
    return last;
  }
  case LangType::Null: return Value::Null_ptr;
  case LangType::Value:
  case LangType::Num:
  case LangType::True: case LangType::False:
  case LangType::List: [[fallthrough]];
  case LangType::Str: {
    const auto& exprVlu = static_cast<const ExprValue*>(&expr);
    return std::make_shared<Value>(exprVlu->value());
  }
  case LangType::Ident: {
    const auto& exprVlu = static_cast<const ExprIdent*>(&expr);
    auto v = args[exprVlu->localIdx()];
    std::cout << "Get ident " << expr.token().ident()
              << " vlu:" << v->asStr() << " type:" << v->typeName() << "\n";
    return v;
  }

  default:
    std::cerr <<
      "unhandled expr.type():" << typeName(expr.type()) << '\n';
    return nullptr;
  }
}

} // namespace atto
