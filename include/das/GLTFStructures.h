// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: GLTFStructurs.h - GLTF parsing functions and structures header
// author: Karl-Mihkel Ott

/**
 * For more information about objects in GLTF file format look:
 *  - https://www.khronos.org/registry/glTF/specs/2.0/glTF-2.0.html
 */

#ifndef GLTF_STRUCTURES_H
#define GLTF_STRUCTURES_H


// texture magnification and minification filter properties
#define KHRONOS_NEAREST                 9728
#define KHRONOS_LINEAR                  9729
#define KHRONOS_NEAREST_MIPMAP_NEAREST  9984
#define KHRONOS_LINEAR_MIPMAP_NEAREST   9985
#define KHRONOS_NEAREST_MIPMAP_LINEAR   9986
#define KHRONOS_LINEAR_MIPMAP_LINEAR    9987

// texture wrapping mode properties 
#define KHRONOS_CLAMP_TO_EDGE           33071
#define KHRONOS_MIRRORED_REPEAT         33648
#define KHRONOS_REPEAT                  10497

// buffer view target properties
#define KHRONOS_ARRAY_BUFFER            34962
#define KHRONOS_ELEMENT_ARRAY_BUFFER    34963

// scalar types
#define KHRONOS_BYTE                    5120
#define KHRONOS_UNSIGNED_BYTE           5121
#define KHRONOS_SHORT                   5122
#define KHRONOS_UNSIGNED_SHORT          5123
#define KHRONOS_UNSIGNED_INT            5125
#define KHRONOS_FLOAT                   5126

// primitive types
#define KHRONOS_POINTS                  0
#define KHRONOS_LINES                   1
#define KHRONOS_LINE_LOOP               2
#define KHRONOS_LINE_STRIP              3
#define KHRONOS_TRIANGLES               4
#define KHRONOS_TRIANGLE_STRIP          5
#define KHRONOS_TRIANGLE_FAN            6


namespace Libdas {

    ///////////////////////////////////////////
    // **** Accessor object structures ***** //
    ///////////////////////////////////////////
    

    /**
     * Structure pointing to a buffer view containing the values of deviating accessor values
     */
    struct GLTFAccessorSparseValues {
        int32_t buffer_view = 0;                        // required
        uint32_t byte_offset = 0;                       // not required (default: 0)
        std::vector<std::any> extensions;               // ignored
        std::vector<std::any> extras;                   // ignored
    };


    /**
     * Structure pointing to a buffer view containing the indices of deviating accessor values
     */
    struct GLTFAccessorSparseIndices {
        int32_t buffer_view = 0;                        // required
        uint32_t byte_offset = 0;                       // not required (default: 0)
        int32_t component_type = INT32_MAX;             // required
        std::vector<std::any> extension;                // ignored
        std::vector<std::any> extras;                   // ignored
    };


    /**
     * Sparse storage of accessor values that deviate from their initialisation values
     */
    struct GLTFAccessorSparse {
        int32_t count = 0;                              // required
        GLTFAccessorSparseIndices indices;              // required
        GLTFAccessorSparseValues values;                // required
        std::vector<std::any> extensions;               // ignored
        std::vector<std::any> extras;                   // ignored
    };


    /**
     * Buffer accessor object that contains a typed view into a buffer view, where raw data is stored
     */
    struct GLTFAccessor {
        int32_t buffer_view = INT32_MAX;                // not required
        uint32_t byte_offset = 0;                       // not required (default: 0)
        int32_t component_type = INT32_MAX;             // required
        bool normalized = false;                        // not required (default: false)
        int32_t count = 0;                              // required
        std::string type;                               // required
        std::vector<float> max;                         // not required
        std::vector<float> min;                         // not required
        GLTFAccessorSparse sparse;                      // not required
        std::string name = "";                          // not required
        std::vector<std::any> extensions;               // ignored
        std::vector<std::any> extras;                   // ignored

        // custom properties not present in official specification
        uint32_t accumulated_offset = 0;
        uint32_t buffer_id = UINT32_MAX;
    };


