/// DENG: dynamic engine - small but powerful 3D game engine
/// licence: Apache, see LICENCE file
/// file: assets.h - DENG asset and texture structure definitions 
/// author: Karl-Mihkel Ott

#ifndef ASSETS_H
#define ASSETS_H

#define DAS_DEFAULT_DIFFUSE_COLOR       (das_ColorData) {0.6f, 0.6f, 0.6f, 1.0f}   
#define DAS_DEFAULT_AMBIENT_COLOR       (das_ColorData) {0.4f, 0.4f, 0.4f, 1.0f}   
#define DAS_DEFAULT_SPECULAR_COLOR       (das_ColorData) {0.0f, 0.0f, 0.0f, 1.0f}   
#define FNAME_LEN       255
#define UUID_LEN        33


/// Universal asset element offset container struct 
typedef struct das_OffsetData {
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
} das_OffsetData;


/// Base structures for storing model vertices data 
typedef struct das_PosData {
    float vert_x;
    float vert_y;
    float vert_z;
    void *tri;        // Pointer to any adjacent triangle
    char pad[4];
} das_PosData;


typedef struct das_PosData2D {
    float vert_x;
    float vert_y;
    void *tri;        // Pointer to any adjacent triangle
} das_PosData2D;


/// Base struct for storing information about model's 
/// texture mapping vertices
typedef struct das_TextureData {
    float tex_x;
    float tex_y;
} das_TextureData;


/// Base struct for storing information about model's
/// vertex normals vertices
typedef struct das_NormalData {
    float nor_x;
    float nor_y;
    float nor_z;
} das_NormalData;


/// Base struct for asset vertices RGB color specification
typedef struct das_ColorData {
    float col_r;
    float col_g;
    float col_b;
    float col_a;
} das_ColorData;


/// Vertex data types for OpenGL
typedef struct das_MergedVertex3D {
    das_PosData pos;

    void *t; // any adjacent triangle 
} das_MergedVertex3D;


/// Vertex data types for OpenGL
typedef struct das_GL3DVertex {
    das_PosData pos;
    das_TextureData tex;
    das_NormalData norm;
} das_GL3DVertex;


typedef struct das_GL3DVertexUnmapped {
    das_PosData pos;
    das_NormalData norm;
} das_GL3DVertexUnmapped;


typedef struct das_GL2DVertex {
    das_PosData2D pos;
    das_TextureData tex;
} das_GL2DVertex;


typedef das_PosData2D das_GL2DVertexUnmapped;


/// Dynamic vertices structures
typedef struct __das_VertDynamic3D {
    struct {
        das_PosData *pos;
        uint64_t pn;
        das_TextureData *tex;
        uint64_t tn;
        das_NormalData *norm;
        uint64_t nn;
    } mul;

    struct {
        das_GL3DVertex *vert;
        das_GL3DVertexUnmapped *uvert;
        uint64_t n;
    } mer;
} __das_VertDynamic3D;


typedef union __das_VertDynamic2D {
    struct {
        das_PosData2D *pos;
        uint64_t pn;
        das_TextureData *tex;
        uint64_t tn;
        uint32_t hier;
    } mul;

    struct {
        das_GL2DVertex *vert;
        das_GL2DVertexUnmapped *uvert;
        uint64_t n;
    } mer;
} __das_VertDynamic2D;


/// Universal dynamic vertices container union for both 3D and 2D assets
typedef union das_VertDynamic {
    __das_VertDynamic2D v2d;
    __das_VertDynamic3D v3d;
} das_VertDynamic;


/// Structure for universal heap allocated indices data storage
typedef struct das_IndicesDynamic {
    uint32_t *pos;
    uint32_t *tex;
    uint32_t *norm;
    uint32_t *gl;
    uint64_t n;
} das_IndicesDynamic;


/// Structure for universal texture image bitmap data storage
typedef struct das_PixelDataDynamic {
    uint8_t *pixel_data;
    uint64_t size;
    uint16_t width;
    uint16_t height;
    uint64_t memory_offset;
} das_PixelDataDynamic;


/// Data structure for containing information about mesh triangle
typedef struct das_MeshTriangle {
    das_PosData *pos[3];
    das_TextureData *tex[3];
    das_NormalData *nor[3];
    struct das_MeshTriangle *nbr[3];
    uint16_t nnbr; // Next neighbour to set
} das_MeshTriangle;


/// Data structure for keeping information about edges
typedef struct das_Edge {
    das_PosData *pos[2];
    das_MeshTriangle *adj_tri[2];
} das_Edge;


/// Data structure for containing information about mesh
typedef struct das_Mesh {
    das_PosData *pos;
    uint64_t pn;

    uint32_t *verti[3]; // vertex indicies
    uint32_t *nbri[3]; // neighbouring triangle indices
    uint32_t *vert_trii[3]; // index of any adjacent triangle
} das_Mesh;


/// Specify the type of the asset 
typedef enum das_AssetMode {
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
} das_AssetMode;


/// Main asset structure for 2D and 3D assets
typedef struct das_Asset {
    id_t uuid;
    id_t *tex_uuid;           // Texture id that the current asset is bound to (can be set to zero, if unmapped)
    id_t *vk_id;              // Vulkan asset id, that is generated when Vulkan is used as the backend api
    char src[FNAME_LEN];                 // Asset name that is displayed in editors 
    char *meta;

    // Temporary variables, since no material system is present
    das_ColorData diffuse;   // Asset diffuse color property
    das_ColorData ambient;   // Asset ambient light intensity
    das_ColorData specular;  // Specular light intensity
    float phong_exp;       // Phong exponent that is used, with Blinn-Phong shading
    

    das_AssetMode asset_mode;       // Asset mode specifier
    bool is_shown;           // False if asset is ignored from draw calls, otherwise True
    bool ignore_transform;   // Toggle transformation on and off
    bool is_opengl;          // Flag to determine if the asset is rendered in OpenGL backend renderer 
    bool force_unmap;        // Force the asset not to use texture image and texture mapping
    das_VertDynamic vertices;       // All asset vertices that will be passed to command buffers
    das_IndicesDynamic indices;     // All asset indices that will be passed to command buffers
    das_OffsetData offsets;         // Offsets, which are taken into account, when allocating buffer storage
} das_Asset;


/// DENG texture struct 
typedef struct das_Texture {
    id_t uuid;
    char pad[UUID_LEN];
    id_t vk_id;
    id_t gl_id;
    char src[FNAME_LEN];
    bool no_reg_cleanup;         // Set this as true if no automatic cleanup is wanted in registry destruction
    das_PixelDataDynamic pixel_data;
} das_Texture;

#endif
