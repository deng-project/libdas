// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: DasStructures.h - Header containing all structures and abstract data types used for reading and writing DAS files
// author: Karl-Mihkel Ott

#ifndef DAS_STRUCTURES_H
#define DAS_STRUCTURES_H

// file constant definitions
#define LIBDAS_DAS_MAGIC 0x00534144
#define LIBDAS_DAS_DEFAULT_AUTHOR "DENG project v 1.0"

// some limits for avoiding memory overflow and too much allocation
#define MAX_CHILDREN_PER_NODE           128
#define MAX_MESHES_PER_NODE             1024
#define MAX_ANIMATIONS_PER_NODE         512
#define MAX_SKELETONS_PER_NODE          2048

#define MAX_NODES_PER_SCENE             128
#define MAX_JOINTS_PER_SKELETON         128
#define MAX_KEYFRAMES_PER_ANIMATION     16384


/// Buffer type definitions
typedef uint16_t BufferType;
#define LIBDAS_BUFFER_TYPE_UNKNOWN                  0x0000
#define LIBDAS_BUFFER_TYPE_VERTEX                   0x0001
#define LIBDAS_BUFFER_TYPE_TEXTURE_MAP              0x0002
#define LIBDAS_BUFFER_TYPE_VERTEX_NORMAL            0x0004
#define LIBDAS_BUFFER_TYPE_VERTEX_TANGENT           0x0008
#define LIBDAS_BUFFER_TYPE_INDICES                  0x0010
#define LIBDAS_BUFFER_TYPE_TEXTURE_JPEG             0x0020
#define LIBDAS_BUFFER_TYPE_TEXTURE_PNG              0x0040
#define LIBDAS_BUFFER_TYPE_TEXTURE_TGA              0x0080
#define LIBDAS_BUFFER_TYPE_TEXTURE_BMP              0x0100
#define LIBDAS_BUFFER_TYPE_TEXTURE_PPM              0x0200
#define LIBDAS_BUFFER_TYPE_TEXTURE_RAW              0x0400
#define LIBDAS_BUFFER_TYPE_KEYFRAME                 0x0800  // can be either morph target keyframe or skinned animation keyframe
#define LIBDAS_BUFFER_TYPE_COLOR                    0x1000
#define LIBDAS_BUFFER_TYPE_WEIGHTS                  0x2000
#define LIBDAS_BUFFER_TYPE_JOINTS                   0x4000

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
        std::string model;
        std::string author = LIBDAS_DAS_DEFAULT_AUTHOR;
        std::string copyright;
        uint64_t moddate;
        uint32_t default_scene = 0;
        bool compression;

        enum ValueType {
            LIBDAS_PROPERTIES_MODEL,
            LIBDAS_PROPERTIES_AUTHOR,
            LIBDAS_PROPERTIES_COPYRIGHT,
            LIBDAS_PROPERTIES_MODDATE,
            LIBDAS_PROPERTIES_COMPRESSION,
            LIBDAS_PROPERTIES_DEFAULT_SCENE
        } val_type;
    };


    /**
     * Data structure for containing indices information (default indices are all -1)
     */
    struct DasFace {
        uint32_t position = UINT32_MAX;
        uint32_t texture = UINT32_MAX;
        uint32_t normal = UINT32_MAX;
    };


    /**
     * DAS scope structure that defines single buffer related information
     */
    struct DasBuffer {
        std::vector<std::pair<const char*, size_t>> data_ptrs;
        uint32_t data_len = 0;
        BufferType type;

        enum ValueType {
            LIBDAS_BUFFER_BUFFER_TYPE,
            LIBDAS_BUFFER_DATA_LEN,
            LIBDAS_BUFFER_DATA
        } val_type;
    };


    /**
     * DAS scope structure that defines mesh related information
     */
    struct DasMesh {
        std::string name = "";
        // buffer ids with values UINT32_MAX are reserved values, indicating that no value is used
        uint32_t index_buffer_id = 0;
        uint32_t index_buffer_offset = 0;
        uint32_t indices_count = 0;
        uint32_t vertex_buffer_id = 0;
        uint32_t vertex_buffer_offset = 0;
        uint32_t texture_id = UINT32_MAX; 
        uint32_t texture_map_buffer_id = UINT32_MAX;
        uint32_t texture_map_buffer_offset = 0;
        uint32_t vertex_normal_buffer_id = UINT32_MAX;
        uint32_t vertex_normal_buffer_offset = 0;

        enum ValueType {
            LIBDAS_MESH_NAME,
            LIBDAS_MESH_INDEX_BUFFER_ID,
            LIBDAS_MESH_INDEX_BUFFER_OFFSET,
            LIBDAS_MESH_INDICES_COUNT,
            LIBDAS_MESH_VERTEX_BUFFER_ID,
            LIBDAS_MESH_VERTEX_BUFFER_OFFSET,
            LIBDAS_MESH_TEXTURE_ID,
            LIBDAS_MESH_TEXTURE_MAP_BUFFER_ID,
            LIBDAS_MESH_TEXTURE_MAP_BUFFER_OFFSET,
            LIBDAS_MESH_VERTEX_NORMAL_BUFFER_ID,
            LIBDAS_MESH_VERTEX_NORMAL_BUFFER_OFFSET,
        } val_type;
    };


    //////////////////////////////////
    // ***** Nodes and scenes ***** //
    //////////////////////////////////
    

    /**
     * DAS scope structure that defines each scene node
     */
    struct DasNode {
        // make memory management semi-managed
        ~DasNode() {
            delete [] children;
            delete [] meshes;
            delete [] animations;
            delete [] skeletons;
        }

        std::string name = "";
        uint32_t children_count = 0;
        uint32_t *children = nullptr;
        uint32_t mesh_count = 0;
        uint32_t *meshes = nullptr;
        uint32_t animation_count = 0;
        uint32_t *animations = nullptr;
        uint32_t skeleton_count = 0;
        uint32_t *skeletons = nullptr;
        Matrix4<float> transform;

        // value types
        enum ValueType {
            LIBDAS_NODE_NAME,
            LIBDAS_NODE_CHILDREN_COUNT,
            LIBDAS_NODE_CHILDREN,
            LIBDAS_NODE_MESH_COUNT,
            LIBDAS_NODE_MESHES,
            LIBDAS_NODE_ANIMATION_COUNT,
            LIBDAS_NODE_ANIMATIONS,
            LIBDAS_NODE_SKELETON_COUNT,
            LIBDAS_NODE_SKELETONS,
            LIBDAS_NODE_TRANSFORM
        } val_types;
    };


    /**
     * DAS scope structure that defines the scene hierarchy structure
     */
    struct DasScene {
        ~DasScene() {
            delete [] nodes;
        }

        std::string name = "";
        uint32_t node_count = 0;
        uint32_t *nodes = nullptr;

        // value types
        enum ValueType {
            LIBDAS_SCENE_NAME,
            LIBDAS_SCENE_NODE_COUNT,
            LIBDAS_SCENE_NODES
        } val_types;
    };


    /////////////////////////////////////
    // ***** Skeleton structures ***** //
    /////////////////////////////////////
    

    /**
     * DAS scope structure that defines a skeleton
     */
    struct DasSkeleton {
        ~DasSkeleton() {
            delete [] joints;
        }

        std::string name;
        uint32_t joint_count = 0;
        uint32_t *joints = nullptr;

        enum ValueType {
            LIBDAS_SKELETON_NAME,
            LIBDAS_SKELETON_JOINT_COUNT,
            LIBDAS_SKELETON_JOINTS
        } val_type;
    };

    
    /**
     * DAS scope structure that defines a skeleton joint
     */
    struct DasSkeletonJoint {
        Matrix4<float> inverse_bind_pos;
        Quaternion rotation;
        Vector3<float> translation;
        std::string name;
        uint32_t parent = UINT32_MAX;
        float scale = 1.0f;             // uniform scale property

        enum ValueType {
            LIBDAS_SKELETON_JOINT_INVERSE_BIND_POS,
            LIBDAS_SKELETON_JOINT_ROTATION,
            LIBDAS_SKELETON_JOINT_TRANSLATION,
            LIBDAS_SKELETON_JOINT_NAME,
            LIBDAS_SKELETON_JOINT_PARENT,
            LIBDAS_SKELETON_JOINT_SCALE
        } val_type;
    };


    /**
     * DAS scope structure that defines animation related information
     */
    struct DasAnimation {
        ~DasAnimation() {
            delete [] keyframe_timestamps;
            delete [] interpolation_types;
            delete [] animation_targets;
        }

        std::string name;
        uint32_t node_id;
        float duration;
        uint32_t keyframe_count = 0;
        float *keyframe_timestamps = nullptr; 
        InterpolationType *interpolation_types = nullptr;
        AnimationTarget *animation_targets = nullptr;
        uint32_t keyframe_buffer_id = 0;
        uint32_t keyframe_buffer_offset = 0;

        enum ValueType {
            LIBDAS_ANIMATION_NAME,
            LIBDAS_ANIMATION_NODE_ID,
            LIBDAS_ANIMATION_DURATION,
            LIBDAS_ANIMATION_KEYFRAME_COUNT,
            LIBDAS_ANIMATION_KEYFRAME_TIMESTAMPS,
            LIBDAS_ANIMATION_INTERPOLATION_TYPES,
            LIBDAS_ANIMATION_TARGETS,
            LIBDAS_ANIMATION_KEYFRAME_BUFFER_ID,
            LIBDAS_ANIMATION_KEYFRAME_BUFFER_OFFSET
        } val_type;
    };


}

#endif
#endif