    /////////////////////////////////////////////
    // ***** Animation object structures ***** //
    /////////////////////////////////////////////
    
    /** 
     * Structure combining animation timestamps with a sequence of output values and
     * defining the interpolation algorithm
     */
    struct GLTFAnimationSampler {
        int32_t input = INT32_MAX;                      // required
        std::string interpolation = "LINEAR";           // not required (default: "LINEAR")
        int32_t output = INT32_MAX;                     // required
        std::vector<std::any> extensions;               // ignored
        std::vector<std::any> extras;                   // ignored
    };


    /**
     * Structure that describes the animated property
     */
    struct GLTFAnimationChannelTarget {
        int32_t node = INT32_MAX;                       // not required
        std::string path;                               // required
        std::vector<std::any> extensions;               // ignored
        std::vector<std::any> extras;                   // ignored
    };

    
    /**
     * Structure tha combines an animation sampler with a target property that is being animated
     */
    struct GLTFAnimationChannel {
        int32_t sampler = INT32_MAX;                    // required
        GLTFAnimationChannelTarget target;              // required
        std::vector<std::any> extensions;               // ignored
        std::vector<std::any> extras;                   // ignored
    };


    /**
     * Structure containing information about keyframe animation
     */
    struct GLTFAnimation {
        std::vector<GLTFAnimationChannel> channels;      // required
        std::vector<GLTFAnimationSampler> samplers;     // required
        std::string name;                               // not required
        std::vector<std::any> extensions;               // ignored
        std::vector<std::any> extras;                   // ignored
    };


    /////////////////////////////////////////
    // ***** Single depth structures ***** //
    /////////////////////////////////////////
    
    /**
     * Structure containing metadata about the glTF asset
     */
    struct GLTFAsset {
        std::string copyright;                          // not required
        std::string generator;                          // not required
        std::string version;                            // required
        std::string min_version;                        // not required
        std::vector<std::any> extensions;               // ignored
        std::vector<std::any> extras;                   // ignored
    };


    /**
     * Structure containing a reference to binary goemetry animation or skins
     */
    struct GLTFBuffer {
        std::string uri;                                // not required
        uint32_t byte_length = 0;                       // required
        std::string name;                               // not required
        std::vector<std::any> extensions;               // ignored
        std::vector<std::any> extras;                   // ignored
    };

    
    /**
     * Structure containing a view into a subset of buffer
     */
    struct GLTFBufferView {
        int32_t buffer = INT32_MAX;                     // required
        uint32_t byte_offset = 0;                       // not required (default: 0)
        uint32_t byte_length = 0;                       // required
        uint32_t byte_stride = 0;                       // not required
        uint32_t target = 0;                            // not required
        std::string name;                               // not required
        std::vector<std::any> extensions;               // ignored
        std::vector<std::any> extras;                   // ignored
    };


    /**
     * Structure containing image data, used for textures
     */
    struct GLTFImage {
        std::string uri;                                // required if no buffer_view is specified
        std::string mime_type;                          // not required
        int32_t buffer_view = INT32_MAX;                // required if no uri is specified
        std::string name;                               // not required
        std::vector<std::any> extensions;               // ignored
        std::vector<std::any> extras;                   // ignored
    };


    /**
     * Universal node structure that can be used in any node hieararchy in GLTF
     */
    struct GLTFNode {
        int32_t camera = INT32_MAX;                     // not required
        std::vector<int32_t> children;                  // not required
        int32_t skin = INT32_MAX;                       // not required
        TRS::Matrix4<float> matrix;                          // not required (default: 1, 0, 0, 0 | 0, 1, 0, 0 | 0, 0, 1, 0 | 0, 0, 0, 1)
        int32_t mesh = INT32_MAX;                       // not required
        // NOTE: quaternions must be implemented sometime
        TRS::Quaternion rotation;                            // not required (default: 0, 0, 0, 1)
        TRS::Point3D<float> scale = {1.0f, 1.0f, 1.0f};      // not required (default: 1, 1, 1)
        TRS::Point3D<float> translation = {0.0f, 0.0f, 0.0f};// not required (default: 0, 0, 0)
        std::vector<float> weights;                     // not required
        std::string name = "";                          // not required
        std::vector<std::any> extensions;               // ignored
        std::vector<std::any> extras;                   // ignored
    };

    
    /**
     * Texture sampler properties for filtering and wrapping models
     */
    struct GLTFSampler {
        uint32_t mag_filter = UINT32_MAX;               // not required
        uint32_t min_filter = UINT32_MAX;               // not required
        uint32_t wrap_s = KHRONOS_REPEAT;               // not required (default: 10497)
        uint32_t wrap_t = KHRONOS_REPEAT;               // not required (default: 10497)
        std::string name = "";                          // not required
        std::vector<std::any> extensions;               // ignored
        std::vector<std::any> extras;                   // ignored
    };


