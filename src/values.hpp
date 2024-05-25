#ifndef ATTO_VALUES_H
#define ATTO_VALUES_H

#include <string_view>
#include <string>
#include <vector>
#include <variant>
#include <memory>
#include "lex.hpp"

namespace atto {

enum class ValueTypes {
  Num, Str, Bool, List, Null
};

class Value {
protected:
  ValueTypes _type;
  std::variant<
    double, std::string, bool, std::vector<Value>, void*> _vlu;
public:
  Value(const Value& other);
  Value(const Token& tok);
  Value(Value&& rhs);
  Value(); // null
  Value(double value);
  Value(bool value);
  Value(std::string_view value);
  Value(std::vector<Value> value);
  virtual ~Value();

  Value& operator=(const Value& other);
  Value& operator=(Value&& rhs);
  bool operator==(const Value& other) const;
  bool operator>(const Value& other) const;
  bool operator>=(const Value& other) const;
  Value operator+(const Value& other) const;
  Value operator-(const Value& other) const;
  Value operator/(const Value& other) const;
  Value operator*(const Value& other) const;
  Value operator%(const Value& other) const;
  Value operator!() const;
  Value neg() const;

  ValueTypes type() const;
  std::string_view typeName() const;
  bool asBool() const;
  double asNum() const;
  std::string asStr() const;
  std::vector<Value> asList() const;

  const Value& at(std::size_t idx) const;

  Value clone() const;

  bool isNull() const { return _type == ValueTypes::Null; }
  bool isBool() const { return _type == ValueTypes::Bool; }
  bool isStr() const { return _type == ValueTypes::Str; }
  bool isList() const { return _type == ValueTypes::List; }


  static Value from_str(std::string str);
  std::string_view into_str() const;

  static Value& Null;
  static std::shared_ptr<Value> Null_ptr;
};

} // namespace atto

#endif // ATTO_VALUES_H
