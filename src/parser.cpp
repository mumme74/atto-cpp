#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <utility>
#include <sstream>
#include "parser.hpp"
#include "errors.hpp"
#include "modules.hpp"

namespace atto {

Expr::Expr(std::shared_ptr<const Token> tok, LangType type, std::vector<Expr> exprs) :
  _tok{tok}, _type{type}, _exprs(std::move(exprs))
{}

Expr::Expr(const Expr& other) :
  _tok{other._tok}, _type{other._type}, _exprs{other._exprs}
{}

Expr::Expr(Expr&& rhs) :
  _tok{std::move(rhs._tok)},
  _type{std::move(rhs._type)},
  _exprs{std::move(rhs._exprs)}
{}

Expr& Expr::operator=(const Expr& other) {
  _tok = other._tok;
  _type = other._type;
  _exprs = other._exprs;
  return *this;
}

Expr& Expr::operator=(Expr&& rhs) {
  _tok = std::move(rhs._tok);
  _type = std::move(rhs._type);
  _exprs = std::move(rhs._exprs);
  return *this;
}

const Token& Expr::token() const { return *_tok; }
LangType Expr::type() const { return _type; }

bool Expr::isFailed() const {
  return _tok->type() == LangType::__Failure;
}

const std::vector<Expr>& Expr::exprs() const { return _exprs; }

const Expr& Expr::operator[](std::size_t idx) const {
  return _exprs[idx];
}

// static
Expr Expr::mkFailed() {
  return Expr{nullptr, LangType::__Failure, std::vector<Expr>{}};
}

// --------------------------------------------

ExprValue::ExprValue(std::shared_ptr<const Token> tok, Value value) :
  Expr{tok, LangType::Value, std::vector<Expr>{}},
  _vlu{std::make_unique<Value>(value).release()}
{}

ExprValue::ExprValue(ExprValue&& rhs) :
  Expr{rhs}, _vlu{std::move(rhs._vlu)}
{}

ExprValue& ExprValue::operator=(ExprValue&& rhs) {
  Expr::operator=(std::move(rhs));
  _vlu.reset(std::move(rhs._vlu.release()));
  return *this;
}

Value& ExprValue::value() const { return _vlu ? *_vlu : Value::Null; }


ExprIdent::ExprIdent(std::shared_ptr<const Token> tok, std::size_t localIdx) :
  Expr{tok, LangType::Ident, std::vector<Expr>{}},
  _localIdx{localIdx}
{}

ExprIdent::ExprIdent(const ExprIdent& other):
  Expr{other}, _localIdx{other._localIdx}
{}

ExprIdent::ExprIdent(ExprIdent&& rhs) :
  Expr{std::move(rhs)}, _localIdx{std::move(rhs._localIdx)}
{}

ExprIdent& ExprIdent::operator=(const ExprIdent& other) {
  Expr::operator=(other);
  _localIdx = other._localIdx;
  return *this;
}

ExprIdent& ExprIdent::operator=(ExprIdent&& rhs) {
  Expr::operator=(std::move(rhs));
  _localIdx = std::move(rhs._localIdx);
  return *this;
}

std::size_t ExprIdent::localIdx() const { return _localIdx; }

// ------------------------------------------------------

Func::Func(std::shared_ptr<const Token> tok,
      std::vector<std::string> args,
      Expr expr) :
  Expr{tok, LangType::Fn, std::vector<Expr>{expr}},
  _args{args}
{}

Func::Func(const Func& other) :
  Expr{other}, _args{other._args}
{}

Func::Func(Func&& rhs) :
  Expr{std::move(rhs)}, _args{std::move(_args)}
{}

Func& Func::operator=(const Func& other) {
  Expr::operator=(other);
  _args = other._args;
  return *this;
}

Func& Func::operator=(Func&& rhs) {
  Expr::operator=(std::move(rhs));
  _args = std::move(rhs._args);
  return *this;
}

const Expr& Func::expr() const { return *_exprs.begin(); }

const std::vector<std::string>& Func::args() const { return _args; }

bool Func::isFailure() const { return _type == LangType::__Failure; }

// -----------------------------------------------------------

Call::Call(std::shared_ptr<const Token> tok,
           std::vector<Expr> params,
           std::string fnName) :
  Expr{tok, LangType::Call, params},
  _fnName{fnName}
{}

Call::Call(const Call& other) :
  Expr{other}, _fnName{other._fnName}
{}

Call::Call(Call&& rhs) :
  Expr{std::move(rhs)}, _fnName{std::move(rhs._fnName)}
{}

Call& Call::operator=(const Call& other) {
  Expr::operator=(other);
  _fnName = other._fnName;
  return *this;
}

Call& Call::operator=(Call&& rhs) {
  Expr::operator=(std::move(rhs));
  _fnName = std::move(rhs._fnName);
  return *this;
}

const std::vector<Expr>& Call::params() const {
  return _exprs;
}

std::string_view Call::fnName() const { return _fnName; }

//------------------------------------------------
// actual parser down below

std::shared_ptr<Module> curModule;

void expect(
  std::string_view msg,
  std::shared_ptr<const Token> tok,
  LangType type)
{
  if (tok->type() != type)
    throw SyntaxError(msg.data(), curModule, tok->line(), tok->col());
}


Expr
parse_expr(
  std::vector<std::shared_ptr<const Token>>::iterator& tok,
  std::vector<std::shared_ptr<const Token>>::iterator& endTok,
  std::vector<std::string>& args,
  std::unordered_map<std::string, std::size_t>& func_def)
{
  auto beginTok = tok;
  auto type = (*tok)->type();
  std::vector<Expr> exprs;
  std::stringstream ss;
  std::cout << (*tok)->line() << " " <<  (*tok)->ident()<<'\n';

  if (tok == endTok)
    return Expr{*tok, LangType::__Finished, exprs};

  // handle one subthing expressions
  if (type >= LangType::Num && type <= LangType::Tail) {
    exprs.emplace_back(parse_expr(++tok, endTok, args, func_def));

  } else if (type >= LangType::Fuse &&
             type <= LangType::LessEq)
  { // handle 2
    exprs.emplace_back(parse_expr(++tok, endTok, args, func_def));
    exprs.emplace_back(parse_expr(++tok, endTok, args, func_def));

  } else if (type == LangType::If) {
    // handle 3 stuff
    exprs.emplace_back(parse_expr(++tok, endTok, args, func_def));
    exprs.emplace_back(parse_expr(++tok, endTok, args, func_def));
    exprs.emplace_back(parse_expr(++tok, endTok, args, func_def));
    ss << "Expected 'operator first second' as condition to if.";
  } else if (type >= LangType::Value && type <= LangType::Str) {
    return ExprValue{*beginTok, (*beginTok)->value()};
  } else if (type == LangType::Ident) {
    auto arg = std::find(args.begin(), args.end(), (*tok)->ident());
    if (arg != args.end()) {
      //found in argument params
      return ExprIdent(*tok, arg - args.begin());
    }
    // handle function names lookup
    auto fn = std::find_if(func_def.begin(), func_def.end(),
      [&](std::pair<std::string, std::size_t> func){
        return func.first == (*tok)->ident();
      });
    if (fn != func_def.end()) {
      // found in function definitions
      std::vector<Expr> params;
      for (std::size_t a; a < fn->second; a++) {
        Expr e = parse_expr(++tok, endTok, args, func_def);
        if (e.isFailed()) {
          ss << "Expected " << fn->second << " arguments in " << fn->first << " call.";
          break;
        }
        params.emplace_back(e);
      }
      return Call{*beginTok, params, fn->first};
    }

    // if not found throw error
    ss << "Function " << (*tok)->ident() << " not found.";
    throw SyntaxError(ss.str(), curModule, (*tok)->line(), (*tok)->col());
  }

  for (const auto& e : exprs)
    if (e.isFailed()) {
      auto s = ss.str();
      if (s.length())
        throw SyntaxError(s, curModule, (*beginTok)->line(), (*beginTok)->col());
      return Expr::mkFailed();
    }
  return Expr{*beginTok, type, exprs};
}


void parse(std::shared_ptr<Module> module, std::size_t fromTok) {
  curModule = module;
  std::unordered_map<std::string, std::size_t> func_defs;

  auto end = module->tokens().end();
  auto tok = module->tokens().begin();
  for(std::size_t i = 0; i < fromTok && tok != end; ++i)
    ++tok;

  for (; tok != end; ++tok) {
    std::vector<std::string> args;
    std::vector<Expr> exprs;
    auto tokBegin = tok;

    expect("Expected fn keyword.", *tok,  LangType::Fn);
    expect("Expected a function name.", *(++tok), LangType::Ident);
    auto fnName = (*tok)->ident();
    // arguments
    while ((*(++tok))->type() != LangType::Is) {
      expect("Expected a parameter.", *tok, LangType::Ident);
      args.emplace_back((*tok)->ident());
    }
    expect("Expected is keyword", *tok, LangType::Is);

    // store function definition before parsing function body
    // recursive function
    func_defs[fnName.data()] = args.size();
    auto expr = parse_expr(++tok, end, args, func_defs);
    module->funcs().emplace(
      std::pair<std::string, Func>{fnName.data(),
      Func{*tokBegin, args, expr}});
  }
}

} // namespace atto
