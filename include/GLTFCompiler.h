#ifndef GLTF_COMPILER_H
#define GLTF_COMPILER_H


#ifdef GLTF_COMPILER_CPP
    #include <any>
    #include <fstream>
    #include <iostream>
    #include <optional>
    #include <vector>
    #include <cfloat>
    #include <unordered_map>

    #include <Api.h>
    #include <Points.h>
    #include <Vector.h>
    #include <Matrix.h>
    #include <Quaternion.h>

    #include <FileNameString.h>
    #include <DasStructures.h>
    #include <DasWriterCore.h>
    #include <GLTFStructures.h>
    #include <BufferImageTypeResolver.h>
#endif


namespace Libdas {

    class LIBDAS_API GLTFCompiler : private DasWriterCore {
        private:
            // images are always appended to the vector after buffers
            size_t m_buffers_size = 0;
            size_t m_images_size = 0;

        private:
            /**
             * Check if any properties are empty and if they are, supplement values from GLTFRoot::asset into it
             * @param _root specifies a reference to GLTFRoot object, where potentially supplement values are held
             * @param _props specifies a reference to DasProperties, where supplementable values are held
             */
            void _CheckAndSupplementProperties(GLTFRoot &_root, DasProperties &_props);
            /**
             * Give buffers appropriate flags according to meshes
             * @param _root specifies a reference to GLTFRoot object
             * @param _buffers specifies a reference to std::vector object, containing all generated buffer instances
             */
            void _FlagBuffersAccordingToMeshes(const GLTFRoot &_root, std::vector<DasBuffer> &_buffers);
            /**
             * Give buffers appropriate flags according to images
             * @param _root specifies a reference to GLTFRoot object
             * @param _buffers specifies a reference to std::vector object, containing all generated buffer instances
             */
            void _FlagBuffersAccordingToImages(const GLTFRoot &_root, std::vector<DasBuffer> &_buffers);
            /**
             * Create all buffer objects from given root node
             * @param _root specifies a reference to GLTFRoot object, where all GLTF data is stored
             * @return std::vector instance containing all DasBuffer objects
             */
            std::vector<DasBuffer> _CreateBuffers(const GLTFRoot &_root);
            /**
             * Create DasMesh instances from GLTF meshes
             * @param _root specifies a reference to GLTFRoot object, where all GLTF data is stored
             * @return std::vector instance containing all DasMesh objects
             */
            std::vector<DasMesh> _CreateMeshes(const GLTFRoot &_root);
            /**
             * Create DasNodes from GLTF nodes
             * @param _root specifies a reference to GLTFRoot object, where all GLTF data is stored
             * @return std::vector instance containing all DasNode objects
             */
            std::vector<DasNode> _CreateNodes(const GLTFRoot &_root);
            /**
             * Create DasScene instances from GLTF scenes
             * @param _root specifies a reference to GLTFRoot object, where all GLTF data is stored
             * @return std::vector instance containing all DasScene objects
             */
            std::vector<DasScene> _CreateScenes(const GLTFRoot &_root);
            /**
             * Create DasSkeleton instances from given GLTF skins
             * @param _root specifies a reference to GLTFRoot object, where all GLTF data is stored
             * @return std::vector instance containing all DasSkeleton objects
             */
            std::vector<DasSkeleton> _CreateSkeletons(const GLTFRoot &_root);
            /**
             * Create DasSkeletonJoint instances from given GLTF skin nodes
             * @param _root specifies a reference to GLTFRoot object, where all GLTF data is stored
             * @return std::vector instance containing all DasSkeletonJoint objects
             */
            std::vector<DasSkeletonJoint> _CreateSkeletonJoints(const GLTFRoot &_root);
            /**
             * Analyse and create DasAnimation instances from GLTF animations
             * @param _root specifies a reference to GLTFRoot object, where all GLTF data is stored
             * @return std::vector instance containing all DasAnimation objects
             */
            std::vector<DasAnimation> _CreateAnimations(const GLTFRoot &_root);
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
