/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: GLTFParser.h - GLTF format parsing class header
/// author: Karl-Mihkel Ott

#ifndef GLTF_PARSER_H
#define GLTF_PARSER_H


#ifdef GLTF_PARSER_CPP
    #include <any>
    #include <set>
    #include <variant>
    #include <map>
    #include <memory>
    #include <fstream>
    #include <iostream>
    #include <string>
    #include <cstring>
    #include <unordered_map>
    #include <vector>
    #include <cfloat>

    #include <Api.h>
    #include <Iterators.h>
    #include <Points.h>
    #include <Vector.h>
    #include <Matrix.h>
    #include <FileNameString.h>
    #include <LibdasAssert.h>
    #include <ErrorHandlers.h>
#define LIBDAS_DEFS_ONLY
    #include <HuffmanCompression.h>
#undef LIBDAS_DEFS_ONLY
    #include <AsciiStreamReader.h>
    #include <JSONParser.h>
    #include <GLTFStructures.h>
    #include <Base64Decoder.h>
    #include <URIResolver.h>
#endif


namespace Libdas {

    /**
     * Enumeral for easy GLTF object type identification with map
     */
    enum GLTFObjectType {
        GLTF_OBJECT_ACCESSORS,
        GLTF_OBJECT_ACCESSOR_SPARSE,
        GLTF_OBJECT_ACCESSOR_SPARSE_INDICES,
        GLTF_OBJECT_ACCESSOR_SPARSE_VALUES,
        GLTF_OBJECT_ANIMATIONS,
        GLTF_OBJECT_ANIMATION_CHANNEL,
        GLTF_OBJECT_ANIMATION_CHANNEL_TARGET,
        GLTF_OBJECT_ANIMATION_SAMPLER,
        GLTF_OBJECT_ANIMATION_SAMPLERS,
        GLTF_OBJECT_ASSET,
        GLTF_OBJECT_BUFFERS,
        GLTF_OBJECT_BUFFERVIEWS,
        GLTF_OBJECT_CAMERAS,
        GLTF_OBJECT_CAMERA_ORTHOGRAPHIC,
        GLTF_OBJECT_CAMERA_PERSPECTIVE,
        GLTF_OBJECT_EXTENSIONS,
        GLTF_OBJECT_EXTRAS,
        GLTF_OBJECT_IMAGES,
        GLTF_OBJECT_MATERIALS,
        GLTF_OBJECT_MATERIAL_NORMAL_TEXTURE_INFO,
        GLTF_OBJECT_MATERIAL_OCCLUSION_TEXTURE_INFO,
        GLTF_OBJECT_MATERIAL_PBR_METALLIC_ROUGHNESS,
        GLTF_OBJECT_MESHES,
        GLTF_OBJECT_MESH_PRIMITIVES,
        GLTF_OBJECT_NODES,
        GLTF_OBJECT_SAMPLERS,
        GLTF_OBJECT_SCENE,
        GLTF_OBJECT_SCENES,
        GLTF_OBJECT_SKINS,
        GLTF_OBJECT_TEXTURES,
        GLTF_OBJECT_TEXTURE_INFO,
        GLTF_OBJECT_ROOT
    };


    /**
     * Enumeral containing all possible GLTF object value types
     * This differs somewhat from JSON data types, since GLTF has arrays that
     * contain only single types of data and custom subnode types as well
     */
    enum GLTFType {
        GLTF_TYPE_STRING,
        GLTF_TYPE_INTEGER,
        GLTF_TYPE_FLOAT,
        GLTF_TYPE_BOOLEAN,
        GLTF_TYPE_STRING_ARRAY,
        GLTF_TYPE_INTEGER_ARRAY,
        GLTF_TYPE_FLOAT_ARRAY,

        /////////////////////////////////////////////////////////////////////////////
        // ***** This is where all GLTF specific data types are written into ***** //
        /////////////////////////////////////////////////////////////////////////////
        
        // accessors.sparse
        GLTF_TYPE_ACCESSOR_SPARSE,
        GLTF_TYPE_ACCESSOR_SPARSE_INDICES,
        GLTF_TYPE_ACCESSOR_SPARSE_VALUES,

        // animation.*
        GLTF_TYPE_ANIMATION_CHANNELS,       // animation.channels must be an array
        GLTF_TYPE_ANIMATION_CHANNEL_TARGET,
        GLTF_TYPE_ANIMATION_SAMPLERS,       // animation.samplers must be an array

        // camera.*
        GLTF_TYPE_CAMERA_ORTHOGRAPHIC,
        GLTF_TYPE_CAMERA_PERSPECTIVE,

