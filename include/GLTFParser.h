/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: GLTFParser.h - GLTF format parsing class header
/// author: Karl-Mihkel Ott

#ifndef GLTF_PARSER_H
#define GLTF_PARSER_H


#ifdef GLTF_PARSER_CPP
    #include <any>
    #include <map>
    #include <memory>
    #include <fstream>
#ifdef _DEBUG
    #include <iostream>
#endif
    #include <string>
    #include <vector>
    #include <cfloat>

    #include <Iterators.h>
    #include <Points.h>
    #include <Vector.h>
    #include <Matrix.h>
    #include <ErrorHandlers.h>
    #include <AsciiStreamReader.h>
    #include <JSONParser.h>
    #include <GLTFStructures.h>
#endif


namespace Libdas {

    enum GLTFObjects {
        GLTF_OBJECT_ACCESSOR,
        GLTF_OBJECT_ACCESSOR_SPARSE,
        GLTF_OBJECT_ACCESSOR_SPARSE_INDICES,
        GLTF_OBJECT_ACCESSOR_SPARSE_VALUES,
        GLTF_OBJECT_ANIMATION,
        GLTF_OBJECT_ANIMATION_CHANNEL,
        GLTF_OBJECT_ANIMATION_CHANNEL_TARGET,
        GLTF_OBJECT_ANIMATION_SAMPLER,
        GLTF_OBJECT_ASSET,
        GLTF_OBJECT_BUFFER,
        GLTF_OBJECT_BUFFERVIEW,
        GLTF_OBJECT_CAMERA,
        GLTF_OBJECT_CAMERA_ORTHOGRAPHIC,
        GLTF_OBJECT_CAMERA_PERSPECTIVE,
        GLTF_OBJECT_EXTENSIONS,
        GLTF_OBJECT_EXTRAS,
        GLTF_OBJECT_IMAGE
    };
    
    class GLTFParser : public JSONParser {
        private:
            std::ifstream m_ext_reader;
            AsciiFormatErrorHandler m_error;
            GLTFRoot m_root;

        public:
            GLTFParser(const std::string &_file_name = "");
            /**
             * Parse GLTF file into appropriate structures
             * @param _file_name specifies the file name to read for parsing
             */
            void Parse(const std::string &_file_name = "");
            /**
             * Get the parsed root object 
             * @return reference to GLTFRoot structure
             */
            GLTFRoot &GetRootObject();
    };
}

#endif
