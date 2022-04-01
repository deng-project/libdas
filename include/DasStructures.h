// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: DasStructures.h - Header containing all structures and abstract data types used for reading and writing DAS files
// author: Karl-Mihkel Ott

#ifndef DAS_STRUCTURES_H
#define DAS_STRUCTURES_H

// file constant definitions
#define LIBDAS_DAS_MAGIC                0x00534144
#define LIBDAS_DAS_DEFAULT_AUTHOR       "DENG project v 1.0"


/// Buffer type definitions
typedef uint16_t BufferType;
#define LIBDAS_BUFFER_TYPE_UNKNOWN                  0x0000
#define LIBDAS_BUFFER_TYPE_VERTEX                   0x0001
#define LIBDAS_BUFFER_TYPE_TEXTURE_MAP              0x0002
#define LIBDAS_BUFFER_TYPE_VERTEX_NORMAL            0x0004
#define LIBDAS_BUFFER_TYPE_VERTEX_TANGENT           0x0008
#define LIBDAS_BUFFER_TYPE_COLOR                    0x0010
#define LIBDAS_BUFFER_TYPE_JOINTS                   0x0020
#define LIBDAS_BUFFER_TYPE_WEIGHTS                  0x0040
#define LIBDAS_BUFFER_TYPE_INDICES                  0x0080
#define LIBDAS_BUFFER_TYPE_TEXTURE_JPEG             0x0100
#define LIBDAS_BUFFER_TYPE_TEXTURE_PNG              0x0200
#define LIBDAS_BUFFER_TYPE_TEXTURE_TGA              0x0400
#define LIBDAS_BUFFER_TYPE_TEXTURE_BMP              0x0800
#define LIBDAS_BUFFER_TYPE_TEXTURE_PPM              0x1000
#define LIBDAS_BUFFER_TYPE_TEXTURE_RAW              0x2000
#define LIBDAS_BUFFER_TYPE_KEYFRAME                 0x4000  // can be either morph target keyframe or skinned animation keyframe
#define LIBDAS_BUFFER_TYPE_TIMESTAMPS               0x8000  // used only for identifying timestamp buffers from GLTF format parsing

typedef uint8_t IndexingMode;
#define LIBDAS_SEPERATE_INDICES                 0
#define LIBDAS_COMPACT_INDICES                  1


/// Animation interpolation technique definitions 
typedef uint8_t InterpolationType;
#define LIBDAS_INTERPOLATION_VALUE_LINEAR       0
#define LIBDAS_INTERPOLATION_VALUE_STEP         1
#define LIBDAS_INTERPOLATION_VALUE_CUBICSPLINE  2

/// Animation target definitions
typedef uint8_t AnimationTarget;
#define LIBDAS_ANIMATION_TARGET_WEIGHTS         0
#define LIBDAS_ANIMATION_TARGET_TRANSLATION     1
#define LIBDAS_ANIMATION_TARGET_ROTATION        2
#define LIBDAS_ANIMATION_TARGET_SCALE           3

#ifndef LIBDAS_DEFS_ONLY
namespace Libdas {

    struct DasSignature {
        uint32_t magic = LIBDAS_DAS_MAGIC;
        char padding[12] = {};
    };

    
    /**
     * DAS scope structure that defines all file properties
     */
    struct DasProperties {
        DasProperties() = default;

        // copy constructor
        DasProperties(const DasProperties &_props) : model(_props.model), author(_props.author), 
            copyright(_props.copyright), moddate(_props.moddate), default_scene(_props.default_scene) {}

        // move constructor
        DasProperties(DasProperties &&_props) : model(std::move(_props.model)), author(std::move(_props.author)), 
            copyright(std::move(_props.copyright)), moddate(_props.moddate), default_scene(_props.default_scene) {}

        void operator=(DasProperties &&_props) {
            model = std::move(_props.model);
            author = std::move(_props.author);
            copyright = std::move(_props.copyright);
            moddate = _props.moddate;
            default_scene = _props.default_scene;
        }

        std::string model;
        std::string author = LIBDAS_DAS_DEFAULT_AUTHOR;
        std::string copyright;
        uint64_t moddate = 0;
        uint32_t default_scene = 0;

        enum ValueType {
            LIBDAS_PROPERTIES_MODEL,
            LIBDAS_PROPERTIES_AUTHOR,
            LIBDAS_PROPERTIES_COPYRIGHT,
            LIBDAS_PROPERTIES_MODDATE,
            LIBDAS_PROPERTIES_DEFAULT_SCENE
        };
    };


