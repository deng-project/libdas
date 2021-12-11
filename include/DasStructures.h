#ifndef DAS_STRUCTURES_H
#define DAS_STRUCTURES_H

// file constant definitions
#define LIBDAS_DAS_MAGIC 0x00534144
#define LIBDAS_DAS_DEFAULT_AUTHOR "DENG project v 1.0"


/// Buffer type definitions
typedef uint8_t BufferType;
#define LIBDAS_BUFFER_TYPE_VERTEX           0
#define LIBDAS_BUFFER_TYPE_TEXTURE_MAP      1
#define LIBDAS_BUFFER_TYPE_VERTEX_NORMAL    2
#define LIBDAS_BUFFER_TYPE_INDICES          3
#define LIBDAS_BUFFER_TYPE_TEXTURE_JPEG     4
#define LIBDAS_BUFFER_TYPE_TEXTURE_PNG      5
#define LIBDAS_BUFFER_TYPE_TEXTURE_TGA      6
#define LIBDAS_BUFFER_TYPE_TEXTURE_BMP      7
#define LIBDAS_BUFFER_TYPE_TEXTURE_PPM      8
#define LIBDAS_BUFFER_TYPE_TEXTURE_RAW      9
#define LIBDAS_BUFFER_TYPE_UNKNOWN          10

/// Animation interpolation technique definitions 
typedef uint8_t InterpolationValue;
#define LIBDAS_INTERPOLATION_VALUE_LINEAR       11
#define LIBDAS_INTERPOLATION_VALUE_STEP         12
#define LIBDAS_INTERPOLATION_VALUE_CUBICSPLINE  13

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
        bool compression;

        enum ValueType {
            LIBDAS_PROPERTIES_MODEL,
            LIBDAS_PROPERTIES_AUTHOR,
            LIBDAS_PROPERTIES_COPYRIGHT,
            LIBDAS_PROPERTIES_MODDATE,
            LIBDAS_PROPERTIES_COMPRESSION
        } val_type;
    };


    /**
     * Data structure for containing indicies information (default indices are: -1)
     */
    struct DasFace {
        uint32_t position;
        uint32_t texture;
        uint32_t normal;
    };


    /**
     * DAS scope structure that defines single buffer related information
     */
    struct DasBuffer {
        BufferType type;
        uint32_t data_len = 0;
        std::vector<std::pair<const char*, size_t>> data_ptrs;

        enum ValueType {
            LIBDAS_BUFFER_BUFFER_TYPE,
            LIBDAS_BUFFER_DATA_LEN,
            LIBDAS_BUFFER_DATA
        } val_type;
    };


    /**
     * DAS scope structure that defines model related information
     */
    struct DasModel {
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
        Matrix4<float> transform;

        enum ValueType {
            LIBDAS_MODEL_NAME,
            LIBDAS_MODEL_INDEX_BUFFER_ID,
            LIBDAS_MODEL_INDEX_BUFFER_OFFSET,
            LIBDAS_MODEL_INDICES_COUNT,
            LIBDAS_MODEL_VERTEX_BUFFER_ID,
            LIBDAS_MODEL_VERTEX_BUFFER_OFFSET,
            LIBDAS_MODEL_TEXTURE_ID,
            LIBDAS_MODEL_TEXTURE_MAP_BUFFER_ID,
            LIBDAS_MODEL_TEXTURE_MAP_BUFFER_OFFSET,
            LIBDAS_MODEL_VERTEX_NORMAL_BUFFER_ID,
            LIBDAS_MODEL_VERTEX_NORMAL_BUFFER_OFFSET,
            LIBDAS_MODEL_TRANSFORM
        } val_type;
    };

    /*****************************************/
    /**** Scene and animation structures *****/
    /*****************************************/


    /**
     * DAS subscope structure that defines keyframe animation keyframes
     */
    struct DasKeyframe {
        // NOTE: timestamp value 0 is never used and by default the initial keyframe is considered to be the models position value
        uint32_t timestamp;
        uint32_t vertex_buffer_id;
        uint32_t vertex_buffer_offset;
        uint32_t texture_map_buffer_id = UINT32_MAX;
        uint32_t texture_map_buffer_offset = 0;
        uint32_t vertex_normal_buffer_id = UINT32_MAX;
        uint32_t vertex_normal_buffer_offset = 0;


        enum ValueType {
            LIBDAS_KEYFRAME_TIMESTAMP,
            LIBDAS_KEYFRAME_VERTEX_BUFFER_ID,
            LIBDAS_KEYFRAME_VERTEX_BUFFER_OFFSET,
            LIBDAS_KEYFRAME_TEXTURE_MAP_VERTICES_BUFFER_ID,
            LIBDAS_KEYFRAME_TEXTURE_MAP_VERTICES_BUFFER_OFFSET,
            LIBDAS_KEYFRAME_VERTEX_NORMAL_BUFFER_ID,
            LIBDAS_KEYFRAME_VERTEX_NORMAL_BUFFER_OFFSET
        } val_type;
    };


    /**
     * DAS scope structure that defines animation related information
     */
    struct DasAnimation {
        std::string name = "";
        uint32_t model;
        uint32_t length;
        std::vector<DasKeyframe> keyframes;
        InterpolationValue interpolation = LIBDAS_INTERPOLATION_VALUE_LINEAR;

        enum ValueType {
            LIBDAS_ANIMATION_NAME,
            LIBDAS_ANIMATION_MODEL,
            LIBDAS_ANIMATION_LENGTH,
            LIBDAS_ANIMATION_INTERPOLATION
        } val_type;
    };


    /**
     * DAS scope structure that defines each scene node
     */
    struct DasSceneNode {
        ~DasSceneNode() {
            if(children_count)
                delete [] children;
            if(model_count)
                delete [] models;
            if(animation_count)
                delete [] animations;
        }

        std::string name = "";
        uint32_t children_count = 0;
        uint32_t *children = nullptr;
        uint32_t model_count = 0;
        uint32_t *models = nullptr;
        uint32_t animation_count = 0;
        uint32_t *animations = nullptr;
        Matrix4<float> transform;

        // value types
        enum ValueType {
            LIBDAS_SCENE_NODE_NAME,
            LIBDAS_SCENE_NODE_CHILDREN_COUNT,
            LIBDAS_SCENE_NODE_CHILDREN,
            LIBDAS_SCENE_NODE_MODEL_COUNT,
            LIBDAS_SCENE_NODE_MODELS,
            LIBDAS_SCENE_NODE_ANIMATION_COUNT,
            LIBDAS_SCENE_NODE_ANIMATIONS,
            LIBDAS_SCENE_NODE_TRANSFORM
        } val_types;
    };


    /**
     * DAS scope structure that defines the scene hierarchy structure
     */
    struct DasScene {
        std::string name = "";
        std::vector<DasSceneNode> nodes;

        // value types
        enum ValueType {
            LIBDAS_SCENE_NAME
        } val_types;
    };
}

#endif
