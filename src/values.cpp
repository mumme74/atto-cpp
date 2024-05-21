
#include <sstream>
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
    return std::get<bool>(_vlu) > std::get<bool>(other._vlu);
  case ValueTypes::List: return false;
  }
  return false;
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

Value Value::Null{};

} // namespace atto
