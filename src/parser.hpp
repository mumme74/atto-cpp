#ifndef ATTO_PARSER_H
#define ATTO_PARSER_H

#include <memory>
#include <vector>
#include "common.hpp"
#include "lex.hpp"
#include "values.hpp"

namespace atto {

class Module;

class Expr
{
protected:
  std::shared_ptr<const Token> _tok;
  LangType _type;
  std::vector<std::shared_ptr<Expr>> _exprs;
  std::shared_ptr<Module> _module;
public:
  Expr(std::shared_ptr<const Token> tok,
       LangType type,
       std::vector<std::shared_ptr<Expr>> exprs);
  Expr(const Expr& other);
  Expr(Expr&& rhs);
  Expr& operator=(const Expr& other);
  Expr& operator=(Expr&& rhs);
  const Token& token() const;
  LangType type() const;
  bool isFailed() const;
  const std::vector<std::shared_ptr<Expr>>& exprs() const;
  const Expr& operator[](std::size_t idx) const;
  std::shared_ptr<Module> module() const;
  static Expr mkFailed();
};

class ExprValue : public Expr
{
  std::unique_ptr<Value> _vlu;
public:
  ExprValue(std::shared_ptr<const Token> tok, Value value);
  ExprValue(const ExprValue& other) = delete;
  ExprValue(ExprValue&& rhs);
  ExprValue& operator=(ExprValue&& rhs);
  ExprValue& operator=(const ExprValue& other) = delete;
  Value& value() const;
};

class ExprIdent : public Expr
{
  std::size_t _localIdx;
public:
  ExprIdent(std::shared_ptr<const Token> tok, std::size_t localIdx);
  ExprIdent(const ExprIdent& other);
  ExprIdent(ExprIdent&& rhs);
  ExprIdent& operator=(const ExprIdent& other);
  ExprIdent& operator=(ExprIdent&& rhs);
  std::size_t localIdx() const;
};

class Func : public Expr
{
  std::vector<std::string> _args;
public:
  Func(std::shared_ptr<const Token> tok,
       std::vector<std::string> args,
       std::vector<std::shared_ptr<Expr>> exprs);
  Func(const Func& other);
  Func(Func&& rhs);
  Func& operator=(const Func& other);
  Func& operator=(Func&& rhs);
  const std::vector<std::string>& args() const;
  std::string_view fnName() const;
};

class Call : public Expr
{
  std::string _fnName;
public:
  Call(std::shared_ptr<const Token> tok,
       std::vector<std::shared_ptr<Expr>> params,
       std::string fnName,
       std::shared_ptr<Module> module);
  Call(const Call& other);
  Call(Call&& rhs);
  Call& operator=(const Call& other);
  Call& operator=(Call&& rhs);
  const std::vector<std::shared_ptr<Expr>>& params() const;
  std::string_view fnName() const;
};

// ----------------------------------

// parses all tokens in module from startTok
void parse(std::shared_ptr<Module> module, std::size_t startTok = 0);

} // namespace atto

#endif // ATTO_PARSER_H
