#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <utility>
#include <sstream>
#include "parser.hpp"
#include "errors.hpp"
#include "modules.hpp"

//#define DEBUG(x) do { std::cerr << x; } while (0)
#define DEBUG(x)

namespace atto {

// the module being parsed
std::shared_ptr<Module> curModule;

Expr::Expr(
  std::shared_ptr<const Token> tok,
  LangType type,
  std::vector<std::shared_ptr<Expr>> exprs
) :
  _tok{tok}, _type{type}, _exprs{exprs},
  _module{curModule}
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
std::shared_ptr<const Token> Expr::tokenPtr() const { return _tok; }
LangType Expr::type() const { return _type; }

bool Expr::isFailed() const {
  return _tok->type() == LangType::__Failure;
}

const std::vector<std::shared_ptr<Expr>>& Expr::exprs() const {
  return _exprs;
}

const Expr& Expr::operator[](std::size_t idx) const {
  return *_exprs[idx];
}

std::shared_ptr<Module> Expr::module() const
{
  return _module;
}

// static
Expr Expr::mkFailed() {
  return Expr{nullptr, LangType::__Failure,
            std::vector<std::shared_ptr<Expr>>{}};
}


// --------------------------------------------

ExprValue::ExprValue(std::shared_ptr<const Token> tok, Value value) :
  Expr{tok, LangType::Value, std::vector<std::shared_ptr<Expr>>{}},
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
  Expr{tok, LangType::Ident, std::vector<std::shared_ptr<Expr>>{}},
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
      std::vector<std::string> args) :
  Expr{tok, LangType::Fn, std::vector<std::shared_ptr<Expr>>{}},
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

void Func::addExprs(std::vector<std::shared_ptr<Expr>> exprs)
{
  _exprs.insert(_exprs.end(), exprs.begin(), exprs.end());
}

const std::vector<std::string>& Func::args() const {
  return _args;
}

std::string_view Func::fnName() const
{
  return (_tok)->ident();
}
// -----------------------------------------------------------

Call::Call(
  std::shared_ptr<const Token> tok,
  std::vector<std::shared_ptr<Expr>> params,
  std::string fnName,
  std::shared_ptr<Module> module
) :
  Expr{tok, LangType::Call, params},
  _fnName{fnName}
{
  _module = module;
}

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

const std::vector<std::shared_ptr<Expr>>& Call::params() const {
  return _exprs;
}

std::string_view Call::fnName() const { return _fnName; }

//------------------------------------------------
// actual parser down below

void expect(
  std::string_view msg,
  std::shared_ptr<const Token> tok,
  LangType type)
{
  if (tok->type() != type)
    throw SyntaxError(msg.data(), curModule, tok->line(), tok->col());
}

