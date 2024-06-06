#ifndef ATTO_VALUES_H
#define ATTO_VALUES_H

#include <string_view>
#include <string>
#include <vector>
#include <variant>
#include <memory>
#include "lex.hpp"

namespace atto {

/// @brief All different types a Value can have
enum class ValueTypes {
  Num, Str, Bool, List, Null
};

/**
 * @brief The value class, all values in wm comes from here
 */
class Value {
protected:
  ValueTypes _type;
  std::variant<
    double, std::string, bool, std::vector<Value>, void*> _vlu;
public:
  Value(const Value& other);
  Value(const Token& tok);
  Value(Value&& rhs);
  /// Create a null value
  Value(); // null
  /// Create a Number value
  Value(double value);
  /// Create a boolean value
  Value(bool value);
  /// create a string Value
  Value(std::string_view value);
  /// create a list value
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
  /// make a number negative +1 => -1
  Value neg() const;

  /// what type this value has
  ValueTypes type() const;
  /// the typename of this value
  std::string_view typeName() const;
  /// get value as bool
  bool asBool() const;
  /// get value as number
  double asNum() const;
  /// get value as string
  std::string asStr() const;
  /// get values as list
  std::vector<Value> asList() const;
  /// get the value at index in a list
  const Value& at(std::size_t idx) const;
  /// clone this value
  Value clone() const;

  bool isNull() const { return _type == ValueTypes::Null; }
  bool isNum()  const { return _type == ValueTypes::Num; }
  bool isBool() const { return _type == ValueTypes::Bool; }
  bool isStr()  const { return _type == ValueTypes::Str; }
  bool isList() const { return _type == ValueTypes::List; }

  /// read value from a string suh as token from source code
  static Value from_str(std::string str);

  /// a global Null value
  static Value& Null;
  static std::shared_ptr<Value> Null_ptr;
};

} // namespace atto

#endif // ATTO_VALUES_H
