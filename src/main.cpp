#include "atto.hpp"
#include <iostream>

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
    else
      engine.execFile(argv[1]);
  }
}