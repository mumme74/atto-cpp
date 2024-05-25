#include <vector>
#include <utility>
#include <fstream>
#include "atto.hpp"
#include "lex.hpp"
#include "parser.hpp"
#include "values.hpp"
#include "errors.hpp"
#include "lib/linenoise.hpp"



using namespace atto;

// private to this file
namespace fs = std::filesystem;

// -------------------------------------

Atto::Atto(std::filesystem::path replHistoryPath,
       std::istream &cin,
       std::ostream &cout,
       std::ostream &cerr) :
  _replHistoryPath{replHistoryPath},
  _cin{cin}, _cout{cout}, _cerr{cerr},
  _core_loaded{false}, vm{}
{
  linenoise::SetMultiLine(true);
  linenoise::LoadHistory(_replHistoryPath.c_str());
  auto corePath = fs::path(__FILE__).parent_path().parent_path();
  corePath.append("atto/core.at");
  _core_loaded = execFile(corePath, "__core__");

  linenoise::SetCompletionCallback([](
    std::string editBuffer, std::vector<std::string>& completions)
  {
    if (!Module::allModules["__main__"]->path().empty())
      return; // only complete when in repl mode

    auto pos = editBuffer.find_last_of(" ");
    const auto str = editBuffer.substr(pos != std::string::npos ? pos+1 : 0);
    if (editBuffer == "f") completions.emplace_back("fn main is ");
    if (str == "i") completions.emplace_back(editBuffer + "s");
    for (const auto& mod : Module::allModules) {
      for (const auto& fn : mod.second->funcs()) {
        if (fn.first.substr(0, str.length()) == str)
          completions.emplace_back(
            editBuffer + fn.first.substr(str.length()));
      }
    }
  });
}

Atto::~Atto()
{
  linenoise::SaveHistory(_replHistoryPath.c_str());
  linenoise::linenoiseAtExit();
}

bool Atto::execFile(std::filesystem::path path, std::string modName)
{
  if (!_core_loaded && modName != "__core__")
    return false;
  bool success = false;
  auto code = readFile(path, success);
  if (success)
    return eval(code, path, modName);

  return false;
}

bool Atto::eval(
  const std::string& code,
  std::filesystem::path path,
  std::string modName
) {
  try {
    auto mod = std::make_shared<Module>(path, code);
    Module::parse(mod, modName);
    const auto& main = mod->funcs()["main"].first;
    if (main) {
      const std::vector<std::shared_ptr<const Value>> args;
      vm.eval(*main, main->module()->funcs(), args);
    }

    return true;
  } catch (const SyntaxError &e) {
    auto lines = split(code, "\n");
    _cerr << e.typeName() << ": in " << e.module()->path() << "\n"
          << e.what() << " at line " << e.line() << " col " << e.col() << '\n'
          << lines[e.line()-1] << '\n' << std::setw(e.col()+1) << '^' << "\n";
  }
  return false;
}


void Atto::repl()
{
  _cout << "Welcome to the Atto prompt.\n"
        << "The core library is included by default.\n";

  auto main = std::make_shared<Module>("","");
  Module::allModules["__main__"] = main;

  std::string line;
  bool quit = false;
  while (!(quit = linenoise::Readline(">>", line)) && !quit) {
    linenoise::AddHistory(line.c_str());
    if (line == "quit()") break;
    eval(line, "", "__main__");
  }
}
