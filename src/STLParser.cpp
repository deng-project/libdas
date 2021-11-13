#define STL_PARSER_CPP
#include <STLParser.h>


namespace Libdas {

    AsciiSTLParser(size_t _chunk_size) : StreamReader(_chunk_size, '\n') {}
}
