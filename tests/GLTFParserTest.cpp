#include <any>
#include <variant>
#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <memory>
#include <cfloat>
#include <fstream>

#include <Api.h>
#include <Points.h>
#include <Vector.h>
#include <Matrix.h>
#include <ErrorHandlers.h>
#include <AsciiStreamReader.h>
#include <JSONParser.h>
#include <GLTFStructures.h>
#include <GLTFParser.h>


// output asset information
void OutputAsset(Libdas::GLTFRoot &_root) {
    std::cout << "Copyright: " << _root.asset.copyright << std::endl;
    std::cout << "Generator: " << _root.asset.generator << std::endl;
    std::cout << "Version: " << _root.asset.version << std::endl;
    std::cout << "MinVersion: " << _root.asset.min_version << std::endl << std::endl;
}


// output information about all buffers
void OutputBuffers(Libdas::GLTFRoot &_root) {
    for(size_t i = 0; i < _root.buffers.size(); i++) {
        std::cout << "Uri: " << _root.buffers[i].uri << std::endl;
        std::cout << "Byte length: " << _root.buffers[i].byte_length << std::endl;

        if(_root.buffers[i].name != "")
            std::cout << "Name: " << _root.buffers[i].name << std::endl << std::endl;
    }
}


// output information about all buffer views
void OutputBufferViews(Libdas::GLTFRoot &_root) {
    for(size_t i = 0; i < _root.buffer_views.size(); i++) {
        std::cout << "Buffer: " << _root.buffer_views[i].buffer << std::endl;
        std::cout << "Byte offset: " << _root.buffer_views[i].byte_offset << std::endl;
        std::cout << "Byte length: " << _root.buffer_views[i].byte_length << std::endl;
        std::cout << "Byte stride: " << _root.buffer_views[i].byte_stride << std::endl;

        // output buffer view target if possible
        if(_root.buffer_views[i].target == KHRONOS_ARRAY_BUFFER ||
           _root.buffer_views[i].target == KHRONOS_ELEMENT_ARRAY_BUFFER) {
            std::cout << "Target: " << (_root.buffer_views[i].target == KHRONOS_ARRAY_BUFFER ? "ARRAY_BUFFER" : 
                         "ELEMENT_ARRAY_BUFFER") << std::endl;
        }

        // output buffer view name is possible
        if(_root.buffer_views[i].name != "") {
            std::cout << "Name: " << _root.buffer_views[i].name << std::endl;
        }
    }
}


int main(int argc, char *argv[]) {
    if(argc < 2) {
        std::cerr << "Please provide a GLTF file to parse" << std::endl;
        std::exit(LIBDAS_ERROR_INVALID_FILE);
    }

    Libdas::GLTFParser parser(argv[1]);
    parser.Parse();

    Libdas::GLTFRoot &root = parser.GetRootObject();
    OutputAsset(root);
    OutputBuffers(root);
    OutputBufferViews(root);

    return 0;
}
