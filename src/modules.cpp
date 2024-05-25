#include "modules.hpp"

using namespace atto;

Module::Module(std::filesystem::path path, std::string code) :
  _path{path}, _code{code}, _parsed{false}, imported{}
{}

Module::~Module() { }

// static
void Module::parse(std::shared_ptr<Module> mod, std::string modName)
{
  if (modName.size() == 0)
    modName = mod->_path.stem();
  Module::allModules[modName] = mod;
  atto::lex(mod);
  atto::parse(mod);
  mod->_parsed = true;
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
  _code += code;
}

void Module::addToken(std::shared_ptr<const Token> &tok)
{
  _tokens.emplace_back(tok);
}

std::vector<std::shared_ptr<const Token>>&
Module::tokens()
{
  return _tokens;
}


std::unordered_map<std::string,
  std::pair<std::shared_ptr<Func>, std::vector<std::string>>
>& Module::funcs()
{
  return _funcs;
}

void Module::import(std::filesystem::path path)
{
  if (path.is_relative()){
    auto p = _path.parent_path();
    path = p / path;
  }

  // already imported?
  for (const auto&m : Module::allModules) {
    if (m.second->path().filename() == path.filename()) {
      imported[m.first] = m.second;
      return;
    }
  }

  bool success = false;
  const auto code = readFile(path, success);
  auto newMod = std::make_shared<Module>(path, code);
  Module::parse(newMod, path.stem());
  imported[path.stem()] = newMod;
}

//static
std::unordered_map<std::string, std::shared_ptr<Module>> Module::allModules{};
