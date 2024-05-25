
#include <sstream>
#include <memory>
#include "values.hpp"
#include "common.hpp"

namespace atto {

Value::Value(const Value& other) :
  _type{other._type}, _vlu{other._vlu}
{}

Value::Value(const Token& tok)
{
  switch (tok.type()) {
  case LangType::True_litr: [[fallthrough]];
  case LangType::False_litr:
    _type = ValueTypes::Bool;
    _vlu = tok.value() == "true";
    break;
  case LangType::Num_litr:
    _type = ValueTypes::Num;
    _vlu = std::stod(std::string(tok.value()));
    break;
  case LangType::Str_litr:
    _type = ValueTypes::Str;
    _vlu = std::string(tok.value());
    break;
  case LangType::Null_litr:  [[fallthrough]];
  default:
    _type = ValueTypes::Null; break;
    _vlu = nullptr;
  }
}

Value::Value(Value&& rhs) :
  _type{std::move(rhs._type)}, _vlu{std::move(rhs._vlu)}
{}

Value::Value() :
  _type{ValueTypes::Null}, _vlu{}
{}

Value::Value(double value) :
  _type{ValueTypes::Num}, _vlu{value}
{}

Value::Value(bool value) :
  _type{ValueTypes::Bool}, _vlu{value}
{}

Value::Value(std::string_view value) :
  _type{ValueTypes::Str}, _vlu{std::string(value)}
{}

Value::Value(std::vector<Value> value) :
  _type{ValueTypes::List}, _vlu{value}
{}

Value::~Value() {}

Value& Value::operator=(const Value& other)
{
  _type = other._type;
  _vlu = other._vlu;
  return *this;
}

Value& Value::operator=(Value&& rhs)
{
  _type = std::move(rhs._type);
  _vlu = std::move(rhs._vlu);
  return *this;
}

bool Value::operator==(const Value& other) const
{
  if (_type != other._type) return false;
  switch (_type) {
  case ValueTypes::Null: return true;
  case ValueTypes::Bool:
    return std::get<bool>(_vlu) == std::get<bool>(other._vlu);
  case ValueTypes::Str:
    return std::get<std::string>(_vlu) == std::get<std::string>(other._vlu);
  case ValueTypes::Num:
    return std::get<double>(_vlu) == std::get<double>(other._vlu);
  case ValueTypes::List: return false;
  }
  return false;
}

bool Value::operator>(const Value& other) const
{
  if (_type != other._type) return false;
  switch (_type) {
  case ValueTypes::Null: return false;
  case ValueTypes::Bool: [[fallthrough]];
  case ValueTypes::Str: [[fallthrough]];
  case ValueTypes::Num:
    return std::get<double>(_vlu) > std::get<double>(other._vlu);
  case ValueTypes::List: return false;
  }
  return false;
}

bool Value::operator>=(const Value& other) const
{
  if (operator==(other))
    return true;
  return operator>(other);
}

Value Value::operator+(const Value& other) const
{
  if (_type == ValueTypes::Num && other._type == ValueTypes::Num) {
    return Value{std::get<double>(_vlu) + std::get<double>(other._vlu)};
  } else if (_type == ValueTypes::Str && other._type == ValueTypes::Str) {
    return Value{std::get<std::string>(_vlu) + std::get<std::string>(other._vlu)};
  }
  return Value::Null;
}

Value Value::operator-(const Value& other) const
{
  if (_type == ValueTypes::Num && other._type == ValueTypes::Num) {
    return Value{std::get<double>(_vlu) - std::get<double>(other._vlu)};
  }
  return Value::Null;
}

Value Value::operator/(const Value& other) const
{
  if (_type == ValueTypes::Num && other._type == ValueTypes::Num) {
    return Value{std::get<double>(_vlu) / std::get<double>(other._vlu)};
  }
  return Value::Null;
}

Value Value::operator*(const Value& other) const
{
  if (_type == ValueTypes::Num && other._type == ValueTypes::Num) {
    return Value{std::get<double>(_vlu) * std::get<double>(other._vlu)};
  }
  return Value::Null;
}

Value Value::operator%(const Value& other) const
{
  if (_type == ValueTypes::Num && other._type == ValueTypes::Num) {
    auto l = std::get<double>(_vlu), r = std::get<double>(other._vlu);
    long res = static_cast<long>(l) % static_cast<long>(r);
    return Value{static_cast<double>(res)};
  }
  return Value::Null;
}

Value Value::operator!() const
{
  switch (_type) {
  case ValueTypes::Bool:
    return Value{!std::get<bool>(_vlu)};
  case ValueTypes::Num:
    return Value{!std::get<double>(_vlu)};
  case ValueTypes::Str:
    return Value{std::get<std::string>(_vlu).size() > 0};
  case ValueTypes::List:
    return Value{std::get<std::vector<Value>>(_vlu).size() > 0};
  default:
    return Value{false};
  }
}

Value Value::neg() const
{
  switch (_type) {
  case ValueTypes::Num: return Value(-asNum());
  default: return Value::Null;
  }
}

ValueTypes Value::type() const
{
  return _type;
}

std::string_view Value::typeName() const
{
  switch (_type) {
  case ValueTypes::Null: return "Null";
  case ValueTypes::Bool: return "Bool";
  case ValueTypes::Str:  return "Str";
  case ValueTypes::Num:  return "Num";
  case ValueTypes::List: return "List";
  }
  return "_wrongType";
}


bool Value::asBool() const
{
  switch(_type) {
  case ValueTypes::Bool: return std::get<bool>(_vlu);
  case ValueTypes::Null: return false;
  case ValueTypes::Num:  return std::get<double>(_vlu) != 0.0;
  case ValueTypes::Str:  return std::get<std::string>(_vlu).size() > 0;
  case ValueTypes::List: return std::get<std::vector<Value>>(_vlu).size() > 0;
  }
  return false;
}

double Value::asNum() const
{
  switch(_type) {
  case ValueTypes::Bool: return std::get<bool>(_vlu) ? 1.0 : 0.0;
  case ValueTypes::Null: return 0.0;
  case ValueTypes::Num:  return std::get<double>(_vlu);
  case ValueTypes::Str:  return std::stod(std::get<std::string>(_vlu));
  case ValueTypes::List: return std::get<std::vector<Value>>(_vlu).size();
  }
  return 0.0;
}

std::vector<Value> Value::asList() const
{
  switch(_type) {
  case ValueTypes::Bool:
  case ValueTypes::Null:
  case ValueTypes::Num:  [[fallthrough]];
  case ValueTypes::Str:  return std::vector<Value>{*this};
  case ValueTypes::List: return std::get<std::vector<Value>>(_vlu);
  }
  return std::vector<Value>{};
}

std::string Value::asStr() const
{
  switch (_type) {
  case ValueTypes::Bool:
    return std::get<bool>(_vlu) ? "true" : "false";
  case ValueTypes::Null:
    return "null";
  case ValueTypes::Num:{
    // floating points i C++ is a mess, puh...
    auto vlu = std::to_string(std::get<double>(_vlu));
    auto dotPos = vlu.find_first_of('.');
    vlu.erase(vlu.find_last_not_of('0', dotPos) + 1, std::string::npos);
    if (vlu.size()-1 == dotPos)
      vlu.erase(dotPos, std::string::npos);
    return vlu;
  }
  case ValueTypes::Str:
    return std::get<std::string>(_vlu);
  case ValueTypes::List: {
    std::vector<std::string> parts;
    for (const auto& itm : std::get<std::vector<Value>>(_vlu))
      parts.emplace_back(itm.asStr());
    return std::string("[") + join(parts, ", ") + "]";
  }
  }
  return "";
}

const Value& Value::at(std::size_t idx) const
{
  if (isList()) {
    auto& l = std::get<std::vector<Value>>(_vlu);
    if (l.size() > idx)
      return l[idx];
  }
  return Value::Null;
}

Value Value::clone() const
{
  switch (_type) {
  case ValueTypes::Null: return Value::Null;
  case ValueTypes::Bool: return Value(std::get<bool>(_vlu));
  case ValueTypes::Num:  return Value(std::get<double>(_vlu));
  case ValueTypes::Str:  return Value(std::get<std::string>(_vlu));
  case ValueTypes::List: {
    const auto& me = std::get<std::vector<Value>>(_vlu);
    std::vector<Value> list;
    list.reserve(me.size());
    for(const auto& itm : me)
      list.emplace_back(itm.clone());
    return Value(list);
  }
  }
  return Value::Null;
}

Value Value::from_str(std::string str)
{
  str = trim(str);
  auto end = str.data()+str.size();
  if (str == "null") return Value();
  if (str == "true") return Value(true);
  if (str == "false") return Value(false);
  double dVlu = std::strtod(str.data(), &end);
  if (end == str.data()+str.size())
    // successfully converted
    return Value(dVlu);
  return Value(std::string(str));
}

std::shared_ptr<Value> Value::Null_ptr{new Value};
Value& Value::Null = *Null_ptr;

} // namespace atto
