// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: DasStructures.h - DAS file format structure header
// author: Karl-Mihkel Ott

#ifndef DAS_STRUCTURES_H
#define DAS_STRUCTURES_H

#ifdef DAS_STRUCTURES_CPP
    #include <cstdint>
    #include <cstring>
    #include <cmath>
#ifdef _DEBUG
    #include <iostream>
#endif
    #include <string>
    #include <vector>

    #include <Points.h>
    #include <Vector.h>
    #include <Matrix.h>
    #include <Quaternion.h>
#endif

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
        DasProperties(const DasProperties &_props);
        DasProperties(DasProperties &&_props);

        void operator=(const DasProperties &_props);
        void operator=(DasProperties &&_props);

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
        DasBuffer(const DasBuffer &_buf);
        DasBuffer(DasBuffer &&_buf);

        void operator=(const DasBuffer &_buf);
        void operator=(DasBuffer &&_buf);

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
        DasMesh(const DasMesh &_mesh);
        DasMesh(DasMesh &&_mesh);
        ~DasMesh();

        void operator=(const DasMesh &_mesh);
        void operator=(DasMesh &&_mesh);

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
        DasMeshPrimitive(const DasMeshPrimitive &_prim);
        DasMeshPrimitive(DasMeshPrimitive &&_prim);
        ~DasMeshPrimitive();

        void operator=(const DasMeshPrimitive &_prim);
        void operator=(DasMeshPrimitive &&_prim);

        uint32_t index_buffer_id = UINT32_MAX;
        uint32_t index_buffer_offset = 0;
        uint32_t indices_count = 0;
        uint32_t vertex_buffer_id = UINT32_MAX;
        uint32_t vertex_buffer_offset = 0;
        uint32_t vertex_normal_buffer_id = UINT32_MAX;
        uint32_t vertex_normal_buffer_offset = 0;
        uint32_t vertex_tangent_buffer_id = UINT32_MAX;
        uint32_t vertex_tangent_buffer_offset = 0;

        // texture attributes
        uint32_t texture_count = 0;
        uint32_t *uv_buffer_ids = nullptr;
        uint32_t *uv_buffer_offsets = nullptr;
        uint32_t *texture_ids = nullptr;

        // color multiplier attributes
        uint32_t color_mul_count = 0;
        uint32_t *color_mul_buffer_ids = nullptr;
        uint32_t *color_mul_buffer_offsets = nullptr;

        // skeletal joint attributes
        uint32_t joint_set_count = 0;
        uint32_t *joint_index_buffer_ids = nullptr;
        uint32_t *joint_index_buffer_offsets = nullptr;
        uint32_t *joint_weight_buffer_ids = nullptr;
        uint32_t *joint_weight_buffer_offsets = nullptr;

        // morph targets
        uint32_t morph_target_count = 0;
        uint32_t *morph_targets = nullptr;
        float *morph_weights = nullptr;

        enum ValueType {
            LIBDAS_MESH_PRIMITIVE_INDEX_BUFFER_ID,
            LIBDAS_MESH_PRIMITIVE_INDEX_BUFFER_OFFSET,
            LIBDAS_MESH_PRIMITIVE_INDICES_COUNT,
            LIBDAS_MESH_PRIMITIVE_VERTEX_BUFFER_ID,
            LIBDAS_MESH_PRIMITIVE_VERTEX_BUFFER_OFFSET,
            LIBDAS_MESH_PRIMITIVE_VERTEX_NORMAL_BUFFER_ID,
            LIBDAS_MESH_PRIMITIVE_VERTEX_NORMAL_BUFFER_OFFSET,
            LIBDAS_MESH_PRIMITIVE_VERTEX_TANGENT_BUFFER_ID,
            LIBDAS_MESH_PRIMITIVE_VERTEX_TANGENT_BUFFER_OFFSET,

            LIBDAS_MESH_PRIMITIVE_TEXTURE_COUNT,
            LIBDAS_MESH_PRIMITIVE_UV_BUFFER_IDS,
            LIBDAS_MESH_PRIMITIVE_UV_BUFFER_OFFSETS,
            LIBDAS_MESH_PRIMITIVE_TEXTURE_IDS,

            LIBDAS_MESH_PRIMITIVE_COLOR_MUL_COUNT,
            LIBDAS_MESH_PRIMITIVE_COLOR_MUL_BUFFER_IDS,
            LIBDAS_MESH_PRIMITIVE_COLOR_MUL_BUFFER_OFFSETS,

            LIBDAS_MESH_PRIMITIVE_JOINT_SET_COUNT,
            LIBDAS_MESH_PRIMITIVE_JOINT_INDEX_BUFFER_IDS,
            LIBDAS_MESH_PRIMITIVE_JOINT_INDEX_BUFFER_OFFSETS,
            LIBDAS_MESH_PRIMITIVE_JOINT_WEIGHT_BUFFER_IDS,
            LIBDAS_MESH_PRIMITIVE_JOINT_WEIGHT_BUFFER_OFFSETS,

            LIBDAS_MESH_PRIMITIVE_MORPH_TARGET_COUNT,
            LIBDAS_MESH_PRIMITIVE_MORPH_TARGETS,
            LIBDAS_MESH_PRIMITIVE_MORPH_WEIGHTS
        };
    };


    /**
     * DAS scope structure that defines mesh morph target related information
     */
    struct DasMorphTarget {
        DasMorphTarget() = default;
        DasMorphTarget(const DasMorphTarget &_morph);
        DasMorphTarget(DasMorphTarget &&_morph);
        ~DasMorphTarget();

        void operator=(const DasMorphTarget &_morph);
        void operator=(DasMorphTarget &&_morph);

        uint32_t vertex_buffer_id = UINT32_MAX;
        uint32_t vertex_buffer_offset = 0;

        uint32_t vertex_normal_buffer_id = UINT32_MAX;
        uint32_t vertex_normal_buffer_offset = UINT32_MAX;
        uint32_t vertex_tangent_buffer_id = UINT32_MAX;
        uint32_t vertex_tangent_buffer_offset = 0;

        uint32_t texture_count = 0;
        uint32_t *uv_buffer_ids = nullptr;
        uint32_t *uv_buffer_offsets = nullptr;

        uint32_t color_mul_count = 0;
        uint32_t *color_mul_buffer_ids = nullptr;
        uint32_t *color_mul_buffer_offsets = nullptr;

        enum ValueType {
            LIBDAS_MORPH_TARGET_VERTEX_BUFFER_ID,
            LIBDAS_MORPH_TARGET_VERTEX_BUFFER_OFFSET,
            LIBDAS_MORPH_TARGET_TEXTURE_COUNT,
            LIBDAS_MORPH_TARGET_UV_BUFFER_IDS,
            LIBDAS_MORPH_TARGET_UV_BUFFER_OFFSETS,
            LIBDAS_MORPH_TARGET_COLOR_MUL_COUNT,
            LIBDAS_MORPH_TARGET_COLOR_MUL_BUFFER_IDS,
            LIBDAS_MORPH_TARGET_COLOR_MUL_BUFFER_OFFSETS,
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
        DasNode(const DasNode &_node);
        DasNode(DasNode &&_node);
        ~DasNode();

        void operator=(const DasNode &_node);
        void operator=(DasNode &&_node);

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
        DasScene(const DasScene &_scene);
        DasScene(DasScene &&_scene);
        ~DasScene();

        void operator=(const DasScene &_scene);
        void operator=(DasScene &&_scene);

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
        DasSkeleton(const DasSkeleton &_skel);
        DasSkeleton(DasSkeleton &&_skel);
        ~DasSkeleton();

        void operator=(const DasSkeleton &_skel);
        void operator=(DasSkeleton &&_skel);

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
        DasSkeletonJoint(const DasSkeletonJoint &_joint);
        DasSkeletonJoint(DasSkeletonJoint &&_joint);
        ~DasSkeletonJoint();

        void operator=(const DasSkeletonJoint &_joint);
        void operator=(DasSkeletonJoint &&_joint);

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
        DasAnimation(const DasAnimation &_ani);
        DasAnimation(DasAnimation &&_ani);
        ~DasAnimation();

        void operator=(const DasAnimation &_ani);
        void operator=(DasAnimation &&_ani);

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
        DasAnimationChannel() = default;
        DasAnimationChannel(const DasAnimationChannel &_ch);
        DasAnimationChannel(DasAnimationChannel &&_ch);
        ~DasAnimationChannel();

        void operator=(const DasAnimationChannel &_ch);
        void operator=(DasAnimationChannel &&_ch);

        uint32_t node_id = UINT32_MAX;
        uint32_t joint_id = UINT32_MAX;
        AnimationTarget target;
        InterpolationType interpolation;
        uint32_t keyframe_count = 0;
        uint32_t weight_count = 0;
        float *keyframes = nullptr;
        char *tangents = nullptr;
        char *target_values = nullptr;

        enum ValueType {
            LIBDAS_ANIMATION_CHANNEL_NODE_ID,
            LIBDAS_ANIMATION_CHANNEL_JOINT_ID,
            LIBDAS_ANIMATION_CHANNEL_TARGET,
            LIBDAS_ANIMATION_CHANNEL_INTERPOLATION,
            LIBDAS_ANIMATION_CHANNEL_KEYFRAME_COUNT,
            LIBDAS_ANIMATION_CHANNEL_WEIGHT_COUNT,
            LIBDAS_ANIMATION_CHANNEL_KEYFRAMES,
            LIBDAS_ANIMATION_CHANNEL_TANGENTS,
            LIBDAS_ANIMATION_CHANNEL_TARGET_VALUES
        };
    };
}

#endif

#endif
