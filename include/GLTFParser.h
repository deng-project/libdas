/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: GLTFParser.h - GLTF format parsing class header
/// author: Karl-Mihkel Ott

#ifndef GLTF_PARSER_H
#define GLTF_PARSER_H


#ifdef GLTF_PARSER_CPP
    #include <any>
    #include <string>
    #include <vector>
    #include <cfloat>

    #include <Iterators.h>
    #include <Points.h>
    #include <Vector.h>
    #include <Matrix.h>
    #include <JSONParser.h>
    #include <GLTFStructures.h>
#endif


namespace Libdas {
    
    class GLTFParser : public JSONParser {
        private:
            std::ifstream m_ext_reader;

        public:
            GLTFParser(const std::string &_file_name = "");
            /**
             * Parse GLTF file into appropriate structures
             * @param _file_name specifies the file name to read for parsing
             */
            void Parse(const std::string &_file_name = "");
    };
}

#endif
