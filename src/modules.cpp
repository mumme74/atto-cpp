#include "modules.hpp"
#include "common.hpp"
#include "errors.hpp"
#include <algorithm>

using namespace atto;

Module::Module(std::filesystem::path path, const std::string& code) :
  _path{path}, _code{code}, _tokens{},
  _imported{}, _funcs{}, _parsed{false}
{}

Module::Module(std::filesystem::path path) :
  Module(path, std::string(""))
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

Module::~Module() { }

// static
void Module::parse(Module &mod, std::string modName)
{
  if (modName.size() == 0)
    modName = mod._path.stem();
  Module::_allModules[modName] = mod;
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

const std::vector<const Token>&
Module::tokens() const
{
  return _tokens;
}


const std::unordered_map<std::string, Module::FuncDef>&
Module::funcs() const
{
  return _funcs;
}

const Func&
Module::func(const std::string& fn) const
{
  return *_funcs.at(fn).first;
}

bool Module::hasFunc(const std::string& fn) const
{
  auto found = std::find_if(_funcs.begin(), _funcs.end(), [&](auto it){
    return it->first == fn;
  });
  return found != _funcs.end();
}

void Module::import(std::filesystem::path path)
{
  if (path.is_relative()){
    auto p = _path.parent_path();
    path = p / path;
  }

  // already imported?
  auto found = std::find(
    Module::_allModules.begin(),
    Module::_allModules.end(),
    path.filename());

  if (found != Module::_allModules.end()) {
    _imported.emplace_back(found->first);
    return;
  }

  bool success = false;
  const auto code = readFile(path, success);
  Module newMod(path, code);
  Module::parse(newMod, path.stem());
  Module::_allModules[path.stem()] = std::move(newMod);
  _imported.emplace_back(path.stem());
}

const std::vector<const std::string>&
Module::imported() const
{
  return _imported;
}

//static
std::unordered_map<std::string, Module> Module::_allModules{};

// static
std::vector<const std::string> Module::allModuleNames()
{
  std::vector<const std::string> names;
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

  Module::_allModules[name] = std::move(Module(path));
}