std::shared_ptr<Expr> parse_expr(
  std::vector<std::shared_ptr<const Token>>::iterator& tok,
  std::vector<std::shared_ptr<const Token>>::iterator& endTok,
  Module::FuncDef& func_def, int depth = 0)
{
  auto beginTok = *tok;
  if ((*tok)->type() == LangType::Fn)
    return nullptr;

  auto type = (*tok)->type();
  std::vector<std::shared_ptr<Expr>> exprs;
  std::stringstream ss;
  DEBUG((*tok)->line() << " " <<  (*tok)->ident() <<" enter: "<<depth<<'\n');

  if (tok == endTok)
    return std::make_shared<Expr>(*tok, LangType::__Finished, exprs);

  // handle one sub thing expressions
  if (type >= LangType::List && type <= LangType::Tail) {
    exprs.emplace_back(parse_expr(++tok, endTok, func_def, depth+1));
    DEBUG("Leave single out '"<<beginTok->ident()<<"' "<<depth << "\n");

  } else if (type >= LangType::Fuse &&
             type <= LangType::LessEq)
  { // handle 2
    exprs.emplace_back(parse_expr(++tok, endTok, func_def, depth+1));
    exprs.emplace_back(parse_expr(++tok, endTok, func_def, depth+1));
    DEBUG("Leave 2 stuff '"<<beginTok->ident()<<"' "<<depth<<"\n");

  } else if (type == LangType::If) {
    // handle if stuff
    exprs.emplace_back(parse_expr(++tok, endTok, func_def, depth+1));
    exprs.emplace_back(parse_expr(++tok, endTok, func_def, depth+1));
    exprs.emplace_back(parse_expr(++tok, endTok, func_def, depth+1));
    DEBUG("leave '"<<beginTok->ident()<<"'"<<depth<<" \n");
    ss << "Expected 'operator first second' as condition to if.";
  } else if (type >= LangType::Value && type <= LangType::Str_litr) {
    DEBUG("Leave epsilon '"<<beginTok->ident()<<"' " << depth << "\n");
    return std::make_shared<ExprValue>(beginTok, Value(*beginTok));
  } else if (type == LangType::Ident) {
    auto& args = func_def.second;
    auto arg = std::find(args.begin(), args.end(), (*tok)->ident());
    if (arg != args.end()) {
      //found in argument params
      DEBUG("found '" << (*tok)->ident() << "' in args\n");
      return std::make_shared<ExprIdent>(*tok, arg - args.begin());
    }
    // handle function names lookup
    auto lookupFn = [&](
      std::vector<std::shared_ptr<const Token>>::iterator& tok,
      std::shared_ptr<Module> module
    ){
      std::unordered_map<std::string, Module::FuncDef>&
        func_defs = module->funcs();
      auto fn = std::find_if(func_defs.begin(), func_defs.end(),
      [&](const auto& func){
        return func.first == (*tok)->ident();
      });
      if (fn != func_defs.end()) {
        // found in function definitions
        std::vector<std::shared_ptr<Expr>> params;
        const auto& args = fn->second.second;
        DEBUG("args for '" << (*tok)->ident() << "' no args:" << args.size() << '\n');
        for (const auto& _ : args) {
          DEBUG("get arg "<< _ <<"\n"); (void)_;
          auto expr = parse_expr(++tok, endTok, func_def, depth+1);
          if (!expr || expr->isFailed()) {
            ss << "Expected " << args.size() << " arguments in " << fn->first << " call.";
            throw ParseError(ss.str(), curModule, (*tok)->line(), (*tok)->col());
          }
          params.emplace_back(expr);
        }
        return std::make_shared<Call>(beginTok, params, fn->first, module);
      }
      return std::shared_ptr<Call>{};
    };
    // search in core or this module
    const auto core = lookupFn(
      tok, Module::allModules["__core__"]);
    if (core) return core;
    const auto thisMod = lookupFn(tok, curModule);
    if (thisMod) return thisMod;

    // search in imported modules
    for (const auto& mod : curModule->imported) {
      auto found = lookupFn(tok, mod.second);
      if (found) return found;
    }

    // if not found throw error
    ss << "Function " << (*tok)->ident() << " not found.";
    throw ParseError(ss.str(), curModule, (*tok)->line(), (*tok)->col());
  }

  // make sure sure we have expected sub exprs
  for (const auto& e : exprs) {
    if (!e || e->isFailed()) {
      auto s = ss.str();
      if (s.length())
        throw SyntaxError(s, curModule, beginTok->line(), beginTok->col());
      return nullptr;
    }
  }
  return std::make_shared<Expr>(beginTok, type, exprs);
}

void parse(std::shared_ptr<Module> module, std::size_t fromTok) {
  curModule = module;

  auto end = module->tokens().end();
  auto tok = module->tokens().begin();
  for(std::size_t i = 0; i < fromTok && tok != end; ++i)
    ++tok;

  std::unordered_map<std::string, std::shared_ptr<const Token>> funcExprsStarts;

  for (; tok != end; ++tok) {
    expect("Expected 'fn' keyword.", *tok,  LangType::Fn);
    auto tokFnName = ++tok;

    expect("Expected function name.", *tokFnName, LangType::Ident);
    std::string fnName{(*tokFnName)->ident()};
    std::vector<std::string> args;

    // arguments
    while ((*(++tok))->type() != LangType::Is) {
      expect("Expected parameter.", *tok, LangType::Ident);
      args.emplace_back((*tok)->ident());
    }
    expect("Expected 'is' keyword", *tok, LangType::Is);

    // store function definition before parsing function body
    // recursive function
    module->funcs().emplace(
      std::pair<std::string, Module::FuncDef>{
          fnName, std::pair{
            std::make_unique<Func>(*tokFnName, args), args
          }
        }
    );
    DEBUG("defining fn '" << fnName << " " << join(args, " ") << "'\n");

    funcExprsStarts.emplace(
      std::pair<std::string, std::shared_ptr<const Token>>(
        fnName, *(++tok)));

    // move to next fn
    while ((tok+1) != end && (*(tok+1))->type() != LangType::Fn)
      ++tok;
  }

  // now that all functions has been defined, parse them
  // we must define them before parse to make sure we have the signatures
  for (auto& fnItm : module->funcs()) {
    const auto& fnName = fnItm.first;
    auto& func_def = module->funcs()[fnName];
    DEBUG("parsing fn '" << fnName << "'\n");
    auto& tokens = module->tokens();
    auto tok = std::find_if(tokens.begin(), tokens.end(),
      [&](std::shared_ptr<const Token> tok) {
        return *tok == *funcExprsStarts[fnName];
      });
    std::vector<std::shared_ptr<Expr>> fnExprs;
    do {
      auto expr = parse_expr(tok, end, func_def);
      fnExprs.emplace_back(expr);
    } while (++tok != end && (*tok)->type() != LangType::Fn);

    func_def.first->addExprs(fnExprs);
  }
}

} // namespace atto
