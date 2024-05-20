#include "atto.hpp"
#include "lex.hpp"
#include "lib/linenoise.hpp"
#include <sstream>
#include <vector>
#include <utility>
#include <fstream>
#include <unordered_map>
#include <algorithm>

// private to this file
namespace fs = std::filesystem;

const char* ws = " \t\n\r\f\v";
std::string_view rtrim(std::string_view str) {
  return str.substr(0, str.find_last_not_of(ws));
}
std::string_view ltrim(std::string_view str) {
  return str.substr(str.find_first_not_of(ws));
}
std::string_view trim(std::string str) {
  return rtrim(ltrim(str));
}

std::vector<std::string> split(const std::string& str, const std::string& delim) {
  std::vector<std::string> parts;
  std::string::size_type pos = 0;
  std::string::size_type prev = 0;
  while ((pos = str.find(delim, prev)) && pos != std::string::npos) {
    parts.emplace_back(str.substr(prev, pos - prev));
    prev = pos + delim.length();
  }

  parts.emplace_back( str.substr(prev));
  return parts;
}

// --------------------------------------------

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
  std::stringstream s;
  switch (_type) {
  case ValueTypes::Bool:
    s << (std::get<bool>(_vlu) ? "true" : "false");
    break;
  case ValueTypes::List:
    for (const auto& v : std::get<std::vector<Value>>(_vlu))
      s << v.into_str();
    break;
  case ValueTypes::Null:
    s << "null";
    break;
  case ValueTypes::Num:
    s << std::get<double>(_vlu);
    break;
  case ValueTypes::Str:
    s << std::get<std::string>(_vlu);
    break;
  }
  return s.str();
}

static Value Null;

//------------------------------------------------

class Expr
{
protected:
  std::shared_ptr<Token> _tok;
  BuiltinTypes _type;
  std::vector<Expr> _exprs;
public:
  Expr(std::shared_ptr<Token> tok, BuiltinTypes type, std::vector<Expr> exprs) :
    _tok{tok}, _type{type}, _exprs(std::move(exprs))
  {}
  Expr(const Expr& other) :
    _tok{other._tok}, _type{other._type}, _exprs{other._exprs}
  {}
  Expr(Expr&& rhs) :
    _tok{std::move(rhs._tok)},
    _type{std::move(rhs._type)},
    _exprs{std::move(rhs._exprs)}
  {}
  Expr& operator=(const Expr& other) {
    _tok = other._tok;
    _type = other._type;
    _exprs = other._exprs;
    return *this;
  }
  Expr& operator=(Expr&& rhs) {
    _tok = std::move(rhs._tok);
    _type = std::move(rhs._type);
    _exprs = std::move(rhs._exprs);
    return *this;
  }
  const Token& token() const { return *_tok; }
  BuiltinTypes type() const { return _type; }
  bool isFailed() const { return _tok->type() == BuiltinTypes::__Failure; }
  const std::vector<Expr>& exprs() const { return _exprs; }
  const Expr& operator[](std::size_t idx) const { return _exprs[idx]; }
  static Expr mkFailed() { return Expr{nullptr, BuiltinTypes::__Failure, std::vector<Expr>{}}; }
};

class ExprValue : public Expr
{
  std::unique_ptr<Value> _vlu;
public:
  ExprValue(std::shared_ptr<Token> tok, Value value) :
    Expr{tok, BuiltinTypes::Value, std::vector<Expr>{}},
    _vlu{std::make_unique<Value>(value).release()}
  {}
  ExprValue(const ExprValue& other) = delete;
  ExprValue(ExprValue&& rhs) :
    Expr{rhs}, _vlu{std::move(rhs._vlu)}
  {}
  ExprValue& operator=(ExprValue&& rhs) {
    Expr::operator=(std::move(rhs));
    _vlu.reset(std::move(rhs._vlu.release()));
    return *this;
  }
  ExprValue& operator=(const ExprValue& other) = delete;

  Value& value() const { return _vlu ? *_vlu : Null; }
};

class ExprIdent : public Expr
{
  std::size_t _localIdx;
public:
  ExprIdent(std::shared_ptr<Token> tok, std::size_t localIdx) :
    Expr{tok, BuiltinTypes::Ident, std::vector<Expr>{}},
    _localIdx{localIdx}
  {}
  ExprIdent(const ExprIdent& other):
    Expr{other}, _localIdx{other._localIdx}
  {}
  ExprIdent(ExprIdent&& rhs) :
    Expr{std::move(rhs)}, _localIdx{std::move(rhs._localIdx)}
  {}
  ExprIdent& operator=(const ExprIdent& other) {
    Expr::operator=(other);
    _localIdx = other._localIdx;
    return *this;
  }
  ExprIdent& operator=(ExprIdent&& rhs) {
    Expr::operator=(std::move(rhs));
    _localIdx = std::move(rhs._localIdx);
    return *this;
  }

