// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: GLTFCompiler.h - GLTF format to DAS compiler header
// author: Karl-Mihkel Ott

#ifndef GLTF_COMPILER_H
#define GLTF_COMPILER_H


#ifdef GLTF_COMPILER_CPP
    #include <any>
    #include <fstream>
    #include <iostream>
    #include <optional>
    #include <vector>
    #include <cfloat>
    #include <cmath>
    #include <cstring>
    #include <unordered_map>
    #include <algorithm>

    #include <Api.h>
    #include <Points.h>
    #include <Vector.h>
    #include <Matrix.h>
    #include <Quaternion.h>

    #include <Algorithm.h>
    #include <LibdasAssert.h>
    #include <ErrorHandlers.h>
    #include <DasStructures.h>
#define LIBDAS_DEFS_ONLY
    #include <HuffmanCompression.h>
#undef LIBDAS_DEFS_ONLY
    #include <Base64Decoder.h>
    #include <URIResolver.h>
    #include <TextureReader.h>
    #include <DasWriterCore.h>
    #include <TextureReader.h>
    #include <GLTFStructures.h>
    #include <BufferImageTypeResolver.h>
#endif

namespace Libdas {

    class LIBDAS_API GLTFCompiler : private DasWriterCore {
        private:
            // images are always appended to the vector after buffers
            size_t m_buffers_size = 0;
            size_t m_images_size = 0;
            const bool m_use_raw_textures;
            std::string m_root_path;

            // buffer related
            std::vector<URIResolver> m_uri_resolvers;
            std::vector<uint32_t> m_modified_buffers_table;
            std::vector<char*> m_supplemented_buffers; // cleanup bookmarking
            std::vector<TextureReader> m_tex_readers;

            const std::unordered_map<std::string, BufferType> m_attribute_type_map = {
                std::make_pair("POSITION", LIBDAS_BUFFER_TYPE_VERTEX),
                std::make_pair("NORMAL", LIBDAS_BUFFER_TYPE_VERTEX_NORMAL),
                std::make_pair("TANGENT", LIBDAS_BUFFER_TYPE_VERTEX_TANGENT),
                std::make_pair("TEXCOORD_", LIBDAS_BUFFER_TYPE_TEXTURE_MAP),
                std::make_pair("COLOR_", LIBDAS_BUFFER_TYPE_COLOR),
                std::make_pair("JOINTS_", LIBDAS_BUFFER_TYPE_JOINTS),
                std::make_pair("WEIGHTS_", LIBDAS_BUFFER_TYPE_WEIGHTS)
            };

            std::vector<int32_t> m_image_buffer_views;

            struct BufferAccessorData {
                uint32_t buffer_id = 0;
                uint32_t buffer_offset = 0;
                uint32_t used_size = 0;
            };

            struct IndexSupplementationInfo {
                uint32_t buffer_offset;
                uint32_t used_size;
                int32_t component_type;

                struct less {
                    bool operator()(const IndexSupplementationInfo &_s1, const IndexSupplementationInfo &_s2) {
                        return _s1.buffer_offset < _s2.buffer_offset;
                    }
                };

                static bool IsDuplicate(const IndexSupplementationInfo &_s1, const IndexSupplementationInfo &_s2) {
                    return _s1.buffer_offset == _s2.buffer_offset;
                }
            };

        private:
            BufferAccessorData _FindAccessorData(const GLTFRoot &_root, int32_t _accessor_id);
            size_t _SupplementIndices(const char *_odata, IndexSupplementationInfo &_suppl_info, DasBuffer &_buffer);
            std::vector<std::vector<GLTFAccessor*>> _GetAllBufferAccessorRegions(GLTFRoot &_root);
            std::vector<std::vector<IndexSupplementationInfo>> _GetBufferIndexRegions(const GLTFRoot &_root);
            void _CorrectOffsets(std::vector<GLTFAccessor*> &_accessors, size_t _diff, size_t _offset);

            /**
             * Augment index buffers to uint32_t type and set all offsets correctly
             * @param _root specifies an reference to GLTFRoot object
             * @param _buffers specifies a DasBuffer vector, containing all generated buffer instances
             */
            void _StrideIndexBuffers(const GLTFRoot &_root, std::vector<DasBuffer> &_buffers);
            /**
             * Check if any properties are empty and if they are, supplement values from GLTFRoot::asset into it
             * @param _root specifies a reference to GLTFRoot object, where potentially supplement values are held
             * @param _props specifies a reference to DasProperties, where supplementable values are held
             */
            void _CheckAndSupplementProperties(const GLTFRoot &_root, DasProperties &_props);
            /**
             * Give buffers appropriate flags according to meshes
             * @param _root specifies a reference to GLTFRoot object
             * @param _buffers specifies a reference to std::vector object, containing all generated buffer instances
             */
            void _FlagBuffersAccordingToMeshes(const GLTFRoot &_root, std::vector<DasBuffer> &_buffers);
            /**
             * Give buffers appropriate flags according to animations
             * @param _root specifies a reference to GLTFRoot object
             * @param _buffers specifies a reference to std::vector object, containing all generated buffer instances
             */
            void _FlagBuffersAccordingToAnimations(const GLTFRoot &_root, std::vector<DasBuffer> &_buffers);
            /**
             * Create all buffer objects from given root node
             * @param _root specifies a reference to GLTFRoot object, where all GLTF data is stored
             * @return std::vector instance containing all DasBuffer objects
             */
            std::vector<DasBuffer> _CreateBuffers(const GLTFRoot &_root, const std::vector<std::string> &_embedded_textures);
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
            GLTFCompiler(const std::string &_in_path, const std::string &_out_file = "", bool _use_raw_textures = false);
            GLTFCompiler(const std::string &_in_path, const GLTFRoot &_root, const DasProperties &_props, 
                         const std::string &_out_file = "", const std::vector<std::string> &_embedded_textures = {}, bool _use_raw_textures = false);
            ~GLTFCompiler();
            /**
             * Compile the DAS file from given GLTFRoot structure
             * @param _root specifies a reference to GLTFRoot structure where all GLTF data is contained
             * @param _props specifies a reference to DasProperties structure where all properties about the output DAS file is contained in
             * @param _out_file optinally specifes a new output file name to use
             */
            void Compile(const GLTFRoot &_root, const DasProperties &_props, const std::vector<std::string> &_embedded_textures, const std::string &_out_file = "");
    };
}

#endif
