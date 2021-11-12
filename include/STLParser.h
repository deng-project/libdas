#ifndef STL_PARSER_H
#define STL_PARSER_H

namespace Libdas {
    
    class AsciiSTLParser {
        private:
            std::unordered_map<std::string, 
        
        public:
            AsciiSTLParser(size_t _chunk_size = DEFAULT_CHUNK);
            AsciiSTLParser(const std::string &_file_name, size_t _chunk_size);
    };
}

#endif