    /**
     * DAS scope structure that defines single buffer related information
     */
    struct DasBuffer {
        DasBuffer() = default;

        DasBuffer(const DasBuffer &_buf) : data_ptrs(_buf.data_ptrs), data_len(_buf.data_len), type(_buf.type) {}

        DasBuffer(DasBuffer &&_buf) : data_ptrs(std::move(_buf.data_ptrs)), data_len(_buf.data_len), type(_buf.type) {}

        void operator=(const DasBuffer &_buf) {
            data_ptrs = _buf.data_ptrs;
            data_len = _buf.data_len;
            type = _buf.type;
        }

        std::vector<std::pair<const char*, size_t>> data_ptrs;
        uint32_t data_len = 0;
        BufferType type = 0;

        enum ValueType {
            LIBDAS_BUFFER_BUFFER_TYPE,
            LIBDAS_BUFFER_DATA_LEN,
            LIBDAS_BUFFER_DATA
        };
    };


    /**
     * DAS scope structure that defines mesh related information
     */
    struct DasMesh {
        DasMesh() = default;

        DasMesh(DasMesh &&_mesh) : name(std::move(_mesh.name)), primitive_count(_mesh.primitive_count), primitives(_mesh.primitives) {
            _mesh.primitives = nullptr;
        }

        DasMesh(const DasMesh &_mesh) : name(_mesh.name), primitive_count(_mesh.primitive_count) {
            primitives = new uint32_t[primitive_count];
            for(uint32_t i = 0; i < primitive_count; i++)
                primitives[i] = _mesh.primitives[i];
        }

        ~DasMesh() {
            delete [] primitives;
        }

        std::string name = "";
        uint32_t primitive_count = 0;
        uint32_t *primitives = nullptr;

        enum ValueType {
            LIBDAS_MESH_NAME,
            LIBDAS_MESH_PRIMITIVE_COUNT,
            LIBDAS_MESH_PRIMITIVES
        };
    };


    /**
     * DAS scope structure that defines mesh primitive related information
     */
    struct DasMeshPrimitive {
        DasMeshPrimitive() = default;

        DasMeshPrimitive(const DasMeshPrimitive &_prim) : index_buffer_id(_prim.index_buffer_id), index_buffer_offset(_prim.index_buffer_offset),
            indices_count(_prim.indices_count), vertex_buffer_id(_prim.vertex_buffer_id), vertex_buffer_offset(_prim.vertex_buffer_offset), 
            texture_id(_prim.texture_id), uv_buffer_id(_prim.uv_buffer_id), uv_buffer_offset(_prim.uv_buffer_offset),
            vertex_normal_buffer_id(_prim.vertex_normal_buffer_id), vertex_normal_buffer_offset(_prim.vertex_normal_buffer_offset), vertex_tangent_buffer_id(_prim.vertex_tangent_buffer_id),
            vertex_tangent_buffer_offset(_prim.vertex_tangent_buffer_offset), joint_index_buffer_id(_prim.joint_index_buffer_id), joint_index_buffer_offset(_prim.joint_index_buffer_offset),
            weight_buffer_id(_prim.weight_buffer_id), weight_buffer_offset(_prim.weight_buffer_offset), morph_target_count(_prim.morph_target_count), indexing_mode(_prim.indexing_mode)
        {
            if(morph_target_count) {
                morph_targets = new uint32_t[morph_target_count];
                for(uint32_t i = 0; i < morph_target_count; i++)
                    morph_targets[i] = _prim.morph_targets[i];

                if(_prim.morph_weights) {
                    morph_weights = new float[morph_target_count];
                    for(uint32_t i = 0; i < morph_target_count; i++)
                        morph_weights[i] = _prim.morph_weights[i];
                }
            }
        }

