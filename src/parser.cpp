#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <utility>
#include <sstream>
#include "parser.hpp"
#include "errors.hpp"
#include "modules.hpp"
#include "ast.hpp"

#define DEBUG(x) do { std::cerr << x; } while (0)
//#define DEBUG(x)

namespace atto {

// the module being parsed
static Module* _curModule;



//------------------------------------------------
// actual parser down below

void expect(
  std::string_view msg,
  const Token& tok,
  LangType type)
{
  if (tok.type() != type)
    throw SyntaxError(msg.data(), *_curModule, tok);
}

AstBasePtr parse_expr(
  std::vector<Token>::const_iterator& tok,
  std::vector<Token>::const_iterator& endTok,
  FuncDef& func_def, int depth = 0)
{
  auto beginTok = *tok;
  if (tok == endTok || tok->type() == LangType::Fn)
    return nullptr;

  auto type = tok->type();
  std::vector<AstBasePtr> children;
  std::stringstream ss;
  DEBUG(tok->line() << " " <<  tok->ident() <<" enter: "<<depth<<'\n');

  if (tok == endTok)
    return std::make_unique<AstBase>(*tok, LangType::__Finished, children);

  // handle one sub thing expressions
  if (type >= LangType::List && type <= LangType::Tail) {
    children.emplace_back(parse_expr(++tok, endTok, func_def, depth+1));
    DEBUG("Leave single out '"<<beginTok.ident()<<"' "<<depth << "\n");

  } else if (type >= LangType::Fuse &&
             type <= LangType::LessEq)
  { // handle 2
    children.emplace_back(parse_expr(++tok, endTok, func_def, depth+1));
    children.emplace_back(parse_expr(++tok, endTok, func_def, depth+1));
    DEBUG("Leave 2 stuff '"<<beginTok.ident()<<"' "<<depth<<"\n");

  } else if (type == LangType::If) {
    // handle if stuff
    children.emplace_back(parse_expr(++tok, endTok, func_def, depth+1));
    children.emplace_back(parse_expr(++tok, endTok, func_def, depth+1));
    children.emplace_back(parse_expr(++tok, endTok, func_def, depth+1));
    DEBUG("leave '"<<beginTok.ident()<<"'"<<depth<<" \n");
    ss << "Expected 'operator first second' as condition to if.";

  } else if (type >= LangType::Value && type <= LangType::Str_litr) {
    DEBUG("Leave epsilon '"<<beginTok.ident()<<"' " << depth << "\n");
    return std::make_unique<AstValue>(beginTok, Value(beginTok));

  } else if (type == LangType::Ident) {
    auto& args = func_def.second;
    auto arg = std::find(args.begin(), args.end(), tok->ident());
    if (arg != args.end()) {
      //found in argument params
      DEBUG("found '" << tok->ident() << "' in args\n");
      return std::make_unique<AstIdent>(*tok, arg - args.begin());
    }
    // handle function names lookup
    auto lookupFn = [&] (
      std::vector<Token>::const_iterator& tok,
      Module& module
    ) -> AstBasePtr {
      const std::unordered_map<std::string, FuncDef>&
        func_defs = module.funcs();

      auto fn = std::find_if(func_defs.begin(), func_defs.end(),
        [&](const auto& func) -> bool {
          return func.first == tok->ident();
        }
      );

      if (fn != func_defs.end()) {
        // found in function definitions
        std::vector<AstBasePtr> params;
        const auto& args = fn->second.second;
        const auto& callTok = *tok;
        DEBUG("args for '" << callTok.ident() << "' num args:" << args.size() << '\n');
        for (const auto& _ : args) {
          DEBUG("get arg "<< _ <<"\n"); (void)_;
          auto expr = parse_expr(++tok, endTok, func_def, depth+1);
          if (!expr || expr->isFailed()) {
            ss << "Expected " << args.size() << " arguments in " << fn->first << " call.";
            throw ParseError(ss.str(), *_curModule, callTok);
          }
          params.emplace_back(std::move(expr));
        }
        return std::make_unique<AstCall>(
          beginTok, std::move(params), fn->first, module);
      }
      return nullptr;
    };

    // search in this module and in core in that order
    auto thisMod = lookupFn(tok, *_curModule);
    if (thisMod) return thisMod;

    auto core = lookupFn(tok, Module::module("__core__"));
    if (core) return core;

    // search in imported modules
    for (const auto& mod : _curModule->imported()) {
      auto found = lookupFn(tok, _curModule->module(mod));
      if (found) return found;
    }

    // if not found throw error
    ss << "Function " << tok->ident() << " not found.";
    throw ParseError(ss.str(), *_curModule, *tok);
  }

  // make sure sure we have expected sub children
  for (auto& ast : children) {
    if (!ast || ast->isFailed()) {
      auto s = ss.str();
      if (s.length())
        throw SyntaxError(s, *_curModule, beginTok);
      return nullptr;
    }
  }
  return std::make_unique<AstBase>(beginTok, type, children);
}

void parse(Module& module, std::size_t fromTok) {

  auto end = module.tokens().end();
  auto tok = module.tokens().begin();
  for(std::size_t i = 0; i < fromTok && tok != end; ++i)
    ++tok;

  std::unordered_map<std::string, const Token*> funcExprsStarts;

  for (; tok != end; ++tok) {
    while (tok->type() == LangType::Import) {
      const std::filesystem::path path = (++tok)->value();
      module.import(path);
      ++tok;
    }
    // set back to our main module after imports are done
    _curModule = &module;

    expect("Expected 'fn' keyword.", *tok,  LangType::Fn);
    auto tokFnName = ++tok;

    expect("Expected function name.", *tokFnName, LangType::Ident);
    std::string fnName{tokFnName->ident()};
    std::vector<std::string> args;

    // arguments
    while ((++tok)->type() != LangType::Is) {
      expect("Expected parameter.", *tok, LangType::Ident);
      args.emplace_back(tok->ident());
    }
    expect("Expected 'is' keyword", *tok, LangType::Is);

    // store function definition before parsing function body
    // recursive function
    FuncDef funcDef{std::make_unique<AstFunc>(*tokFnName, args), args};
    module.addFunc(fnName, funcDef);
    DEBUG("defining fn '" << fnName << " " << join(args, " ") << "'\n");

    funcExprsStarts.emplace(
      std::pair<std::string, const Token*>(fnName, &*(++tok)));

    // move to next fn
    while ((tok+1) != end && (tok+1)->type() != LangType::Fn)
      ++tok;
  }

  // now that all functions has been defined, parse them
  // we must define them before parse to make sure we have the signatures
  for (auto& fnItm : module.funcs()) {
    const auto& fnName = fnItm.first;
    auto& funcDef = module.funcDef(fnName);
    DEBUG("parsing fn '" << fnName << "'\n");
    auto& tokens = module.tokens();
    auto tok = std::find_if(tokens.begin(), tokens.end(),
      [&](const Token& tok) {
        return tok == *funcExprsStarts[fnName];
      });
    std::vector<AstBasePtr> fnExprs;
    do {
      auto expr = parse_expr(tok, end, funcDef);
      if (expr) fnExprs.emplace_back(std::move(expr));
    } while (++tok != end && tok->type() != LangType::Fn);

    // we want it as a const normally,
    // but we have to add children after construction
    auto func = const_cast<AstFunc*>(&*funcDef.first);
    func->addChildren(std::move(fnExprs));
  }
}

Module* curModule()
{
  return _curModule;
}

} // namespace atto
