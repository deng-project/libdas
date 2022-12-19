// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: GLTFCompiler.h - GLTF format to DAS compiler header
// author: Karl-Mihkel Ott

#ifndef GLTF_COMPILER_H
#define GLTF_COMPILER_H


#ifdef GLTF_COMPILER_CPP
    #include <any>
    #include <algorithm>
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

#ifdef _WIN32
    #include <Windows.h>
#endif

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
#include <type_traits>

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
                uint32_t unit_stride = 0;            // might be a necessary variable, since the data might not be tightly packed

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

            std::vector<DasMesh> m_meshes;
            std::vector<DasMeshPrimitive> m_mesh_primitives;
            std::vector<DasMorphTarget> m_morph_targets;
            std::vector<uint32_t> m_scene_node_id_table;
            std::vector<uint32_t> m_skeleton_joint_id_table;

        private:
            uint32_t _FindKhronosComponentSize(int32_t _component_type);
            BufferAccessorData _FindAccessorData(const GLTFRoot &_root, int32_t _accessor_id);
            size_t _FindPrimitiveCount(const GLTFRoot &_root);
            size_t _FindMorphTargetCount(const GLTFRoot &_root);

            // node flagging method
            void _FlagJointNodes(const GLTFRoot &_root);

            // common parent root finding methods
            bool _IsRootNode(const GLTFRoot &_root, int32_t _node_id, const std::vector<int32_t> &_pool);
            uint32_t _FindCommonRootJoint(const GLTFRoot &_root, const GLTFSkin &_skin);

            DasBuffer _RewriteMeshBuffer(GLTFRoot &_root);

            // high level vertex attribute data type casts
            TRS::Vector2<float> _GetUV(BufferAccessorData &_ad, uint32_t _index);
            TRS::Vector4<float> _GetColorMultiplier(BufferAccessorData &_ad, uint32_t _index);
            TRS::Vector4<uint16_t> _GetJointIndices(BufferAccessorData &_ad, uint32_t _index);
            TRS::Vector4<float> _GetJointWeights(BufferAccessorData &_ad, uint32_t _index);
            uint32_t _GetIndex(BufferAccessorData &_ad, uint32_t _index);

            // indexing methods
            GenericVertexAttributeAccessors _GenerateGenericVertexAttributeAccessors(GLTFMeshPrimitive::AttributesType &_attrs);
            void _CopyVertexAttributeAccessorDataToBuffer(GLTFRoot &_root, uint32_t _accessor, DasBuffer &_buffer, uint32_t &_attr_id, uint32_t &_attr_offset);

            /**
             * Write a single attribute values that might need casting
             */
            template<typename T>
            void _RewriteSingleAttributeAccessorDataToBuffer(GLTFRoot &_root,
                                                             uint32_t &_accessor,
                                                             uint32_t &_prim_id,
                                                             uint32_t &_prim_offset,
                                                             DasBuffer &_buffer,
                                                             T (GLTFCompiler::*GetAttrib)(BufferAccessorData &_acc, uint32_t _index))
            {
                BufferAccessorData acc = _FindAccessorData(_root, _accessor);
                uint32_t stride = acc.unit_stride ? acc.unit_stride : acc.unit_size;
                size_t len = acc.used_size / stride * sizeof(T);
                char *buf = new char[len]{};
                for(size_t j = 0; j < len / sizeof(T); j++) {
                    reinterpret_cast<T*>(buf)[j] = (this->*GetAttrib)(acc, j);
                }

                _buffer.data_ptrs.push_back(std::make_pair(buf, len));
                _prim_id = 0;
                _prim_offset = _buffer.data_len;
                _buffer.data_len += static_cast<uint32_t>(len);
                m_allocated_memory.push_back(buf);
            }
            
            /**
             * Write attributes that can occur multiple times in a mesh primitive
             */
            template<typename T>
            void _RewriteMultiAttributeAccessorsDataToBuffer(GLTFRoot &_root,
                                                             std::vector<uint32_t> &_accessors, 
                                                             uint32_t *_prim_ids,
                                                             uint32_t *_prim_offsets,
                                                             DasBuffer &_buffer,
                                                             T (GLTFCompiler::*GetAttrib)(BufferAccessorData &_acc, uint32_t _index))
            {
                for(size_t i = 0; i < _accessors.size(); i++) {
                    _RewriteSingleAttributeAccessorDataToBuffer(_root,
                                                                _accessors[i],
                                                                _prim_ids[i],
                                                                _prim_offsets[i],
                                                                _buffer,
                                                                GetAttrib);
                }
            }

            /**
             * Write mesh primitive or morph target data 
             */
            template<typename T>
            void _WritePrimitiveData(GLTFRoot &_root, GenericVertexAttributeAccessors &_gen_acc, DasBuffer &_buffer, T &_prim) {
                LIBDAS_ASSERT(_gen_acc.pos_accessor != UINT32_MAX);
                _CopyVertexAttributeAccessorDataToBuffer(_root, _gen_acc.pos_accessor, _buffer, 
                                                         _prim.vertex_buffer_id, 
                                                         _prim.vertex_buffer_offset);

                if(_gen_acc.normal_accessor != UINT32_MAX) {
                    _CopyVertexAttributeAccessorDataToBuffer(_root, _gen_acc.normal_accessor, _buffer,
                                                             _prim.vertex_normal_buffer_id, 
                                                             _prim.vertex_normal_buffer_offset);
                } 

                if(_gen_acc.tangent_accessor != UINT32_MAX) {
                    _CopyVertexAttributeAccessorDataToBuffer(_root, _gen_acc.tangent_accessor, _buffer,
                                                             _prim.vertex_tangent_buffer_id, 
                                                             _prim.vertex_tangent_buffer_offset);
                }

                // uv
                if(_gen_acc.uv_accessors.size()) {
                    _prim.texture_count = static_cast<uint32_t>(_gen_acc.uv_accessors.size());
                    _prim.uv_buffer_ids = new uint32_t[_gen_acc.uv_accessors.size()];
                    _prim.uv_buffer_offsets = new uint32_t[_gen_acc.uv_accessors.size()];
                    _RewriteMultiAttributeAccessorsDataToBuffer(_root,
                                                                _gen_acc.uv_accessors, 
                                                                _prim.uv_buffer_ids,
                                                                _prim.uv_buffer_offsets, 
                                                                _buffer,
                                                                &GLTFCompiler::_GetUV);
                }

                // color multipliers
                if(_gen_acc.color_mul_accessors.size()) {
                    _prim.color_mul_count = static_cast<uint32_t>(_gen_acc.color_mul_accessors.size());
                    _prim.color_mul_buffer_ids = new uint32_t[_gen_acc.color_mul_accessors.size()];
                    _prim.color_mul_buffer_offsets = new uint32_t[_gen_acc.color_mul_accessors.size()];
                    _RewriteMultiAttributeAccessorsDataToBuffer(_root,
                                                                _gen_acc.color_mul_accessors,
                                                                _prim.color_mul_buffer_ids,
                                                                _prim.color_mul_buffer_offsets,
                                                                _buffer,
                                                                &GLTFCompiler::_GetColorMultiplier);
                }


                // these properties apply only to DasMeshPrimitive template class
                if constexpr(std::is_base_of<DasMeshPrimitive, T>::value) {
                    // indices if they exist
                    if(_gen_acc.indices_accessor != UINT32_MAX) {
                        _RewriteSingleAttributeAccessorDataToBuffer(_root,
                                                                    _gen_acc.indices_accessor,
                                                                    _prim.index_buffer_id,
                                                                    _prim.index_buffer_offset,
                                                                    _buffer,
                                                                    &GLTFCompiler::_GetIndex);
                        _prim.draw_count = static_cast<uint32_t>(_buffer.data_ptrs.back().second / sizeof(uint32_t));
                    } else {
                        BufferAccessorData acc = _FindAccessorData(_root, _gen_acc.pos_accessor);
                        _prim.draw_count = acc.used_size / acc.unit_stride;
                    }

                    // joint properties
                    if(_gen_acc.joints_accessors.size() && _gen_acc.joints_accessors.size() == _gen_acc.weights_accessors.size()) {
                        _prim.joint_set_count = static_cast<uint32_t>(_gen_acc.joints_accessors.size());

                        // joint indices
                        _prim.joint_index_buffer_ids = new uint32_t[_gen_acc.joints_accessors.size()];
                        _prim.joint_index_buffer_offsets = new uint32_t[_gen_acc.joints_accessors.size()];
                        _RewriteMultiAttributeAccessorsDataToBuffer(_root,
                                                                    _gen_acc.joints_accessors,
                                                                    _prim.joint_index_buffer_ids,
                                                                    _prim.joint_index_buffer_offsets,
                                                                    _buffer,
                                                                    &GLTFCompiler::_GetJointIndices);

                        // joint weights
                        _prim.joint_weight_buffer_ids = new uint32_t[_gen_acc.weights_accessors.size()];
                        _prim.joint_weight_buffer_offsets = new uint32_t[_gen_acc.weights_accessors.size()];
                        _RewriteMultiAttributeAccessorsDataToBuffer(_root,
                                                                    _gen_acc.weights_accessors,
                                                                    _prim.joint_weight_buffer_ids,
                                                                    _prim.joint_weight_buffer_offsets,
                                                                    _buffer,
                                                                    &GLTFCompiler::_GetJointWeights);
                    }
                }
            }

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
