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
      auto main = std::make_shared<Module>(path, code);
      _modules.emplace_back(main);
      lex(main);
      parse(main);
      for (const auto& tok : main->tokens()) {
        _cout << "line:" << tok->line() << " col: " << tok->col() << " " << tok->ident() << '\n';
      }
      for (const auto&f : main->funcs()) {
        _cout << "func:" << f.first << " \n";
      }
      return true;
    }
  } catch (SyntaxError &e) {
    auto lines = split(code, "\n");
    _cerr << e.typeName() << ":\n"
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
  _modules.emplace_back(main);

  std::size_t from = 0;
  std::string line;
  bool quit = false;
  while (!(quit = linenoise::Readline(">>", line)) && !quit) {
    linenoise::AddHistory(line.c_str());
    if (line == "quit()") break;
    std::size_t fromTok = main->tokens().size();
    main->appendCode(line);
    lex(main, from);
    parse(main, fromTok);
    from += line.size();
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
