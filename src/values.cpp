
#include <sstream>
#include <memory>
#include "values.hpp"
#include "common.hpp"

namespace atto {

Value::Value(const Value& other) :
  _type{other._type}, _vlu{other._vlu}
{}

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
  case ValueTypes::Bool: [[fallthrough]];
  case ValueTypes::Str: [[fallthrough]];
  case ValueTypes::Num:
    return std::get<bool>(_vlu) == std::get<bool>(other._vlu);
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
  if (_type == ValueTypes::Num && other._type == ValueTypes::Num){
    return Value{std::get<double>(_vlu) + std::get<double>(_vlu)};
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
  case ValueTypes::Bool: [[fallthrough]];
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

ValueTypes Value::type() const
{
  return _type;
}

std::string Value::asStr() const
{
  switch (_type) {
  case ValueTypes::Bool:
    return std::get<bool>(_vlu) ? "true" : "false";
  case ValueTypes::Null:
    return "null";
  case ValueTypes::Num:
    return std::to_string(std::get<double>(_vlu));
  case ValueTypes::Str:
    return std::get<std::string>(_vlu);
  case ValueTypes::List: {
    std::vector<std::string> parts;
    for (const auto& itm : std::get<std::vector<Value>>(_vlu))
      parts.emplace_back(itm.asStr());
    return join(parts);
  }
  }
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

std::string_view Value::into_str() const
{
  std::stringstream ss;
  switch (_type) {
  case ValueTypes::Bool:
    ss << (std::get<bool>(_vlu) ? "true" : "false");
    break;
  case ValueTypes::List:
    for (const auto& v : std::get<std::vector<Value>>(_vlu))
      ss << v.into_str();
    break;
  case ValueTypes::Null:
    ss << "null";
    break;
  case ValueTypes::Num:
    ss << std::get<double>(_vlu);
    break;
  case ValueTypes::Str:
    ss << std::get<std::string>(_vlu);
    break;
  }
  return ss.str();
}

std::shared_ptr<Value> Value::Null_ptr{new Value};
Value& Value::Null = *Null_ptr;

} // namespace atto
