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

    #include <Hash.h>
    #include <Api.h>
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

    // structure for containing generic vertex attributes, that can be used for indexing
    struct GenericVertexAttribute {
        GenericVertexAttribute() = default;
        inline bool operator==(const GenericVertexAttribute &_a1) const {
            return pos == _a1.pos && norm == _a1.norm && tangent == _a1.tangent && tex0 == _a1.tex0 && joints == _a1.joints && weights == _a1.weights;
        }

        Vector3<float> pos = { FLT_MAX, FLT_MAX, FLT_MAX };
        Vector3<float> norm = { FLT_MAX, FLT_MAX, FLT_MAX };
        Vector4<float> tangent = { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX };
        Vector2<float> tex0 = { FLT_MAX, FLT_MAX };
        Vector4<uint32_t> joints = { UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX };
        Vector4<float> weights = { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX };
    };


    class LIBDAS_API GLTFCompiler : private DasWriterCore {
        private:
            // images are always appended to the vector after buffers
            size_t m_buffers_size = 0;
            size_t m_images_size = 0;
            const bool m_use_raw_textures;
            std::string m_root_path;

            // buffer related
            std::vector<URIResolver> m_uri_resolvers;
            std::vector<char*> m_supplemented_buffers; // cleanup bookmarking
            std::vector<TextureReader> m_tex_readers;
            std::vector<std::vector<uint32_t>> m_unindexed_primitives;  // indices for primitives indexed per mesh

            const std::unordered_map<std::string, BufferType> m_attribute_type_map = {
                std::make_pair("POSITION", LIBDAS_BUFFER_TYPE_VERTEX),
                std::make_pair("TEXCOORD_0", LIBDAS_BUFFER_TYPE_TEXTURE_MAP),
                std::make_pair("NORMAL", LIBDAS_BUFFER_TYPE_VERTEX_NORMAL),
                std::make_pair("TANGENT", LIBDAS_BUFFER_TYPE_VERTEX_TANGENT),
                std::make_pair("COLOR_0", LIBDAS_BUFFER_TYPE_COLOR),
                std::make_pair("JOINTS_0", LIBDAS_BUFFER_TYPE_JOINTS),
                std::make_pair("WEIGHTS_0", LIBDAS_BUFFER_TYPE_WEIGHTS)
            };

            struct BufferAccessorData {
                uint32_t buffer_id = UINT32_MAX;
                uint32_t buffer_offset = UINT32_MAX; // bytes
                int32_t component_type = INT32_MAX; 
                uint32_t used_size = UINT32_MAX;     // bytes
                uint32_t unit_size = UINT32_MAX;     // bytes

                struct less {
                    bool operator()(const BufferAccessorData &_s1, const BufferAccessorData &_s2) {
                        if(_s1.buffer_offset == _s2.buffer_offset)
                            return _s1.used_size > _s2.used_size;
                        else return _s1.buffer_offset < _s2.buffer_offset;
                    }
                };

                static bool IsDuplicate(const BufferAccessorData &_s1, const BufferAccessorData &_s2) {
                    return _s1.buffer_offset == _s2.buffer_offset;
                }
            };

            std::vector<uint32_t> m_scene_node_id_table;
            std::vector<uint32_t> m_skeleton_joint_id_table;
            std::vector<std::vector<uint32_t>> m_supplemented_indices;

        private:
            BufferAccessorData _FindAccessorData(const GLTFRoot &_root, int32_t _accessor_id);
            void _CorrectOffsets(std::vector<GLTFAccessor*> &_accessors, size_t _diff, size_t _offset);
            size_t _FindPrimitiveCount(const GLTFRoot &_root);
            std::vector<size_t> _FindMeshNodes(const GLTFRoot &_root, size_t _mesh_index); // O(n)
            const std::vector<float> _FindMorphWeightsFromNodes(const GLTFRoot &_root, size_t _mesh_index); // O(n)
            auto _FindDataPtrFromOffset(const std::vector<std::pair<const char*, size_t>> &_ptrs, size_t &_offset); // O(n)

            // region finders
            std::vector<std::vector<GLTFAccessor*>> _GetAllBufferAccessorRegions(GLTFRoot &_root);
            std::vector<std::vector<BufferAccessorData>> _GetBufferIndexRegions(GLTFRoot &_root);
            std::vector<std::vector<BufferAccessorData>> _GetBufferJointRegions(GLTFRoot &_root);
            std::vector<std::vector<BufferAccessorData>> _GetBufferWeightRegions(GLTFRoot &_root);
            void _GetUnindexedMeshPrimitives(GLTFRoot &_root); // for each mesh not for each buffer

            // node flagging method
            void _FlagJointNodes(const GLTFRoot &_root);

            // common parent root finding methods
            bool _IsRootNode(const GLTFRoot &_root, int32_t _node_id, const std::vector<int32_t> &_pool);
            uint32_t _FindCommonRootJoint(const GLTFRoot &_root, const GLTFSkin &_skin);

            // supplementing methods
            uint32_t _SupplementIndices(const char *_odata, BufferAccessorData &_suppl_info, DasBuffer &_buffer);
            uint32_t _SupplementJointIndices(const char *_odata, BufferAccessorData &_suppl_info, DasBuffer &_buffer);
            uint32_t _SupplementJointWeights(const char *_odata, BufferAccessorData &_suppl_info, DasBuffer &_buffer);
            void _SupplementVertexAttrs(const std::unordered_map<GenericVertexAttribute, uint32_t, Hash<GenericVertexAttribute>> &_map, uint32_t _max, BufferType _type, DasBuffer &_buffer);

            void _CopyToBuffer(const std::vector<std::pair<const char*, size_t>> &_optrs, char *_dst, size_t _len, size_t _offset, DasBuffer &_buffer);

            // striding methods
            typedef std::vector<std::vector<GLTFAccessor*>> GLTFAccessors;
            typedef std::vector<std::vector<BufferAccessorData>> BufferAccessorDatas;
            typedef uint32_t (GLTFCompiler::*Supplement_PFN)(const char*, BufferAccessorData&, DasBuffer&);
            void _StrideBuffer(GLTFAccessors &_accessors, BufferAccessorDatas &_regions, std::vector<DasBuffer> &_buffers, Supplement_PFN _suppl_fn);
            void _StrideBuffers(GLTFRoot &_root, std::vector<DasBuffer> &_buffers);

            // indexing methods
            void _WriteIndexedData(GLTFAccessors &_accessors, std::vector<DasBuffer> &_buffers, std::unordered_map<GenericVertexAttribute, uint32_t, Hash<GenericVertexAttribute>> &_map, 
                                   const std::vector<uint32_t> &_indices, std::array<std::pair<uint64_t, BufferType>, 6> &_attr_array, uint32_t _max);
            void _IndexMeshPrimitive(GLTFRoot &_root, GLTFMeshPrimitive &_prim, std::vector<DasBuffer> &_buffers, GLTFAccessors &_accessors, uint32_t _vert_accessor, uint32_t _norm_accessor, uint32_t _tang_accessor, 
                                     uint32_t _tex_map_accessor, uint32_t _joints_accessor, uint32_t _weights_accessor);
            void _IndexGeometry(GLTFRoot &_root, GLTFAccessors &_accessors, std::vector<DasBuffer> &_buffers);
            void _CreateNewIndexRegion(GLTFRoot &_root, std::vector<GLTFAccessor*> &_accessors, GLTFMeshPrimitive &_prim, DasBuffer &_buffer, std::vector<uint32_t> &_indices);


            void _FreeSupplementedBuffers(std::vector<char*> _mem_areas);

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
