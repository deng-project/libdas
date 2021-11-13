#ifndef STL_PARSER_H
#define STL_PARSER_H


#ifdef STL_PARSER_CPP
    #include <queue>
    #include <fstream>
    #include <unordered_map>

    #include <StreamReader.h>
#endif

namespace Libdas {

    enum AsciiSTLStatementType {
        ASCII_STL_STATEMENT_NONE,                       // special value to define empty newline
        ASCII_STL_STATEMENT_SOLID,                      // solid
        ASCII_STL_STATEMENT_NORMAL,                     // normal
        ASCII_STL_STATEMENT_OUTER,                      // outer
        ASCII_STL_STATEMENT_VERTEX,                     // vertex
        ASCII_STL_STATEMENT_ENDLOOP,                    // endloop
        ASCII_STL_STATEMENT_ENDFACET                    // endfacet
    };
    

    class AsciiSTLParser : private StreamReader {
        private:
            std::unordered_map<std::string, AsciiSTLStatementType> m_tokens;

        private:
            /** 
             * Create all possible tokens to consider for Wavefront Obj format
             */
            void _Tokenise();
        
        public:
            AsciiSTLParser(size_t _chunk_size = DEFAULT_CHUNK);
            AsciiSTLParser(const std::string &_file_name, size_t _chunk_size);
            /**
             * Parse the currently loaded file given as a parameter in constructor
             */
            void Parse();
            /**
             * Parse completely new file
             */
            void Parse(const std::string &_file_name);
    };
}

#endif
