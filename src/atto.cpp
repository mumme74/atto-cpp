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


const Value eval(const std::function<const Value()> cb) {
  try {
    return cb();

  } catch (const SyntaxError &e) {
    auto lines = split(std::string(e.module().code()), "\n");
    std::cerr << e.typeName() << ": in " << e.module().path() << "\n"
          << e.what() << " at line " << e.line() << " col " << e.col() << '\n'
          << lines[e.line()-1] << '\n' << std::setw(e.col()+1) << '^' << "\n";
  }
  return false;
}

// -------------------------------------

Atto::Atto(std::filesystem::path replHistoryPath) :
  _replHistoryPath{replHistoryPath}, vm{}
{
  auto corePath = fs::path(__FILE__).parent_path().parent_path();
  corePath.append("atto/core.at");
  Module::module(std::string("__core__"), corePath);
}

Atto::~Atto()
{
  linenoise::linenoiseAtExit();
}

const Value Atto::execFile(
  std::filesystem::path path, std::string modName)
{
  auto lambdaEval = [&]() -> const Value {
    auto mod = Module::module(modName, path);
    if (mod.hasFunc("main")) {
      const std::vector<std::shared_ptr<const Value>> args;
      return *vm.eval(mod.func("main"), mod.funcs(), args);
    }
    return Value(false);
  };

  return eval(lambdaEval);
}



void Atto::repl()
{
  std::cout << "Welcome to the Atto prompt.\n"
        << "The core library is included by default.\n";

  linenoise::SetMultiLine(true);
  linenoise::LoadHistory(_replHistoryPath.c_str());

  linenoise::SetCompletionCallback([](
    std::string editBuffer, std::vector<std::string>& completions)
  {    auto pos = editBuffer.find_last_of(" ");
    const auto str = editBuffer.substr(pos != std::string::npos ? pos+1 : 0);
    if (editBuffer == "f") completions.emplace_back("fn main is ");
    if (str == "i") completions.emplace_back(editBuffer + "s");
    for (const auto& name : Module::allModuleNames()) {
      for (const auto& fn : Module::module(name).funcs()) {
        if (fn.first.substr(0, str.length()) == str)
          completions.emplace_back(
            editBuffer + fn.first.substr(str.length()));
      }
    }
  });

  auto main = Module::module("__main__","");

  std::string line;
  bool quit = false;
  while (!(quit = linenoise::Readline(">>", line)) && !quit) {
    linenoise::AddHistory(line.c_str());
    if (line == "quit()") break;
    auto lambdaEval = [&]() -> const Value {
      main.appendCode(line);
      if (main.hasFunc("main")) {
        const std::vector<std::shared_ptr<const Value>> args;
        return *vm.eval(main.func("main"), main.funcs(), args);
      }
    };

    eval(lambdaEval);
  }

  linenoise::SaveHistory(_replHistoryPath.c_str());
}
