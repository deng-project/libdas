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
    #include <array>
    #include <vector>
    #include <cfloat>
    #include <cmath>
    #include <cstring>
    #include <unordered_map>
    #include <algorithm>
    #include <string>

    #include "trs/Points.h"
    #include "trs/Vector.h"
    #include "trs/Matrix.h"
    #include "trs/Quaternion.h"

    #include "das/Api.h"
    #include "das/Hash.h"
    #include "das/STDUtils.h"
    #include "das/Algorithm.h"
    #include "das/LibdasAssert.h"
    #include "das/ErrorHandlers.h"
    #include "das/DasStructures.h"
#define LIBDAS_DEFS_ONLY
    #include "das/HuffmanCompression.h"
#undef LIBDAS_DEFS_ONLY
    #include "das/Base64Decoder.h"
    #include "das/URIResolver.h"
    #include "das/TextureReader.h"
    #include "das/DasWriterCore.h"
    #include "das/TextureReader.h"
    #include "das/GLTFStructures.h"
    #include "das/BufferImageTypeResolver.h"
#endif

namespace Libdas {


    class LIBDAS_API GLTFCompiler : private DasWriterCore {
        private:
            // bytes that are used for padding
            const char m_pad[16] = {};
            
            // images are always appended to the vector after buffers
            size_t m_buffers_size = 0;
            size_t m_images_size = 0;
            const bool m_use_raw_textures;
            std::string m_root_path;

            // buffer related
            std::vector<URIResolver> m_uri_resolvers;
            std::vector<std::vector<char*>> m_supplemented_buffers; // cleanup bookmarking
            std::vector<TextureReader> m_tex_readers;
            std::vector<char*> m_allocated_memory;

            const std::unordered_map<std::string, BufferType> m_attribute_type_map = {
                std::make_pair("POSITION", LIBDAS_BUFFER_TYPE_VERTEX),
                std::make_pair("NORMAL", LIBDAS_BUFFER_TYPE_VERTEX_NORMAL),
                std::make_pair("TANGENT", LIBDAS_BUFFER_TYPE_VERTEX_TANGENT),
                std::make_pair("TEXCOORD_", LIBDAS_BUFFER_TYPE_TEXTURE_MAP),
                std::make_pair("COLOR_", LIBDAS_BUFFER_TYPE_COLOR),
                std::make_pair("JOINTS_", LIBDAS_BUFFER_TYPE_JOINTS),
                std::make_pair("WEIGHTS_", LIBDAS_BUFFER_TYPE_WEIGHTS)
            };

            struct BufferAccessorData {
                uint32_t buffer_id = UINT32_MAX;
                uint32_t buffer_offset = UINT32_MAX; // bytes
                int32_t component_type = INT32_MAX; 
                uint32_t used_size = UINT32_MAX;     // bytes
                uint32_t unit_size = UINT32_MAX;     // bytes
                uint32_t unit_stride = 0;            // if this is specified then compiler executes a lot of cpu instructions for no other reason that gltf just being a shit format

                struct less {
                    bool operator()(const BufferAccessorData &_s1, const BufferAccessorData &_s2) {
                        return _s1.buffer_offset < _s2.buffer_offset;
                    }
                };

                static bool IsDuplicate(const BufferAccessorData &_s1, const BufferAccessorData &_s2) {
                    return _s1.buffer_offset == _s2.buffer_offset;
                }
            };

            struct GenericVertexAttributeAccessors {
                uint32_t pos_accessor = UINT32_MAX;
                uint32_t normal_accessor = UINT32_MAX;
                uint32_t tangent_accessor = UINT32_MAX;
                std::vector<uint32_t> uv_accessors;
                std::vector<uint32_t> color_mul_accessors;
                std::vector<uint32_t> joints_accessors;
                std::vector<uint32_t> weights_accessors;

                uint32_t indices_accessor = UINT32_MAX;
            };


            struct GenericVertexAttribute {
                GenericVertexAttribute() = default;

                inline bool operator==(const GenericVertexAttribute &_v) const {
                    return pos == _v.pos &&
                           normal == _v.normal &&
                           tangent == _v.tangent &&
                           uv == _v.uv &&
                           color == _v.color &&
                           weights == _v.weights;
                }

                TRS::Vector3<float> pos;
                TRS::Vector3<float> normal;
                TRS::Vector4<float> tangent;

                std::vector<TRS::Vector2<float>> uv;
                std::vector<TRS::Vector4<float>> color;
                std::vector<TRS::Vector4<uint16_t>> joints;
                std::vector<TRS::Vector4<float>> weights;
            };

            std::vector<DasMesh> m_meshes;
            std::vector<DasMeshPrimitive> m_mesh_primitives;
            std::vector<DasMorphTarget> m_morph_targets;
            std::vector<GenericVertexAttribute> m_indexed_attrs;
            std::vector<uint32_t> m_generated_indices;
            std::vector<uint32_t> m_scene_node_id_table;
            std::vector<uint32_t> m_skeleton_joint_id_table;