        DasMeshPrimitive(DasMeshPrimitive &&_prim) : index_buffer_id(_prim.index_buffer_id), index_buffer_offset(_prim.index_buffer_offset),
            indices_count(_prim.indices_count), vertex_buffer_id(_prim.vertex_buffer_id), vertex_buffer_offset(_prim.vertex_buffer_offset), 
            texture_id(_prim.texture_id), uv_buffer_id(_prim.uv_buffer_id), uv_buffer_offset(_prim.uv_buffer_offset),
            vertex_normal_buffer_id(_prim.vertex_normal_buffer_id), vertex_normal_buffer_offset(_prim.vertex_normal_buffer_offset), 
            vertex_tangent_buffer_id(_prim.vertex_tangent_buffer_id), vertex_tangent_buffer_offset(_prim.vertex_tangent_buffer_offset),
            joint_index_buffer_id(_prim.joint_index_buffer_id), joint_index_buffer_offset(_prim.joint_index_buffer_offset), 
            weight_buffer_id(_prim.weight_buffer_id), weight_buffer_offset(_prim.weight_buffer_offset), morph_target_count(_prim.morph_target_count),
            morph_targets(_prim.morph_targets), morph_weights(_prim.morph_weights), indexing_mode(_prim.indexing_mode)
        {
            _prim.morph_targets = nullptr;
            _prim.morph_weights = nullptr;
        }

        ~DasMeshPrimitive() {
            delete [] morph_targets;
            delete [] morph_weights;
        }

        uint32_t index_buffer_id = UINT32_MAX;
        uint32_t index_buffer_offset = 0;
        uint32_t indices_count = 0;
        uint32_t vertex_buffer_id = UINT32_MAX;
        uint32_t vertex_buffer_offset = 0;
        uint32_t texture_id = UINT32_MAX;
        uint32_t uv_buffer_id = UINT32_MAX;
        uint32_t uv_buffer_offset = 0;
        uint32_t vertex_normal_buffer_id = UINT32_MAX;
        uint32_t vertex_normal_buffer_offset = 0;
        uint32_t vertex_tangent_buffer_id = UINT32_MAX;
        uint32_t vertex_tangent_buffer_offset = 0;
        uint32_t joint_index_buffer_id = UINT32_MAX;
        uint32_t joint_index_buffer_offset = 0;
        uint32_t weight_buffer_id = UINT32_MAX;
        uint32_t weight_buffer_offset = 0;
        uint32_t morph_target_count = 0;
        uint32_t *morph_targets = nullptr;
        float *morph_weights = nullptr;
        IndexingMode indexing_mode;

        enum ValueType {
            LIBDAS_MESH_PRIMITIVE_INDEX_BUFFER_ID,
            LIBDAS_MESH_PRIMITIVE_INDEX_BUFFER_OFFSET,
            LIBDAS_MESH_PRIMITIVE_INDICES_COUNT,
            LIBDAS_MESH_PRIMITIVE_INDEXING_MODE,
            LIBDAS_MESH_PRIMITIVE_VERTEX_BUFFER_ID,
            LIBDAS_MESH_PRIMITIVE_VERTEX_BUFFER_OFFSET,
            LIBDAS_MESH_PRIMITIVE_TEXTURE_ID,
            LIBDAS_MESH_PRIMITIVE_TEXTURE_MAP_BUFFER_ID,
            LIBDAS_MESH_PRIMITIVE_TEXTURE_MAP_BUFFER_OFFSET,
            LIBDAS_MESH_PRIMITIVE_VERTEX_NORMAL_BUFFER_ID,
            LIBDAS_MESH_PRIMITIVE_VERTEX_NORMAL_BUFFER_OFFSET,
            LIBDAS_MESH_PRIMITIVE_VERTEX_TANGENT_BUFFER_ID,
            LIBDAS_MESH_PRIMITIVE_VERTEX_TANGENT_BUFFER_OFFSET,
            LIBDAS_MESH_PRIMITIVE_JOINT_INDEX_BUFFER_ID,
            LIBDAS_MESH_PRIMITIVE_JOINT_INDEX_BUFFER_OFFSET,
            LIBDAS_MESH_PRIMITIVE_WEIGHT_BUFFER_ID,
            LIBDAS_MESH_PRIMITIVE_WEIGHT_BUFFER_OFFSET,
            LIBDAS_MESH_PRIMITIVE_MORPH_TARGET_COUNT,
            LIBDAS_MESH_PRIMITIVE_MORPH_TARGETS,
            LIBDAS_MESH_PRIMITIVE_MORPH_WEIGHTS,
        };
    };


    /**
     * DAS scope structure that defines mesh morph target related information
     */
    struct DasMorphTarget {
        uint32_t vertex_buffer_id = UINT32_MAX;
        uint32_t vertex_buffer_offset = 0;
        uint32_t uv_buffer_id = UINT32_MAX;
        uint32_t uv_buffer_offset = 0;
        uint32_t vertex_normal_buffer_id = UINT32_MAX;
        uint32_t vertex_normal_buffer_offset = UINT32_MAX;
        uint32_t vertex_tangent_buffer_id = UINT32_MAX;
        uint32_t vertex_tangent_buffer_offset = 0;

