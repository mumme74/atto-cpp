#ifndef ATTO_VALUES_H
#define ATTO_VALUES_H

#include <string_view>
#include <string>
#include <vector>
#include <variant>

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

  ValueTypes type() const;
  bool asBool() const;
  double asNum() const;
  std::string asStr() const;
  std::vector<Value> asList() const;

  bool isNull() const { return _type == ValueTypes::Null; }
  bool isBool() const { return _type == ValueTypes::Bool; }
  bool isStr() const { return _type == ValueTypes::Str; }
  bool isList() const { return _type == ValueTypes::List; }


  static Value from_str(std::string str);
  std::string_view into_str() const;

  static Value Null;
};

} // namespace atto

#endif // ATTO_VALUES_H