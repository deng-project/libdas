#ifndef DAS_STRUCTURES_H
#define DAS_STRUCTURES_H

// file constant definitions
#define LIBDAS_DAS_MAGIC 0x00534144
#define LIBDAS_DAS_DEFAULT_AUTHOR "DENG project v 1.0"

typedef uint8_t BufferType;
#define LIBDAS_BUFFER_TYPE_VERTEX           0
#define LIBDAS_BUFFER_TYPE_TEXTURE_MAP      1
#define LIBDAS_BUFFER_TYPE_VERTEX_NORMAL    2
#define LIBDAS_BUFFER_TYPE_INDICIES         3
#define LIBDAS_BUFFER_TYPE_TEXTURE_JPEG     4
#define LIBDAS_BUFFER_TYPE_TEXTURE_PNG      5
#define LIBDAS_BUFFER_TYPE_TEXTURE_TGA      6
#define LIBDAS_BUFFER_TYPE_TEXTURE_BMP      7
#define LIBDAS_BUFFER_TYPE_TEXTURE_PPM      8

namespace Libdas {

    struct DasSignature {
        uint32_t magic = LIBDAS_DAS_MAGIC;
        char padding[12] = {};
    };

    
    struct DasProperties {
        std::string model;
        std::string author = LIBDAS_DAS_DEFAULT_AUTHOR;
        std::string copyright;
        uint64_t moddate;
        bool compression;
    };


    struct DasBuffer {
        BufferType type;
        uint32_t data_len = 0;
        char *data;
    };


    struct DasModel {
        std::string name = "";
        uint32_t index_buffer_id;
        uint32_t index_buffer_offset;
        uint32_t indices_count;
        uint32_t vertex_buffer_id;
        uint32_t vertex_buffer_offset;
        uint32_t texture_id;
        uint32_t texture_map_buffer_id;
        uint32_t texture_map_buffer_offset;
        uint32_t vertex_normal_buffer_id;
        uint32_t vertex_normal_buffer_offset;
        Matrix4<float> transform;
    };
}

#endif