        enum ValueType {
            LIBDAS_MORPH_TARGET_VERTEX_BUFFER_ID,
            LIBDAS_MORPH_TARGET_VERTEX_BUFFER_OFFSET,
            LIBDAS_MORPH_TARGET_TEXTURE_MAP_BUFFER_ID,
            LIBDAS_MORPH_TARGET_TEXTURE_MAP_BUFFER_OFFSET,
            LIBDAS_MORPH_TARGET_VERTEX_NORMAL_BUFFER_ID,
            LIBDAS_MORPH_TARGET_VERTEX_NORMAL_BUFFER_OFFSET,
            LIBDAS_MORPH_TARGET_VERTEX_TANGENT_BUFFER_ID,
            LIBDAS_MORPH_TARGET_VERTEX_TANGENT_BUFFER_OFFSET,
        };
    };


    //////////////////////////////////
    // ***** Nodes and scenes ***** //
    //////////////////////////////////
    

    /**
     * DAS scope structure that defines each scene node
     */
    struct DasNode {
        // default constructor
        DasNode() = default;

        // deep copy constructor
        DasNode(const DasNode &_node) : name(_node.name), children_count(_node.children_count), mesh(_node.mesh), skeleton(_node.skeleton), transform(_node.transform) {
            // copy child refs
            if(children_count) {
                children = new uint32_t[children_count];
                for(uint32_t i = 0; i < children_count; i++)
                    children[i] = _node.children[i];
            }
        }

        // move constructor
        DasNode(DasNode &&_node) : name(std::move(_node.name)), children_count(_node.children_count), children(_node.children), mesh(_node.mesh), skeleton(_node.skeleton), transform(_node.transform) {
            _node.children = nullptr;
        }

        ~DasNode() {
            delete [] children;
        }

        std::string name = "";
        uint32_t children_count = 0;
        uint32_t *children = nullptr;
        uint32_t mesh = UINT32_MAX;
        uint32_t skeleton = UINT32_MAX;
        Matrix4<float> transform;

        // value types
        enum ValueType {
            LIBDAS_NODE_NAME,
            LIBDAS_NODE_CHILDREN_COUNT,
            LIBDAS_NODE_CHILDREN,
            LIBDAS_NODE_MESH,
            LIBDAS_NODE_SKELETON,
            LIBDAS_NODE_TRANSFORM
        };
    };


    /**
     * DAS scope structure that defines the scene hierarchy structure
     */
    struct DasScene {
        DasScene() = default;
        
        DasScene(const DasScene &_scene) : name(_scene.name), node_count(_scene.node_count), root_count(_scene.root_count) {
            if(node_count) {
                nodes = new uint32_t[node_count];
                for(uint32_t i = 0; i < node_count; i++)
                    nodes[i] = _scene.nodes[i];
            }

            if(root_count) {
                roots = new uint32_t[root_count];
                for(uint32_t i = 0; i < root_count; i++)
                    roots[i] = _scene.roots[i];
            }
        }

        ~DasScene() {
            delete [] nodes;
            delete [] roots;
        }

        std::string name = "";
        uint32_t node_count = 0;
        uint32_t *nodes = nullptr;

        // custom members not included in the file specification
        uint32_t root_count = 0;
        uint32_t *roots = nullptr;

        // value types
        enum ValueType {
            LIBDAS_SCENE_NAME,
            LIBDAS_SCENE_NODE_COUNT,
            LIBDAS_SCENE_NODES
        };
    };


    /////////////////////////////////////
    // ***** Skeleton structures ***** //
    /////////////////////////////////////
    

    /**
     * DAS scope structure that defines a skeleton
     */
    struct DasSkeleton {
        DasSkeleton() = default;

        // deep copy constructor
        DasSkeleton(const DasSkeleton &_skel) : name(_skel.name), joint_count(_skel.joint_count) {
            if(_skel.joint_count) {
                joints = new uint32_t[joint_count];
                for(uint32_t i = 0; i < joint_count; i++)
                    joints[i] = _skel.joints[i];
            }
        }

        // move constructor
        DasSkeleton(DasSkeleton &&_skel) : name(std::move(_skel.name)), joint_count(_skel.joint_count), joints(_skel.joints) {
            _skel.joints = nullptr;
        }

        ~DasSkeleton() {
            delete [] joints;
        }

