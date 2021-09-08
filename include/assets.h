/// DENG: dynamic engine - small but powerful 3D game engine
/// licence: Apache, see LICENCE file
/// file: assets.h - DENG asset and texture structure definitions 
/// author: Karl-Mihkel Ott

#ifndef ASSETS_H
#define ASSETS_H

#define DAS_DEFAULT_DIFFUSE_COLOR       (ColorData) {0.6f, 0.6f, 0.6f, 1.0f}   
#define DAS_DEFAULT_AMBIENT_COLOR       (ColorData) {0.4f, 0.4f, 0.4f, 1.0f}   
#define DAS_DEFAULT_SPECULAR_COLOR       (ColorData) {0.0f, 0.0f, 0.0f, 1.0f}   
#define FNAME_LEN       255
#define UUID_LEN        33


// Boolean definitions for no_cleanup variable
#define CLEANUP                                     false
#define NO_CLEANUP                                  true

/// Universal asset element offset container struct 
typedef struct OffsetData {
    // Merged vertex data offset
    uint64_t mer_offset;

    // Vertices offsets
    uint64_t pos_offset;
    uint64_t tex_offset;
    uint64_t nor_offset;

    // Indices buffer offset
    uint64_t ind_offset;

    // Color uniform buffer offset
    uint64_t ubo_offset;
} OffsetData;


#pragma pack(4)

/// Base structures for storing model vertices data 
typedef struct PosData {
    float vert_x;
    float vert_y;
    float vert_z;
    void *tri;        // Pointer to any adjacent triangle
} PosData;


typedef struct PosData2D {
    float vert_x;
    float vert_y;
    void *tri;        // Pointer to any adjacent triangle
} PosData2D;


/// Base struct for storing information about model's 
/// texture mapping vertices
typedef struct TextureData {
    float tex_x;
    float tex_y;
} TextureData;


/// Base struct for storing information about model's
/// vertex normals vertices
typedef struct NormalData {
    float nor_x;
    float nor_y;
    float nor_z;
} NormalData;


/// Base struct for asset vertices RGB color specification
typedef struct ColorData {
    float col_r;
    float col_g;
    float col_b;
    float col_a;
} ColorData;


/// Vertex data types for OpenGL
typedef struct MergedVertex3D {
    PosData pos;

    void *t; // any adjacent triangle 
} MergedVertex3D;


/// Vertex data types for OpenGL
typedef struct GL3DVertex {
    PosData pos;
    TextureData tex;
    NormalData norm;
} GL3DVertex;


typedef struct GL3DVertexUnmapped {
    PosData pos;
    NormalData norm;
} GL3DVertexUnmapped;


typedef struct GL2DVertex {
    PosData2D pos;
    TextureData tex;
} GL2DVertex;


typedef PosData2D GL2DVertexUnmapped;


/// Dynamic vertices structures
typedef struct VertDynamic3D {
    struct {
        PosData *pos;
        uint64_t pn;
        TextureData *tex;
        uint64_t tn;
        NormalData *norm;
        uint64_t nn;
    } mul;

    struct {
        GL3DVertex *vert;
        GL3DVertexUnmapped *uvert;
        uint64_t n;
    } mer;
} VertDynamic3D;


typedef union VertDynamic2D {
    struct {
        PosData2D *pos;
        uint64_t pn;
        TextureData *tex;
        uint64_t tn;
        uint32_t hier;
    } mul;

    struct {
        GL2DVertex *vert;
        GL2DVertexUnmapped *uvert;
        uint64_t n;
    } mer;
} VertDynamic2D;


/// Universal dynamic vertices container union for both 3D and 2D assets
typedef union VertDynamic {
    VertDynamic2D v2d;
    VertDynamic3D v3d;
} VertDynamic;


/// Structure for universal heap allocated indices data storage
typedef struct IndicesDynamic {
    uint32_t *pos;
    uint32_t *tex;
    uint32_t *norm;
    uint32_t *gl;
    uint64_t n;
} IndicesDynamic;


