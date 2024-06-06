#include <vector>
#include "ast.hpp"
#include "parser.hpp"

#define DEBUG(x) do { std::cerr << x; } while (0)
//#define DEBUG(x)

using namespace atto;

AstBase::AstBase(const Token& tok, LangType type) :
  _tok{tok}, _type{type}
{}

AstBase::AstBase(
  const Token& tok,
  LangType type,
  std::vector<AstBasePtr>& children
) :
  _tok{tok}, _type{type}, _children{std::move(children)}
{}

/*AstBase::AstBase(const AstBase& other) :
  _tok{other._tok}, _type{other._type}, _children{other._children}
{}*/

AstBase::AstBase(AstBase&& rhs) :
  _tok{std::move(rhs._tok)},
  _type{std::move(rhs._type)},
  _children{std::move(rhs._children)}
{}

/*AstBase&
AstBase::operator=(const AstBase& other) {
  *const_cast<Token*>(&_tok) = other._tok;
  _type = other._type;
  _children = other._children;
  return *this;
}*/

AstBase&
AstBase::operator=(AstBase&& rhs) {
  *const_cast<Token*>(&_tok) = std::move(rhs._tok);
  _type = std::move(rhs._type);
  _children = std::move(rhs._children);
  return *this;
}

const Token&
AstBase::token() const { return _tok; }

LangType AstBase::type() const { return _type; }

const std::vector<AstBasePtr>&
AstBase::children() const {
  return _children;
}

void AstBase::addChildren(std::vector<AstBasePtr> children)
{
  for (auto& child : children)
    _children.emplace_back(std::move(child));
}

const AstBase&
AstBase::operator[](std::size_t idx) const {
  return *_children[idx];
}

// static
AstBase
AstBase::mkFailed()
{
  std::vector<AstBasePtr> children;
  auto tok = Token::mkFailure();
  return AstBase(tok, LangType::__Failure, children);
}

// --------------------------------------------

AstValue::AstValue(
  const Token& tok,
  const Value value
) :
  AstBase{tok, LangType::Value},
  _vlu{value}
{}

AstValue::AstValue(AstValue&& rhs) :
  AstBase{std::move(rhs)}, _vlu{std::move(rhs._vlu)}
{}

AstValue& AstValue::operator=(AstValue&& rhs) {
  AstBase::operator=(std::move(rhs));
  *const_cast<Value*>(&_vlu) = rhs._vlu;
  return *this;
}

const Value& AstValue::value() const { return _vlu; }

// -------------------------------------------------------------

AstIdent::AstIdent(
  const Token& tok,
  std::size_t localIdx
) :
  AstBase{tok, LangType::Ident},
  _localIdx{localIdx}
{}

/*AstIdent::AstIdent(const AstIdent& other):
  AstBase{other}, _localIdx{other._localIdx}
{}*/

AstIdent::AstIdent(AstIdent&& rhs) :
  AstBase{std::move(rhs)}, _localIdx{std::move(rhs._localIdx)}
{}

/*AstIdent& AstIdent::operator=(const AstIdent& other) {
  AstBase::operator=(other);
  _localIdx = other._localIdx;
  return *this;
}*/

AstIdent& AstIdent::operator=(AstIdent&& rhs) {
  AstBase::operator=(std::move(rhs));
  _localIdx = std::move(rhs._localIdx);
  return *this;
}

std::size_t AstIdent::localIdx() const { return _localIdx; }

// ------------------------------------------------------

AstFunc::AstFunc(
  const Token& tok,
  FuncParams args
) :
  AstBase{tok, LangType::Fn},
  _args{args}
{}

/*AstFunc::AstFunc(const AstFunc& other) :
  AstBase{other}, _args{other._args}
{}*/

AstFunc::AstFunc(AstFunc&& rhs) :
  AstBase{std::move(rhs)}, _args{std::move(_args)}
{}

/*AstFunc& AstFunc::operator=(const AstFunc& other) {
  AstBase::operator=(other);
  _args = other._args;
  return *this;
}*/

AstFunc& AstFunc::operator=(AstFunc&& rhs) {
  AstBase::operator=(std::move(rhs));
  _args = std::move(rhs._args);
  return *this;
}

const FuncParams& AstFunc::args() const {
  return _args;
}

std::string_view AstFunc::fnName() const
{
  return _tok.ident();
}

// -----------------------------------------------------------

AstCall::AstCall(
  const Token& tok,
  std::vector<AstBasePtr> params,
  std::string fnName,
  const Module& module
) :
  AstBase{tok, LangType::Call, params},
  _fnName{fnName},
  _module{module}
{ }

/*AstCall::AstCall(const AstCall& other) :
  AstBase{other}, _fnName{other._fnName}, _module{other._module}
{}*/

AstCall::AstCall(AstCall&& rhs) :
  AstBase{std::move(rhs)},
  _fnName{std::move(rhs._fnName)},
  _module{std::move(rhs._module)}
{}

/*AstCall& AstCall::operator=(const AstCall& other) {
  AstBase::operator=(other);
  _fnName = other._fnName;
  return *this;
}*/

AstCall& AstCall::operator=(AstCall&& rhs) {
  AstBase::operator=(std::move(rhs));
  _fnName = std::move(rhs._fnName);
  return *this;
}

const std::vector<AstBasePtr>& AstCall::params() const
{
  return _children;
}

std::string_view AstCall::fnName() const
{
  return _fnName;
}

const Module& AstCall::module() const
{
  return _module;
}

// static
AstCall AstCall::mkFailed()
{
  std::vector<AstBasePtr> params;
  auto tok = Token::mkFailure();
  AstCall fcall{tok, std::move(params), "bad", *curModule()};
  fcall._type = LangType::__Failure;
  return fcall;
}