        std::string name;
        uint32_t parent = UINT32_MAX;
        uint32_t joint_count = 0;
        uint32_t *joints = nullptr;

        enum ValueType {
            LIBDAS_SKELETON_NAME,
            LIBDAS_SKELETON_PARENT,
            LIBDAS_SKELETON_JOINT_COUNT,
            LIBDAS_SKELETON_JOINTS
        };
    };

    
    /**
     * DAS scope structure that defines a skeleton joint
     */
    struct DasSkeletonJoint {
        DasSkeletonJoint() = default;

        DasSkeletonJoint(const DasSkeletonJoint &_joint) : inverse_bind_pos(_joint.inverse_bind_pos), name(_joint.name), children_count(_joint.children_count),
            scale(_joint.scale), rotation(_joint.rotation), translation(_joint.translation) 
        {
            if(children_count) {
                children = new uint32_t[children_count];
                for(uint32_t i = 0; i < children_count; i++)
                    children[i] = _joint.children[i];
            }
        }

        DasSkeletonJoint(DasSkeletonJoint &&_joint) : inverse_bind_pos(_joint.inverse_bind_pos), name(std::move(_joint.name)), children_count(_joint.children_count),
            children(_joint.children), scale(_joint.scale), rotation(_joint.rotation), translation(_joint.translation) 
        {
            _joint.children = nullptr;
        }

        ~DasSkeletonJoint() {
            delete [] children;
        }

        Matrix4<float> inverse_bind_pos;
        std::string name;
        uint32_t children_count = 0;
        uint32_t *children = nullptr;
        float scale = 1.0f;                     // uniform scale property
        Quaternion rotation;
        Point3D<float> translation;

        enum ValueType {
            LIBDAS_SKELETON_JOINT_INVERSE_BIND_POS,
            LIBDAS_SKELETON_JOINT_NAME,
            LIBDAS_SKELETON_JOINT_CHILDREN_COUNT,
            LIBDAS_SKELETON_JOINT_CHILDREN,
            LIBDAS_SKELETON_JOINT_SCALE,
            LIBDAS_SKELETON_JOINT_ROTATION,
            LIBDAS_SKELETON_JOINT_TRANSLATION,
        };
    };


    /**
     * DAS scope structure that defines animation related information
     */
    struct DasAnimation {
        DasAnimation() = default;

        // deep copy constructor
        DasAnimation(const DasAnimation &_ani) : name(_ani.name), channel_count(_ani.channel_count) {
            // channels are present
            if(channel_count) {
                channels = new uint32_t[channel_count];
                for(uint32_t i = 0; i < channel_count; i++)
                    channels[i] = _ani.channels[i];
            }
        }

        // move constructor
        DasAnimation(DasAnimation &&_ani) : name(std::move(_ani.name)), channel_count(_ani.channel_count), channels(_ani.channels) {
            _ani.channels = nullptr;
        }

        ~DasAnimation() {
            delete [] channels;
        }

        std::string name;
        uint32_t channel_count = 0;
        uint32_t *channels = nullptr;

        enum ValueType {
            LIBDAS_ANIMATION_NAME,
            LIBDAS_ANIMATION_CHANNEL_COUNT,
            LIBDAS_ANIMATION_CHANNELS
        };
    };


    struct DasAnimationChannel {
        uint32_t node_id;
        AnimationTarget target;
        uint32_t weight_count = 0;
        InterpolationType interpolation;
        uint32_t keyframe_count = 0;
        uint32_t keyframe_buffer_id = UINT32_MAX;
        uint32_t keyframe_buffer_offset = 0;
        uint32_t target_value_buffer_id = UINT32_MAX;
        uint32_t target_value_buffer_offset = 0;

        enum ValueType {
            LIBDAS_ANIMATION_CHANNEL_NODE_ID,
            LIBDAS_ANIMATION_CHANNEL_TARGET,
            LIBDAS_ANIMATION_CHANNEL_INTERPOLATION,
            LIBDAS_ANIMATION_CHANNEL_KEYFRAME_COUNT,
            LIBDAS_ANIMATION_CHANNEL_KEYFRAME_BUFFER_ID,
            LIBDAS_ANIMATION_CHANNEL_KEYFRAME_BUFFER_OFFSET,
            LIBDAS_ANIMATION_CHANNEL_TARGET_VALUE_BUFFER_ID,
            LIBDAS_ANIMATION_CHANNEL_TARGET_VALUE_BUFFER_OFFSET
        };
    };
}

#endif

#endif