        // material.*
        GLTF_TYPE_MATERIAL_PBR_METALLIC_ROUGHNESS,
        GLTF_TYPE_MATERIAL_NORMAL_TEXTURE,
        GLTF_TYPE_MATERIAL_OCCLUSION_TEXTURE,
        GLTF_TYPE_MATERIAL_TEXTURE_INFO,

        // mesh.primitive*
        GLTF_TYPE_MESH_PRIMITIVES,
        GLTF_TYPE_MESH_PRIMITIVE_ATTRIBUTES,            // some unspecified JSON object


        // extensions and extras
        GLTF_TYPE_EXTRAS_OR_EXTENSIONS,
    };


    /**
     * Universal structure containing pointer to scope and type declaration
     */
    struct GLTFUniversalScopeValue {
        void *val_ptr;       // this structure is meant to be used for value pointers only
        GLTFType type;          // type specifier for more elegant data casting
    };

    
    /**
     * GLTF file format parser class
     */
    class LIBDAS_API GLTFParser : public JSONParser {
        private:
            std::ifstream m_ext_reader;
            AsciiFormatErrorHandler m_error;
            GLTFRoot m_root;
            std::unordered_map<std::string, GLTFObjectType> m_root_objects;

        private:
            /**
             * Create correct key value map for identifying the root object type
             */
            void _InitialiseRootObjectTypeMap();
            /**
             * Find the object type from string with correct error handling
             * @param _obj_str specifies the key string used for certain object
             * @param _line specifies the JSON line, where the key was read
             */
            GLTFObjectType _FindRootObjectType(const std::string &_obj_str, uint32_t _line);
            /**
             * Verify source data that is submitted to _CopyJSONDataToGLTFRoot according to the 
             * required destination type
             * @param _node is a reference to JSONNode that specifies the node whose values would be verified
             * @param _supported_type specifies the supported source type 
             * @param _is_array specifies if the given destination format supports array type
             */
            void _VerifySourceData(JSONNode *_node, JSONType _supported_type, bool _is_array);
            /**
             * Helper method to add data into correct data structure according to specified type,
             * @param _src specifies a valid pointer to JSONNode instance, where parsed values are stored
             * @param _dst specifies a reference to universal scope structure, where all data 
             * should be copied to
             * @param _root optionally specifies a reference to std::any object that can contain the destination
             * root object
             */
            void _CopyJSONDataToGLTFRoot(JSONNode *_src, GLTFUniversalScopeValue &_dst);
            /**
             * Helper method to iterate through all sub nodes of a node
             * @param _node specifies the pointer to a node use for iteratation
             * @param _val_map specifies the value map, where all string values are stored as keys and
             * values represent GLTFUniversalScopeValue object that specifies the value pointer with its type
             */
            void _IterateSubNodes(JSONNode *_node, std::unordered_map<std::string, GLTFUniversalScopeValue> &_val_map);
            /**
             * Helper method to iterate through value objects and and their sub nodes
             * @tparam T specifies the correct GLTF structure to use 
             * @param _node specifies the root node of the values that will be iterated
             * @param _val_map specifies key/value pairs where key is string key and value is a GLTFUniversalScopeValue instance, where 
             * correct data pointers are stored for parsing sub_nodes 
             * @param _dst_item specifies the destination item, where data is written to
             * @param _dst_vecto specifies a vector that points to some vector instace from the root object
             */
            template<typename T>
            void _IterateValueObjects(JSONNode *_node, std::unordered_map<std::string, GLTFUniversalScopeValue> &_val_map, T &_dst_item, std::vector<T> &_dst_vector) {
                // for each element in values
                for(size_t i = 0; i < _node->values.size(); i++) {
                    // error: invalid element type, only JSON objects are supported
                    if(_node->values[i].first != JSON_TYPE_OBJECT)
                        m_error.Error(LIBDAS_ERROR_INVALID_TYPE, _node->key_val_decl_line, _node->name, "JSON object");

                    _IterateSubNodes(std::any_cast<JSONNode>(&_node->values[i].second), _val_map);
                    _dst_vector.push_back(_dst_item);
                    _dst_item = T();
                }
            }
            /**
             * Create an object vector from JSONValues vector without checking the type
             * of each element
             * @param _val is a reference to JSONValues instance where original JSONData is 
             * stored
             */
            template<typename T>
            std::vector<T> _JsonValueToVectorCast(JSONValues &_val) {
                std::vector<T> out;
                out.reserve(_val.size());
                for(size_t i = 0; i < _val.size(); i++)
                    out.push_back(std::any_cast<T>(_val[i].second));

                return out;
            }
            /**
             * Create an object vector from non-numerical JSONValues vector without checking the type
             * of each element
             * @param _val is a reference to JSONValues instance, where original JSONData is stored
             */
            template <typename T>
            std::vector<T> _NumericalJsonValueToVectorCast(JSONValues &_val) {
                std::vector<T> out;
                out.reserve(_val.size());
                for(size_t i = 0; i < _val.size(); i++) {
                    if(!std::is_floating_point<T>::value)
                        out.push_back(_CastVariantNumber<T, JSONNumber>(std::any_cast<std::variant<JSONInteger, JSONNumber>>(_val[i].second)));
                    else out.push_back(_CastVariantNumber<T, JSONInteger>(std::any_cast<std::variant<JSONInteger, JSONNumber>>(_val[i].second)));
                }

                return out;
            }
            /**
             * Helper method to cast variant numbers into correct types
             * @tparam Primary specifies the destination type to cast into, supported: JSONInteger and JSONNumber
             * @tparam Secondary specifies the secondary type that can be used in variant as well (opposite of 
             * Primary in this regard)
             * @param _var_num is a const reference to std::variant<JSONInteger, JSONNumber> 
             * that specifies the variant type to be casted
             */
            template<typename Primary, typename Secondary>
            Primary _CastVariantNumber(const std::variant<JSONInteger, JSONNumber> &_var_num) {
                // attempt to cast directly
                try {
                    return std::get<Primary>(_var_num);
                } catch (const std::bad_variant_access& ex) {
                    return std::get<Secondary>(_var_num);
                }
            }

