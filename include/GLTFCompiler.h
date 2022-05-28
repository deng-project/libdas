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

    #include <Api.h>
    #include <Hash.h>
    #include <Points.h>
    #include <Vector.h>
    #include <Matrix.h>
    #include <Quaternion.h>

    #include <STDUtils.h>
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

                Vector3<float> pos;
                Vector3<float> normal;
                Vector4<float> tangent;

                std::vector<Vector2<float>> uv;
                std::vector<Vector4<float>> color;
                std::vector<Vector4<uint16_t>> joints;
                std::vector<Vector4<float>> weights;
            };

            // indexing related
            std::vector<std::vector<uint32_t>> m_unindexed_primitives;              // indices for primitives indexed per mesh
            std::vector<GenericVertexAttribute> m_indexed_attributes;
            std::vector<uint32_t> m_supplemented_indices;
            uint32_t m_multi_spec_vertex_attribute_index = 0;

            std::vector<uint32_t> m_scene_node_id_table;
            std::vector<uint32_t> m_skeleton_joint_id_table;

            // data for n attribute types
            std::vector<uint32_t> m_texcoord_accessors;
            std::vector<uint32_t> m_colormul_accessors;
            std::vector<uint32_t> m_joints_accessors;
            std::vector<uint32_t> m_weights_accessors;

        private:
            uint32_t _FindKhronosComponentSize(int32_t _component_type);
            BufferAccessorData _FindAccessorData(const GLTFRoot &_root, int32_t _accessor_id);
            uint32_t _CorrectOffsets(GLTFRoot &_root, std::vector<GLTFAccessor*> &_accessors, uint32_t _diff, size_t _offset, DasBuffer &_buffer);
            size_t _FindPrimitiveCount(const GLTFRoot &_root);
            std::vector<size_t> _FindMeshNodes(const GLTFRoot &_root, size_t _mesh_index); // O(n)
            const std::vector<float> _FindMorphWeightsFromNodes(const GLTFRoot &_root, size_t _mesh_index); // O(n)
            auto _FindDataPtrFromOffset(const std::vector<std::pair<const char*, size_t>> &_ptrs, size_t &_offset); // O(n)

            // multi spec vertex attribute handling
            uint32_t _FindMultiSpecVertexAttributeIndex(const std::string &_attr_name);
            uint32_t _EnumerateMultiSpecVertexAttributes(const GLTFMeshPrimitive::AttributesType &_attrs, const std::string &_attr_name_core);
            void _FindMultiSpecVertexAttributeAccessors(const GLTFMeshPrimitive::AttributesType &_attrs);

            template<typename T>
            void _CopyUniversalMultiSpecVertexAttributes(const GLTFRoot &_root, T &_prim) {
                // allocate memory for custom attributes
                _prim.texture_count = static_cast<uint32_t>(m_texcoord_accessors.size());
                if(_prim.texture_count) {
                    _prim.uv_buffer_ids = new uint32_t[_prim.texture_count];
                    _prim.uv_buffer_offsets = new uint32_t[_prim.texture_count];

                    // NOTE: TEXTUREIDS value is ignored for now

                    // write uv buffer data to das structure
                    for(uint32_t i = 0; i < _prim.texture_count; i++) {
                        BufferAccessorData accessor_data = _FindAccessorData(_root, m_texcoord_accessors[i]);
                        _prim.uv_buffer_ids[i] = accessor_data.buffer_id;
                        _prim.uv_buffer_offsets[i] = accessor_data.buffer_offset;
                    }
                }

                _prim.color_mul_count = m_colormul_accessors.size();
                if(_prim.color_mul_count) {
                    _prim.color_mul_buffer_ids = new uint32_t[_prim.color_mul_count];
                    _prim.color_mul_buffer_offsets = new uint32_t[_prim.color_mul_count];
                    
                    // write color multiplier buffer data to das structure
                    for(uint32_t i = 0; i < _prim.color_mul_count; i++) {
                        BufferAccessorData accessor_data = _FindAccessorData(_root, m_colormul_accessors[i]);
                        _prim.color_mul_buffer_ids[i] = accessor_data.buffer_id;
                        _prim.color_mul_buffer_offsets[i] = accessor_data.buffer_offset;
                    }
                }
            }

            // region finders
            std::vector<std::vector<GLTFAccessor*>> _GetAllBufferAccessorRegions(GLTFRoot &_root);
            std::vector<std::vector<BufferAccessorData>> _GetInvalidStridedBufferRegions(GLTFRoot &_root);
            std::vector<std::vector<BufferAccessorData>> _GetBufferIndexRegions(GLTFRoot &_root);
            std::vector<std::vector<BufferAccessorData>> _GetBufferColorStrideRegions(GLTFRoot &_root);
            std::vector<std::vector<BufferAccessorData>> _GetBufferJointRegions(GLTFRoot &_root);
            std::vector<std::vector<BufferAccessorData>> _GetBufferWeightRegions(GLTFRoot &_root);
            std::vector<std::vector<BufferAccessorData>> _GetAnimationDataRegions(GLTFRoot &_root);
            void _GetUnindexedMeshPrimitives(GLTFRoot &_root); // for each mesh not for each buffer

            // node flagging method
            void _FlagJointNodes(const GLTFRoot &_root);

            // non-texture buffer search method
            uint32_t _FindFirstNonTextureBuffer(const std::vector<DasBuffer> &_buffers);

            // common parent root finding methods
            bool _IsRootNode(const GLTFRoot &_root, int32_t _node_id, const std::vector<int32_t> &_pool);
            uint32_t _FindCommonRootJoint(const GLTFRoot &_root, const GLTFSkin &_skin);

            // supplementing methods
            uint32_t _SupplementIndices(const char *_odata, BufferAccessorData &_suppl_info, DasBuffer &_buffer);
            uint32_t _SupplementInvalidStridedData(const char *_odata, BufferAccessorData &_suppl_info, DasBuffer &_buffer);
            uint32_t _SupplementColorMultipliers(const char *_odata, BufferAccessorData &_suppl_info, DasBuffer &_buffer);
            uint32_t _SupplementJointIndices(const char *_odata, BufferAccessorData &_suppl_info, DasBuffer &_buffer);
            uint32_t _SupplementJointWeights(const char *_odata, BufferAccessorData &_suppl_info, DasBuffer &_buffer);
            uint32_t _SupplementAnimationKeyframeData(const char *_odata, BufferAccessorData &_suppl_info, DasBuffer &_buffer);

            uint32_t _SupplementIndexedPositionVertices(const char *_odata, BufferAccessorData &_suppl_info, DasBuffer &_buffer);
            uint32_t _SupplementIndexedVertexNormals(const char *_odata, BufferAccessorData &_suppl_info, DasBuffer &_buffer);
            uint32_t _SupplementIndexedVertexTangents(const char *_odata, BufferAccessorData &_suppl_info, DasBuffer &_buffer);
            uint32_t _SupplementIndexedUVVertices(const char *_odata, BufferAccessorData &_suppl_info, DasBuffer &_buffer);
            uint32_t _SupplementIndexedColorMultipliers(const char *_odata, BufferAccessorData &_suppl_info, DasBuffer &_buffer);
            uint32_t _SupplementIndexedJointsIndices(const char *_odata, BufferAccessorData &_suppl_info, DasBuffer &_buffer);
            uint32_t _SupplementIndexedJointWeights(const char *_odata, BufferAccessorData &_suppl_info, DasBuffer &_buffer);

            void _AdjustBufferViewStrides(GLTFRoot &_root);
            void _CopyToBuffer(const std::vector<std::pair<const char*, size_t>> &_optrs, char *_dst, size_t _len, size_t _offset, DasBuffer &_buffer, uint32_t _buffer_id);

            // striding methods
            typedef std::vector<std::vector<GLTFAccessor*>> GLTFAccessors;
            typedef std::vector<std::vector<BufferAccessorData>> BufferAccessorDatas;
            typedef uint32_t (GLTFCompiler::*Supplement_PFN)(const char*, BufferAccessorData&, DasBuffer&);
            void _StrideBuffer(GLTFRoot &_root, GLTFAccessors &_accessors, BufferAccessorDatas &_regions, std::vector<DasBuffer> &_buffers, Supplement_PFN _suppl_fn);
            void _StrideBuffers(GLTFRoot &_root, std::vector<DasBuffer> &_buffers);
            void _OmitEmptyBuffers(GLTFRoot &_root, std::vector<DasBuffer> &_buffers);

            // indexing methods
            void _WriteIndexedData(GLTFRoot &_root, std::vector<DasBuffer> &_buffers, GenericVertexAttributeAccessors &_gen_acc);
            GenericVertexAttribute _GenerateGenericVertexAttribute(GLTFRoot &_root, GenericVertexAttributeAccessors &_gen_acc, std::vector<DasBuffer> &_buffers, uint32_t _index);
            void _IndexMeshPrimitive(GLTFRoot &_root, GLTFMeshPrimitive &_prim, std::vector<DasBuffer> &_buffers, GLTFAccessors &_accessors, GenericVertexAttributeAccessors &_gen_acc);
            void _IndexGeometry(GLTFRoot &_root, GLTFAccessors &_accessors, std::vector<DasBuffer> &_buffers);
            void _CreateNewIndexRegion(GLTFRoot &_root, std::vector<GLTFAccessor*> &_accessors, GLTFMeshPrimitive &_prim, DasBuffer &_buffer, const uint32_t _id);


            void _FreeSupplementedBuffers(std::vector<std::vector<char*>> _mem_areas);

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
             * Create DasMorphTarget instances from given meshes 
             * @param _root specifies a reference to GLTFRoot object that contains all necessary information
             */
            std::vector<DasMorphTarget> _CreateMorphTargets(const GLTFRoot &_root);

            /**
             * Create DasMeshPrimitive instances from given meshes
             * @param _root specifies a reference to GLTFRoot object that contains all necessary information 
             */
            std::vector<DasMeshPrimitive> _CreateMeshPrimitives(const GLTFRoot &_root);

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
             * Create DasSkeletonJoint instances from given GLTF skin nodes
             * @param _root specifies a reference to GLTFRoot object, where all GLTF data is stored
             * @return std::vector instance containing all DasSkeletonJoint objects
             */
            std::vector<DasSkeletonJoint> _CreateSkeletonJoints(const GLTFRoot &_root, const std::vector<DasBuffer> &_buffers);

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
            std::vector<DasAnimationChannel> _CreateAnimationChannels(const GLTFRoot &_root, const std::vector<DasBuffer> &_buffers);

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
