#ifndef ATTO_PARSER_H
#define ATTO_PARSER_H

#include "modules.hpp"

namespace atto {

// parses all tokens in module from startTok
void parse(Module& module, std::size_t startTok = 0);

Module* curModule();


} // namespace atto

#endif // ATTO_PARSER_H