        private:
            uint32_t _FindKhronosComponentSize(int32_t _component_type);
            BufferAccessorData _FindAccessorData(const GLTFRoot &_root, int32_t _accessor_id);
            size_t _FindPrimitiveCount(const GLTFRoot &_root);
            size_t _FindMorphTargetCount(const GLTFRoot &_root);
            uint32_t _EnumerateAttributes(const std::string &_attr_name, const GLTFMeshPrimitive::AttributesType &_attrs);

            // node flagging method
            void _FlagJointNodes(const GLTFRoot &_root);

            // common parent root finding methods
            bool _IsRootNode(const GLTFRoot &_root, int32_t _node_id, const std::vector<int32_t> &_pool);
            uint32_t _FindCommonRootJoint(const GLTFRoot &_root, const GLTFSkin &_skin);

            void _CreateMeshPrimitive(const GLTFRoot &_root, GenericVertexAttributeAccessors &_gen_acc, size_t _mesh_id, size_t _prim_id);
            void _CreateMorphTarget(const GLTFRoot &_root, GenericVertexAttributeAccessors &_gen_acc, size_t _mesh_id, size_t _prim_id, size_t _morph_id);
            DasBuffer _RewriteMeshBuffer(GLTFRoot &_root);

            // high level data type casts
            TRS::Vector2<float> _GetUV(BufferAccessorData &_ad, uint32_t _index);
            TRS::Vector4<float> _GetColorMultiplier(BufferAccessorData &_ad, uint32_t _index);
            TRS::Vector4<uint16_t> _GetJointIndices(BufferAccessorData &_ad, uint32_t _index);
            TRS::Vector4<float> _GetJointWeights(BufferAccessorData &_ad, uint32_t _index);
            uint32_t _GetIndex(BufferAccessorData &_ad, uint32_t _index);

            // indexing methods
            GenericVertexAttribute _GenerateGenericVertexAttribute(GLTFRoot &_root, GenericVertexAttributeAccessors &_gen_acc, uint32_t _index);
            GenericVertexAttributeAccessors _GenerateGenericVertexAttributeAccessors(GLTFMeshPrimitive::AttributesType &_attrs);
            /**
             * @return new offset value in bytes
             */
            void _IndexMeshPrimitive(GLTFRoot &_root, GenericVertexAttributeAccessors &_gen_acc);
            void _WriteIndexedData(GLTFRoot &_root, GenericVertexAttributeAccessors &_gen_acc, DasBuffer &_buffer, bool _write_indices = true);


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
             * Create all buffer objects from given root node
             * @param _root specifies a reference to GLTFRoot object, where all GLTF data is stored
             * @return std::vector instance containing all DasBuffer objects
             */
            std::vector<DasBuffer> _CreateBuffers(GLTFRoot &_root, const std::vector<std::string> &_embedded_textures);

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
             * Create DasSkeletonJoint instances from given GLTF skin nodes
             * @param _root specifies a reference to GLTFRoot object, where all GLTF data is stored
             * @return std::vector instance containing all DasSkeletonJoint objects
             */
            std::vector<DasSkeletonJoint> _CreateSkeletonJoints(const GLTFRoot &_root);

            /**
             * Create DasSkeleton instances from given GLTF skins
             * @param _root specifies a reference to GLTFRoot object, where all GLTF data is stored
             * @return std::vector instance containing all DasSkeleton objects
             */
            std::vector<DasSkeleton> _CreateSkeletons(const GLTFRoot &_root);

            /**
             * Create DasAnimationChannel instances from given GLTF animations
             * @param _root specifies a reference to GLTFRoot object
             */
            std::vector<DasAnimationChannel> _CreateAnimationChannels(const GLTFRoot &_root);

            /**
             * Analyse and create DasAnimation instances from GLTF animations
             * @param _root specifies a reference to GLTFRoot object, where all GLTF data is stored
             * @return std::vector instance containing all DasAnimation objects
             */
            std::vector<DasAnimation> _CreateAnimations(const GLTFRoot &_root);
        public:
            GLTFCompiler(const std::string &_in_path, const std::string &_out_file = "", bool _use_raw_textures = false);
            GLTFCompiler(const std::string &_in_path, GLTFRoot &_root, const DasProperties &_props, 
                         const std::string &_out_file = "", const std::vector<std::string> &_embedded_textures = {}, bool _use_raw_textures = false);
            ~GLTFCompiler();
            /**
             * Compile the DAS file from given GLTFRoot structure
             * @param _root specifies a reference to GLTFRoot structure where all GLTF data is contained
             * @param _props specifies a reference to DasProperties structure where all properties about the output DAS file is contained in
             * @param _out_file optinally specifes a new output file name to use
             */
            void Compile(GLTFRoot &_root, const DasProperties &_props, const std::vector<std::string> &_embedded_textures, const std::string &_out_file = "");
    };
}

#endif