    /**
     * Scene structure for GLTF format
     */
    struct GLTFScene {
        std::vector<int32_t> nodes;                     // not required
        std::string name = "";                          // not required
        std::vector<std::any> extensions;               // ignored
        std::vector<std::any> extras;                   // ignored
    };


    /**
     * Structure containing joints and matrices defining a skin
     */
    struct GLTFSkin {
        int32_t inverse_bind_matrices = INT32_MAX;      // not required
        int32_t skeleton = INT32_MAX;                   // not required
        std::vector<int32_t> joints;                    // required
        std::string name = "";                          // not required
        std::vector<std::any> extensions;               // ignored
        std::vector<std::any> extras;                   // ignored
    };


    /**
     * Structure containing GLTF texture with its sampler
     */
    struct GLTFTexture {
        int32_t sampler = INT32_MAX;                    // not required
        int32_t source = INT32_MAX;                     // not required
        std::string name = "";                          // not required
        std::vector<std::any> extensions;               // ignored
        std::vector<std::any> extras;                   // ignored
    };


    /**
     * Structure containing a reference to a texture
     */
    struct GLTFTextureInfo {
        int32_t index = INT32_MAX;                      // required
        int32_t tex_coord = 0;                          // not required (default: 0)
        std::vector<std::any> extensions;               // ignored
        std::vector<std::any> extras;                   // ignored
    };


    ////////////////////////////////
    // ***** Camera objects ***** //
    ////////////////////////////////

    /**
     * Structure containing orthographic camera's properties
     */
    struct GLTFCameraOrthographic {
        uint32_t xmag = 0;                              // required
        uint32_t ymag = 0;                              // required
        float zfar = 0.0f;                              // required
        float znear = 0.0f;                             // required
        std::vector<std::any> extensions;               // ignored
        std::vector<std::any> extras;                   // ignored
    };


    /**
     * Structure containing perspective camera's properties
     */
    struct GLTFCameraPerspective {
        float aspect_ratio = 16.0f / 9.0f;              // not required
        float yfov = 0.0f;                              // required
        float zfar = FLT_MAX;                           // not required
        float znear = FLT_MIN;                          // required
        std::vector<std::any> extensions;               // ignored
        std::vector<std::any> extras;                   // ignored
    };


    /**
     * Structure containing camera's projection 
     */
    struct GLTFCamera {
        GLTFCameraOrthographic orthographic;            // not required
        GLTFCameraPerspective perspective;              // not required
        std::string type;                               // required
        std::string name;                               // required
        std::vector<std::any> extensions;               // ignored
        std::vector<std::any> extras;                   // ignored
    };


    //////////////////////////////////
    // ***** Material objects ***** //
    //////////////////////////////////
    
    /**
     * Reference to a normal material texture 
     */
    struct GLTFNormalTextureInfo {
        int32_t index = INT32_MAX;                      // required
        int32_t tex_coord = 0;                          // not required (default: 0)
        float scale = 1.0f;                             // not required (default: 1)
        std::vector<std::any> extensions;               // ignored
        std::vector<std::any> extras;                   // ignored
    };


    /**
     * Reference to a occlusion material texture 
     */
    struct GLTFOcclusionTextureInfo {
        int32_t index = INT32_MAX;                      // required
        int32_t tex_coord = 0;                          // not required (default: 0)
        float strength = 1.0f;                          // not required (default: 1)
        std::vector<std::any> extensions;               // ignored
        std::vector<std::any> extras;                   // ignored
    };


