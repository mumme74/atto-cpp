#ifndef ATTO_AST_H
#define ATTO_AST_H

#include <vector>
#include <unordered_map>
#include <memory>
#include "common.hpp"
#include "lex.hpp"
#include "values.hpp"


namespace atto {

class AstFunc;
class AstBase;
class AstCall;

//! All functions in module should have this type
using AstBasePtr = std::unique_ptr<const AstBase>;
using AstFuncPtr = std::unique_ptr<const AstFunc>;
using AstCallPtr = std::unique_ptr<const AstCall>;

using FuncParams = std::vector<std::string>;
using FuncDef = std::pair<AstFuncPtr, FuncParams>;
using FuncMap = std::unordered_map<std::string, FuncDef>;

class AstBase
{
protected:
  const Token&  _tok;
  LangType _type;
  std::vector<AstBasePtr> _children;
public:
  AstBase(const Token& tok, LangType type);
  AstBase(const Token& tok,
       LangType type,
       std::vector<AstBasePtr>& children);
  AstBase(const AstBase& other) = delete;
  AstBase(AstBase&& rhs);
  AstBase& operator=(const AstBase& other) = delete;
  AstBase& operator=(AstBase&& rhs);
  const Token& token() const;
  LangType type() const;
  bool isFailed() const { return _type == LangType::__Failure; }
  const AstBase& operator[](std::size_t idx) const;
  const std::vector<AstBasePtr>& children() const;
  void addChildren(std::vector<AstBasePtr> children);

  static AstBase mkFailed();
};

class AstValue : public AstBase
{
  const Value _vlu;
public:
  AstValue(const Token& tok, const Value value);
  AstValue(const AstValue& other) = delete;
  AstValue(AstValue&& rhs);
  AstValue& operator=(AstValue&& rhs);
  AstValue& operator=(const AstValue& other) = delete;
  const Value& value() const;
};

class AstIdent : public AstBase
{
  std::size_t _localIdx;
public:
  AstIdent(const Token& tok, std::size_t localIdx);
  AstIdent(const AstIdent& other) = delete;
  AstIdent(AstIdent&& rhs);
  AstIdent& operator=(const AstIdent& other);
  AstIdent& operator=(AstIdent&& rhs);
  std::size_t localIdx() const;
};

class AstFunc : public AstBase
{
  FuncParams _args;
public:
  AstFunc(const Token& tok,
       FuncParams args);
  AstFunc(const AstFunc& other) = delete;
  AstFunc(AstFunc&& rhs);
  AstFunc& operator=(const AstFunc& other);
  AstFunc& operator=(AstFunc&& rhs);
  const FuncParams& args() const;
  std::string_view fnName() const;
};

class AstCall : public AstBase
{
  std::string _fnName;
  const Module& _module;
public:
  AstCall(const Token& tok,
       std::vector<AstBasePtr> params,
       std::string fnName,
       const Module& module);
  AstCall(const AstCall& other) = delete;
  AstCall(AstCall&& rhs);
  AstCall& operator=(const AstCall& other);
  AstCall& operator=(AstCall&& rhs);
  const std::vector<AstBasePtr>& params() const;
  std::string_view fnName() const;
  const Module& module() const;

  static AstCall mkFailed();
};

} // end namspace atto

#endif // ATTO_AST_H