  std::size_t localIdx() const { return _localIdx; }
};

class Func : public Expr
{
  std::vector<std::string> _args;
public:
  Func(std::shared_ptr<Token> tok,
       std::vector<std::string> args,
       Expr expr) :
    Expr{tok, BuiltinTypes::Fn, std::vector<Expr>{expr}},
    _args{args}
  {}
  Func(const Func& other) :
    Expr{other}, _args{other._args}
  {}
  Func(Func&& rhs) :
    Expr{std::move(rhs)}, _args{std::move(_args)}
  {}
  Func& operator=(const Func& other) {
    Expr::operator=(other);
    _args = other._args;
    return *this;
  }
  Func& operator=(Func&& rhs) {
    Expr::operator=(std::move(rhs));
    _args = std::move(rhs._args);
    return *this;
  }
  const Expr& expr() const { return *_exprs.begin(); }
  const std::vector<std::string>& args() const { return _args; }
  bool isFailure() const { return _type == BuiltinTypes::__Failure; }
};

class Call : public Expr
{
  std::string _fnName;
public:
  Call(std::shared_ptr<Token> tok,
       std::vector<Expr> params,
       std::string fnName) :
    Expr{tok, BuiltinTypes::Call, params},
    _fnName{fnName}
  {}
  Call(const Call& other) :
    Expr{other}, _fnName{other._fnName}
  {}
  Call(Call&& rhs) :
    Expr{std::move(rhs)}, _fnName{std::move(rhs._fnName)}
  {}
  Call& operator=(const Call& other) {
    Expr::operator=(other);
    _fnName = other._fnName;
    return *this;
  }
  Call& operator=(Call&& rhs) {
    Expr::operator=(std::move(rhs));
    _fnName = std::move(rhs._fnName);
    return *this;
  }
  const std::vector<Expr>& params() const {
    return _exprs;
  }
  std::string_view fnName() const { return _fnName; }
};

// ----------------------------------

Expr
parse_expr(
  std::vector<std::shared_ptr<Token>>::iterator& tok,
  std::vector<std::shared_ptr<Token>>::iterator& endTok,
  std::vector<std::string>& args,
  std::unordered_map<std::string, std::size_t>& func_def)
{
  auto beginTok = tok;
  auto type = (*tok)->type();
  std::vector<Expr> exprs;
  std::stringstream ss;

  if (tok == endTok)
    return Expr{*tok, BuiltinTypes::__Finished, exprs};

  // handle one subthing expressions
  if (type >= BuiltinTypes::Num && type <= BuiltinTypes::Tail) {
    exprs.emplace_back(parse_expr(++tok, endTok, args, func_def));

  } else if (type >= BuiltinTypes::Fuse &&
             type <= BuiltinTypes::LessEq)
  { // handle 2
    exprs.emplace_back(parse_expr(++tok, endTok, args, func_def));
    exprs.emplace_back(parse_expr(++tok, endTok, args, func_def));

  } else if (type == BuiltinTypes::If) {
    // handle 3 stuff
    exprs.emplace_back(parse_expr(++tok, endTok, args, func_def));
    exprs.emplace_back(parse_expr(++tok, endTok, args, func_def));
    exprs.emplace_back(parse_expr(++tok, endTok, args, func_def));
    ss << "Expected 'operator first second' as condition to if.";
  } else if (type == BuiltinTypes::Value) {
    return ExprValue{*beginTok, (*beginTok)->value()};
  } else if (type == BuiltinTypes::Ident) {
    auto arg = std::find(args.begin(), args.end(), (*tok)->ident());
    if (arg != args.end()) {
      //found in argument params
      return ExprIdent(*tok, arg - args.begin());
    }
    // handle function names lookup
    auto fn = std::find_if(func_def.begin(), func_def.end(),
      [&](std::pair<std::string, std::size_t> func){
        return func.first == (*tok)->ident();
      });
    if (fn != func_def.end()) {
      // found in function definitions
      std::vector<Expr> params;
      for (int a; a < fn->second; a++) {
        Expr e = parse_expr(++tok, endTok, args, func_def);
        if (e.isFailed()) {
          ss << "Expected " << fn->second << " arguments in " << fn->first << " call.";
          break;
        }
        params.emplace_back(e);
      }
      return Call{*beginTok, params, fn->first};
    }

    // if not found throw error
    ss << "Function " << (*tok)->ident() << " not found.";
    throw SyntaxError(ss.str(), (*tok)->line(), (*tok)->col());
  }

  for (const auto& e : exprs)
    if (e.isFailed()) {
      auto s = ss.str();
      if (s.length())
        throw SyntaxError(s, (*beginTok)->line(), (*beginTok)->col());
      return Expr::mkFailed();
    }
  return Expr{*beginTok, type, exprs};
}

