#include "atto.hpp"
#include "values.hpp"
#include <iostream>

using namespace atto;

void printHelp()
{
  std::cout <<
    "Usage:\n" <<
    " atto [file]     executes file\n" <<
    " atto            runs in REPL mode\n" <<
    " atto -h         display this help\n";
}

int main(int argc, const char *argv[]) {
  Atto engine;

  if (argc == 1) {
    engine.repl();
  } else if (argc != 2) {
    printHelp();
  } else {
    if (argv[1][0] == '-')
      printHelp();
    else {
      auto retVlu = engine.execFile(argv[1]);
      switch (retVlu.type()) {
      case ValueTypes::Null: return 0;
      case ValueTypes::Num: return static_cast<int>(retVlu.asNum());
      case ValueTypes::Bool: return retVlu.asBool() ? 1 : 0;
      case ValueTypes::Str: return retVlu.asStr().length();
      default: return 0;
      }
    }
  }
}