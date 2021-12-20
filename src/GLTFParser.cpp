/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: GLTFParser.cpp - GLTF format parsing class implementation
/// author: Karl-Mihkel Ott

#define GLTF_PARSER_CPP
#include <GLTFParser.h>


namespace Libdas {

    GLTFParser::GLTFParser(const std::string &_file_name) : 
        JSONParser(_file_name), m_error(MODEL_FORMAT_GLTF){}


    void GLTFParser::Parse(const std::string &_file_name) {
        // parse json files with JSON parser
        JSONParser::Parse(_file_name);

        JSONNode &root = JSONParser::GetRootNode();

        // traverse the root node for data
        for(auto it = root.sub_nodes.begin(); it != root.sub_nodes.end(); it++) {
            // do something
        }
    }
}