/// Structure for universal texture image bitmap data storage
typedef struct PixelDataDynamic {
    uint8_t *pixel_data;
    uint64_t size;
    uint16_t width;
    uint16_t height;
    uint64_t memory_offset;
} PixelDataDynamic;


/// Data structure for containing information about mesh triangle
typedef struct MeshTriangle {
    PosData *pos[3];
    TextureData *tex[3];
    NormalData *nor[3];
    struct MeshTriangle *nbr[3];
    uint16_t nnbr; // Next neighbour to set
} MeshTriangle;


/// Data structure for keeping information about edges
typedef struct Edge {
    PosData *pos[2];
    MeshTriangle *adj_tri[2];
} Edge;


/// Data structure for containing information about mesh
typedef struct Mesh {
    PosData *pos;
    uint64_t pn;

    uint32_t *verti[3]; // vertex indicies
    uint32_t *nbri[3]; // neighbouring triangle indices
    uint32_t *vert_trii[3]; // index of any adjacent triangle
} Mesh;


/// Specify the type of the asset 
typedef enum AssetMode {
    DAS_ASSET_MODE_UNDEFINED                        = -1,
    DAS_ASSET_MODE_3D_TEXTURE_MAPPED                = 0,
    __DAS_ASSET_MODE_3D_TEXTURE_MAPPED_UNOR         = 1,
    DAS_ASSET_MODE_3D_UNMAPPED                      = 2,
    __DAS_ASSET_MODE_3D_UNMAPPED_UNOR               = 3,
    DAS_ASSET_MODE_2D_TEXTURE_MAPPED                = 4,
    DAS_ASSET_MODE_2D_UNMAPPED                      = 5,
    DAS_ASSET_MODE_FIRST_3D                         = DAS_ASSET_MODE_3D_TEXTURE_MAPPED,
    DAS_ASSET_MODE_LAST_3D                          = __DAS_ASSET_MODE_3D_UNMAPPED_UNOR,
    DAS_ASSET_MODE_FIRST_2D                         = DAS_ASSET_MODE_2D_TEXTURE_MAPPED,
    DAS_ASSET_MODE_LAST_2D                          = DAS_ASSET_MODE_2D_UNMAPPED,
    DAS_ASSET_MODE_FIRST                            = DAS_ASSET_MODE_UNDEFINED,
    DAS_ASSET_MODE_LAST                             = DAS_ASSET_MODE_2D_UNMAPPED,
} AssetMode;


/// Main asset structure for 2D and 3D assets
typedef struct Asset {
    uuid_t uuid;
    uuid_t *tex_uuid;           // Texture id that the current asset is bound to (can be set to zero, if unmapped)
    uuid_t *vk_id;              // Vulkan asset id, that is generated when Vulkan is used as the backend api
    char src[FNAME_LEN];                 // Asset name that is displayed in editors 
    char *meta;

    // Temporary variables, since no material system is present
    ColorData diffuse;   // Asset diffuse color property
    ColorData ambient;   // Asset ambient light intensity
    ColorData specular;  // Specular light intensity
    float phong_exp;       // Phong exponent that is used, with Blinn-Phong shading
    

    AssetMode asset_mode;       // Asset mode specifier
    bool is_shown;           // False if asset is ignored from draw calls, otherwise True
    bool ignore_transform;   // Toggle transformation on and off
    bool is_opengl;          // Flag to determine if the asset is rendered in OpenGL backend renderer 
    bool force_unmap;        // Force the asset not to use texture image and texture mapping
    VertDynamic vertices;       // All asset vertices that will be passed to command buffers
    IndicesDynamic indices;     // All asset indices that will be passed to command buffers
    OffsetData offsets;         // Offsets, which are taken into account, when allocating buffer storage
} Asset;


/// DENG texture struct 
typedef struct Texture {
    uuid_t uuid;
    char pad[UUID_LEN];
    uuid_t vk_id;
    uuid_t gl_id;
    char src[FNAME_LEN];
    bool no_reg_cleanup;         // Set this as true if no automatic cleanup is wanted in registry destruction
    PixelDataDynamic pixel_data;
} Texture;

#pragma pack()

#endif