            /////////////////////////////////////////////
            // ***** Node object parsing methods ***** //
            /////////////////////////////////////////////
            
            void _ReadAccessors(JSONNode *_node);
            void _ReadAccessorSparse(JSONNode *_node, GLTFAccessorSparse &_root);
            void _ReadAccessorSparseIndices(JSONNode *_node, GLTFAccessorSparseIndices &_root);
            void _ReadAccessorSparseValues(JSONNode *_node, GLTFAccessorSparseValues &_root);
            void _ReadAnimations(JSONNode *_node);
            void _ReadAnimationChannels(JSONNode *_node, std::vector<GLTFAnimationChannel> &_root);
            void _ReadAnimationChannelTarget(JSONNode *_node, GLTFAnimationChannelTarget &_root);
            void _ReadAnimationSamplers(JSONNode *_node, std::vector<GLTFAnimationSampler> &_root);
            void _ReadAsset(JSONNode *_node);
            void _ReadBuffers(JSONNode *_node);
            void _ReadBufferviews(JSONNode *_node);
            void _ReadCameras(JSONNode *_node);
            void _ReadCameraOrthographic(JSONNode *_node, GLTFCameraOrthographic &_root);
            void _ReadCameraPerspective(JSONNode *_node, GLTFCameraPerspective &_root);
            void _ReadImages(JSONNode *_node);
            void _ReadMaterials(JSONNode *_node);
            void _ReadMaterialPbrMetallicRoughness(JSONNode *_node, GLTFpbrMetallicRoughness &_root);
            void _ReadMaterialNormalTexture(JSONNode *_node, GLTFNormalTextureInfo &_root);
            void _ReadMaterialOcclusionTexture(JSONNode *_node, GLTFOcclusionTextureInfo &_root);
            void _ReadMaterialTextureInfo(JSONNode *_node, GLTFTextureInfo &_root); // emissiveTexture
            void _ReadMeshes(JSONNode *_node);
            void _ReadMeshPrimitives(JSONNode *_node, std::vector<GLTFMeshPrimitive> &_root);
            void _ReadMeshPrimitiveAttributes(JSONNode *_node, GLTFMeshPrimitive::AttributesType &_attrs);
            void _ReadNodes(JSONNode *_node);
            void _ReadSamplers(JSONNode *_node);
            void _ReadScenes(JSONNode *_node, bool is_root);
            void _ReadSkins(JSONNode *_node);
            void _ReadTextures(JSONNode *_node);


            /**
             * Call appropriate parsing method for certain root object
             * @param _type is a GLTFObjectType value specifying the requested type that will be read 
             * @param _node is a valid pointer to JSONNode instance, that specifies the current subnode that will be parsed
             */
            void _RootObjectParserCaller(GLTFObjectType _type, JSONNode *_node);
            /**
             * Resolve all buffer uris and create appropriate map entries for them into GLTFRoot::resources
             * @param _file_name specifies the current gltf file name in order to extract root path from it
             */
            void _ResolveBufferUris(const std::string &_file_name);

        public:
            GLTFParser(const std::string &_file_name = "");
            /**
             * Parse GLTF file into appropriate structures
             * @param _file_name specifies the file name to read for parsing
             */
            void Parse(const std::string &_file_name = "");
            /**
             * Get the parsed root object 
             * @return reference to GLTFRoot structure
             */
            GLTFRoot &GetRootObject();
    };
}

#endif
