#include "modules.hpp"
#include "common.hpp"
#include "errors.hpp"
#include "lex.hpp"
#include "parser.hpp"
#include <algorithm>

using namespace atto;

Module::Module(std::filesystem::path path, const std::string& code) :
  _path{path}, _code{code}, _tokens{},
  _imported{}, _funcs{}, _parsed{false}
{}

Module::Module(std::filesystem::path path) :
  Module{path, std::string("")}
{
  if (!path.empty()) {
    bool success = false;
    _code = readFile(path, success);
    if (!success)
      throw new FileIOError(
        std::string("File " + path.string() +
                    " not loaded.\n" +
                    "Check that it exists and has readable permissions"),
        *this, path);
  }
}

Module::Module(const std::string& code) :
  Module{"", code}
{}

Module::Module():
  _path{}, _code{},
  _tokens{}, _imported{},
  _funcs{}, _parsed{}
{}

/*Module::Module(const Module& other):
  _path{other._path}, _code{other._code},
  _tokens{other._tokens}, _imported{other._imported},
  _funcs{other._funcs}, _parsed{other._parsed}
{}*/

Module::Module(Module&& rhs):
  _path{std::move(rhs._path)}, _code{std::move(rhs._code)},
  _tokens{std::move(rhs._tokens)}, _imported{std::move(rhs._imported)},
  _funcs{std::move(rhs._funcs)}, _parsed{std::move(rhs._parsed)}
{}

Module::~Module() { }

/*Module& Module::operator=(const Module& other)
{
  _path = other._path;
  _code = other._code;
  _tokens = other._tokens;
  _imported = other._imported;
  _funcs = other._funcs;
  return *this;
}*/

Module& Module::operator=(Module&& rhs)
{
  _path = std::move(rhs._path);
  _code = std::move(rhs._code);
  _tokens = std::move(rhs._tokens);
  _imported = std::move(rhs._imported);
  _funcs = std::move(rhs._funcs);
  return *this;
}

// static
void Module::parse(Module &mod, std::string modName)
{
  if (modName.size() == 0)
    modName = mod._path.stem();
  // we can only have one in memory, take ownership
  Module::_allModules[modName] = std::move(mod);
  atto::lex(mod);
  atto::parse(mod);
  mod._parsed = true;
}

bool Module::isParsed() const
{
  return _parsed;
}

const std::filesystem::path& Module::path() const
{
  return _path;
}

std::string_view Module::code() const
{
  return _code;
}

void Module::appendCode(const std::string code)
{
  auto prevEnd = _code.size();
  _code += code;
  if (_parsed) {
    atto::lex(*this, prevEnd);
    atto::parse(*this);
  }
}

void Module::addToken(const Token &tok)
{
  _tokens.emplace_back(std::move(tok));
}

const std::vector<Token>&
Module::tokens() const
{
  return _tokens;
}


const FuncMap&
Module::funcs() const
{
  return _funcs;
}

const AstFunc&
Module::func(const std::string& fn) const
{
  return *_funcs.at(fn).first.get();
}

FuncDef&
Module::funcDef(const std::string& fn)
{
  return _funcs.at(fn);
}

const FuncParams&
Module::funcParams(const std::string& fn) const
{
  return _funcs.at(fn).second;
}

bool Module::hasFunc(const std::string& fn) const
{

  for (const auto& f : _funcs)
    if (f.first == fn)
      return true;
  return false;
}

void Module::addFunc(const std::string& fn, FuncDef& def)
{
  _funcs.emplace(std::pair<std::string, FuncDef>{fn, std::move(def)});
}

void Module::import(std::filesystem::path path)
{
  if (path.is_relative()){
    auto p = _path.parent_path();
    path = p / path;
  }

  // already imported?
  for (const auto& m : Module::_allModules)
    if (m.second.path() == path) return;

  bool success = false;
  const auto code = readFile(path, success);
 // we can oly have one in memory, store it first, then lookup
  Module::_allModules.emplace(
    std::pair<std::string, Module>{path.stem(), Module{path}});
  auto& mod = Module::_allModules.at(path.stem());
  Module::parse(mod);
  _imported.emplace_back(path.stem());
}

const std::vector<std::string>&
Module::imported() const
{
  return _imported;
}

//static
std::unordered_map<std::string, Module> Module::_allModules{};

// static
std::vector<std::string> Module::allModuleNames()
{
  std::vector<std::string> names;
  for (const auto& [name, _] : Module::_allModules)
    names.emplace_back(name);
  return names;
}

// static
Module& Module::module(const std::string& name,
                       const std::filesystem::path path /* = "" */)
{
  auto found = Module::_allModules.find(name);
  if (found != Module::_allModules.end())
    return found->second;

  // we can oly have one in memory, store it first, then lookup
  Module::_allModules.emplace(
    std::pair<std::string, Module>{name, Module{path}});
  auto& mod = Module::_allModules.at(name);
  atto::lex(mod);
  atto::parse(mod);
  mod._parsed = true;

  return mod;
}