    /**
     * A structure with a set of parameters that are used to define the metallic-roughness material model from
     * Physically-Based Rendering methology
     */
    struct GLTFpbrMetallicRoughness {
        TRS::Point4D<float> base_color_factor = {1, 1, 1, 1};// not required (default: 1, 1, 1, 1)
        GLTFTextureInfo base_color_texture;             // not required
        float metallic_factor = 1.0f;                   // not required (default: 1.0f)
        float roughness_factor = 1.0f;                  // not required (default: 1.0f)
        GLTFTextureInfo metallic_roughness_texture;     // not required
        std::vector<std::any> extensions;               // ignored
        std::vector<std::any> extras;                   // ignored
    };

    
    /**
     * Material structure for primitives
     */
    struct GLTFMaterial {
        std::string name = "";                          // not required
        std::vector<std::any> extensions;               // ignored
        std::vector<std::any> extras;                   // ignored
        GLTFpbrMetallicRoughness pbr_metallic_roughness;// not required
        GLTFNormalTextureInfo normal_texture;           // not required
        GLTFOcclusionTextureInfo occlusion_texture;     // not required
        GLTFTextureInfo emissive_texture;               // not required
        TRS::Point3D<float> emissive_factor = {0, 0, 0};     // not required (default: [0, 0, 0])
        std::string alpha_mode = "OPAQUE";              // not required (default: "OPAQUE")
        float alpha_cutoff = 0.5f;                      // not required (default: 0.5f)
        bool double_sided = false;                      // not required (default: false)
    };


    //////////////////////////////
    // ***** Mesh objects ***** //
    //////////////////////////////
    
    
    /**
     * Structure containing geometry to be rendered with given material
     */
    struct GLTFMeshPrimitive {
        typedef std::vector<std::pair<std::string, uint32_t>> AttributesType;
        AttributesType attributes;                      // required
        int32_t indices = INT32_MAX;                    // not required
        int32_t material = INT32_MAX;                   // not required
        uint32_t mode = KHRONOS_TRIANGLES;              // not required (default: 4) 
        std::vector<AttributesType> targets;            // not required
        std::vector<std::any> extensions;               // ignored
        std::vector<std::any> extras;                   // ignored
    };


    /**
     * Structure containing a set of primitives to be rendered
     */
    struct GLTFMesh {
        std::vector<GLTFMeshPrimitive> primitives;      // required
        std::vector<float> weights;                     // not required
        std::string name = "";                          // not required
        std::vector<std::any> extensions;               // ignored
        std::vector<std::any> extras;                   // ignored
    };


    /////////////////////////////
    // ***** Root object ***** //
    /////////////////////////////


    /**
     * Root object for all GLTF objects
     */
    struct GLTFRoot {
        std::vector<std::string> extensions_used;                           // not required
        std::vector<std::string> extensions_required;                       // not required
        std::vector<GLTFAccessor> accessors;                                // not required
        std::vector<GLTFAnimation> animations;                              // not required
        GLTFAsset asset;                                                    // required
        std::vector<GLTFBuffer> buffers;                                    // not required
        std::vector<GLTFBufferView> buffer_views;                           // not required
        std::vector<GLTFCamera> cameras;                                    // not required
        std::vector<GLTFImage> images;                                      // not required
        std::vector<GLTFMaterial> materials;                                // not required
        std::vector<GLTFMesh> meshes;                                       // not required
        std::vector<GLTFNode> nodes;                                        // not required
        std::vector<GLTFSampler> samplers;                                  // not required
        std::vector<GLTFScene> scenes;                                      // not required
        int32_t load_time_scene = INT32_MAX;                                // not required
        std::vector<GLTFSkin> skins;                                        // not required
        std::vector<GLTFTexture> textures;                                  // not required
        std::vector<std::any> extensions;                                   // ignored
        std::vector<std::any> extras;                                       // ignored
    };
}

#endif
