#ifndef ATTO_ATTO_H
#define ATTO_ATTO_H

#include <string>
#include <string_view>
#include <vector>
#include <variant>
#include <filesystem>
#include <iostream>
#include <memory>

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
  std::vector<::Value> asList() const;

  bool isNull() const { return _type == ValueTypes::Null; }
  bool isBool() const { return _type == ValueTypes::Bool; }
  bool isStr() const { return _type == ValueTypes::Str; }
  bool isList() const { return _type == ValueTypes::List; }


  static Value from_str(std::string str);
  std::string_view into_str() const;
};

// ------------------------------------------------

class Atto
{
protected:
  std::filesystem::path _replHistoryPath;
  std::istream &_cin;
  std::ostream &_cout, &_cerr;
  std::string _core_code;
public:
  Atto(std::filesystem::path replHistoryPath = ".replHistory",
       std::istream &cin = std::cin,
       std::ostream &cout = std::cout,
       std::ostream &cerr = std::cerr);
  virtual ~Atto();

  void print(std::string_view msg) const;
  Value input(std::string_view msg) const;


  bool execFile(std::filesystem::path path);
  void repl();
protected:
  std::string readFile(std::filesystem::path, bool & success);
};

#endif // ATTO_ATTO_H
