#ifndef GLTF_COMPILER_H
#define GLTF_COMPILER_H


#ifdef GLTF_COMPILER_CPP
    #include <any>
    #include <fstream>
    #include <vector>
    #include <cfloat>
    #include <unordered_map>

    #include <Points.h>
    #include <Vector.h>
    #include <Matrix.h>
    #include <DasStructures.h>
    #include <DasWriterCore.h>
    #include <GLTFStructures.h>
#endif


namespace Libdas {

    class GLTFCompiler : private DasWriterCore {
        private:
            /**
             * Check if any properties are empty and if they are, supplement values from GLTFRoot::asset into it
             * @param _root specifies a reference to GLTFRoot object, where potentially supplement values are held
             * @param _props specifies a reference to DasProperties, where supplementable values are held
             */
            void _CheckAndSupplementProperties(GLTFRoot &_root, DasProperties &_props);
            /**
             * Create all buffer objects from given root node
             * @param _root specifies a reference to GLTFRoot object, where all GLTF data is stored
             * @return moveable std::vector instance containing all DasBuffer objects
             */
            std::vector<DasBuffer> &&_MakeBuffers(const GLTFRoot &_root);
            /**
             * Create all static models from GLTF meshes
             * @param _root specifies a reference to GLTFRoot object, where all GLTF data is stored
             * @return moveable std::vector instance containing all DasModel objects
             */
            std::vector<DasModel> &&_MakeModels(const GLTFRoot &_root);
            std::vector<DasAnimation> &&_MakeAnimations(const GLTFRoot &_root);
        public:
            GLTFCompiler(const std::string &_out_file = "");
            GLTFCompiler(const GLTFRoot &_root, const DasProperties &_props, const std::string &_out_file);
            /**
             * Compile the DAS file from given GLTFRoot structure
             * @param _root specifies a reference to GLTFRoot structure where all GLTF data is contained
             * @param _props specifies a reference to DasProperties structure where all properties about the output DAS file is contained in
             * @param _out_file optinally specifes a new output file name to use
             */
            void Compile(const GLTFRoot &_root, const DasProperties &_props, const std::string &_out_file = "");
    };
}

#endif
