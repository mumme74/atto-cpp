#include "modules.hpp"

using namespace atto;

Module::Module(std::filesystem::path path, std::string code) :
  _path{path}, _code{code}
{}

Module::~Module() { }

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

//static
std::unordered_map<std::string, std::shared_ptr<Module>> Module::allModules{};