bool expect(
  std::string_view msg,
  std::shared_ptr<Token> tok,
  BuiltinTypes type)
{
  if (tok->type() != type)
    throw SyntaxError(msg.data(), tok->line(), tok->col());
}

std::unordered_map<std::string, Func>
parse_funcs(std::vector<std::shared_ptr<Token>>& tokens)
{
  std::unordered_map<std::string, Func> funcs;
  std::unordered_map<std::string, std::size_t> func_defs;
  auto end = tokens.end();
  for (auto tok = tokens.begin(); tok != end; ++tok) {
    std::vector<std::string> args;
    std::vector<Expr> exprs;
    auto tokBegin = tok;

    expect("Expected fn keyword.", *tok,  BuiltinTypes::Fn);
    expect("Expected a function name.", *(++tok), BuiltinTypes::Ident);
    auto fnName = (*tok)->ident();
    // arguments
    while ((*(++tok))->type() != BuiltinTypes::Is) {
      expect("Expected a parameter.", *tok, BuiltinTypes::Ident);
      args.emplace_back((*tok)->ident());
    }
    expect("Expected is keyword", *tok, BuiltinTypes::Is);

    // store function definition before parsing function body
    // recursive function
    func_defs[fnName.data()] = args.size();
    auto expr = parse_expr(++tok, end, args, func_defs);
    funcs.emplace(
      std::pair<std::string, Func>{fnName.data(),
      Func{*tok, args, expr}});
  }
  return funcs;
}

// -------------------------------------

Atto::Atto(std::filesystem::path replHistoryPath,
       std::istream &cin,
       std::ostream &cout,
       std::ostream &cerr) :
  _replHistoryPath{replHistoryPath},
  _cin{cin}, _cout{cout}, _cerr{cerr},
  _core_code{}
{
  linenoise::SetMultiLine(true);
  linenoise::LoadHistory(_replHistoryPath.c_str());

  bool success;
  _core_code = readFile("../atto/core.at", success);

  /*linenoise::SetCompletionCallback([](
    std::string editBuffer, std::vector<std::string>& completions)
  {
    for (const auto& comp : completions) {
      if (comp.substr(0, editBuffer.length()) == editBuffer)

    }
  });*/
}

Atto::~Atto()
{
  linenoise::SaveHistory(_replHistoryPath.c_str());
  linenoise::linenoiseAtExit();
}

void Atto::print(std::string_view msg) const
{
  _cout << msg;
}

Value Atto::input(std::string_view msg) const
{
  auto str = linenoise::Readline(msg.begin());
  linenoise::AddHistory(str.c_str());
  return Value(str);
}

bool Atto::execFile(std::filesystem::path path)
{
  std::string code;
  try {
    bool success = false;
    code = _core_code + readFile(path, success);
    if (success) {
      auto tokens = lex(code);
      auto funcs = parse_funcs(tokens);
      for (const auto& tok : tokens) {
        _cout << "line:" << tok->line() << " col: " << tok->col() << " " << tok->ident() << '\n';
      }
      for (const auto&f : funcs) {
        _cout << "func:" << f.first << " \n";
      }
      return true;
    }
  } catch (SyntaxError e) {
    auto lines = split(code, "\n");
    _cerr << "SyntaxError:\n"
          << e.what << " at line " << e.line << " col " << e.col << '\n'
          << lines[e.line-1] << '\n' << std::setw(e.col+1) << '^' << "\n";
  }
  return false;
}


void Atto::repl()
{
  _cout << "Welcome to the Atto prompt.\n"
        << "The core library is included by default.\n";

  std::string line;
  bool quit = false;
  while (!(quit = linenoise::Readline(">>", line)) && !quit) {
    linenoise::AddHistory(line.c_str());
    if (line == "quit()") break;
    auto tokens = lex(line);
    _cout << "got tokens\n";
  }
}

std::string Atto::readFile(fs::path path, bool& success)
{
  success = false;
  auto filestat = fs::status(path);
  if (!fs::exists(path)) {
    _cerr << "File: " << path << " does not exist.\n";
    return "";
  }
  if (!fs::is_regular_file(filestat)) {
    _cerr << "File: " << path << " is not a regular file.\n";
    return "";
  }

  auto perms = filestat.permissions();
  if ((perms & fs::perms::owner_read) == fs::perms::none ||
      (perms & fs::perms::group_read) == fs::perms::none ||
      (perms & fs::perms::others_read) == fs::perms::none)
  {
    _cerr << "Insufficient privileges to access file: " << path << ".\n";
    return "";
  }

  if (fs::is_empty(path)) {
    _cerr << "File: " << path << " is empty.";
    return "";
  }

  std::ifstream file;
  file.open(path, std::ios::in);
  if (file.is_open()) {
    std::stringstream ss;
    ss << file.rdbuf();
    file.close();
    success = true;
    return ss.str();
  }
  _cerr << "Failed to open file " << path << '\n';
  return "";
}
