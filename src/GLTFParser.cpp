/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: GLTFParser.cpp - GLTF format parsing class implementation
/// author: Karl-Mihkel Ott

#define GLTF_PARSER_CPP
#include <GLTFParser.h>


namespace Libdas {

    GLTFParser::GLTFParser(const std::string &_file_name) : 
        JSONParser(MODEL_FORMAT_GLTF, _file_name), m_error(MODEL_FORMAT_GLTF) 
    {
        _InitialiseRootObjectTypeMap();
    }

    
    void GLTFParser::_InitialiseRootObjectTypeMap() {
        m_root_objects["accessors"] = GLTF_OBJECT_ACCESSORS;
        m_root_objects["animations"] = GLTF_OBJECT_ANIMATIONS;
        m_root_objects["asset"] = GLTF_OBJECT_ASSET;
        m_root_objects["buffers"] = GLTF_OBJECT_BUFFERS;
        m_root_objects["bufferViews"] = GLTF_OBJECT_BUFFERVIEWS;
        m_root_objects["cameras"] = GLTF_OBJECT_CAMERAS;
        m_root_objects["extensions"] = GLTF_OBJECT_EXTENSIONS;
        m_root_objects["extensionsUsed"] = GLTF_OBJECT_EXTENSIONS;
        m_root_objects["extensionsRequired"] = GLTF_OBJECT_EXTENSIONS;
        m_root_objects["extras"] = GLTF_OBJECT_EXTRAS;
        m_root_objects["images"] = GLTF_OBJECT_IMAGES;
        m_root_objects["materials"] = GLTF_OBJECT_MATERIALS;
        m_root_objects["meshes"] = GLTF_OBJECT_MESHES;
        m_root_objects["nodes"] = GLTF_OBJECT_NODES;
        m_root_objects["samplers"] = GLTF_OBJECT_SAMPLERS;
        m_root_objects["scene"] = GLTF_OBJECT_SCENE;
        m_root_objects["scenes"] = GLTF_OBJECT_SCENES;
        m_root_objects["skins"] = GLTF_OBJECT_SKINS;
        m_root_objects["textures"] = GLTF_OBJECT_TEXTURES;
    }


    GLTFObjectType GLTFParser::_FindRootObjectType(const std::string &_obj_str, uint32_t _line) {
        // object key not found
        if(m_root_objects.find(_obj_str) == m_root_objects.end())
            m_error.Error(LIBDAS_ERROR_INVALID_KEYWORD, _line, _obj_str);

        return m_root_objects[_obj_str];
    }


    void GLTFParser::_VerifySourceData(JSONNode *_node, JSONType _supported_type, bool _is_array) {
        // find an appropriate expected type string in case of error
        std::string exp_type_str;
        if(_supported_type & JSON_TYPE_STRING) {
            if(!_is_array)
                exp_type_str = "string";
            else exp_type_str = "string array";
        } else if (_supported_type & JSON_TYPE_BOOLEAN) {
            if(!_is_array)
                exp_type_str = "boolean";
            else exp_type_str = "boolean array";
        } else if (_supported_type & JSON_TYPE_INTEGER || _supported_type & JSON_TYPE_FLOAT) {
            if(!_is_array)
                exp_type_str = "number";
            else exp_type_str = "number array";
        } else if (_supported_type & JSON_TYPE_OBJECT) {
            if(!_is_array)
                exp_type_str = "object";
            else exp_type_str = "object array";
        }

        // throw an error if array type is not specified, but there are 
        // multiple values in values vector
        if(!_is_array && _node->values.size() > 1)
            m_error.Error(LIBDAS_ERROR_INVALID_TYPE, _node->key_val_decl_line, _node->name, exp_type_str);

        else {
            // check if array elements are homogenous and supported
            for(size_t i = 0; i < _node->values.size(); i++) {
                if(!(_node->values[i].first & _supported_type))
                    m_error.Error(LIBDAS_ERROR_INVALID_TYPE, _node->key_val_decl_line, _node->name, exp_type_str);
            }
        }
    }


    void GLTFParser::_CopyJSONDataToGLTFRoot(JSONNode *_src, GLTFUniversalScopeValue &_dst) {
        switch(_dst.type) {
            case GLTF_TYPE_STRING:
                _VerifySourceData(_src, JSON_TYPE_STRING, false);
                *reinterpret_cast<JSONString*>(_dst.val_ptr) = std::any_cast<JSONString>(_src->values.back().second);
                break;

            case GLTF_TYPE_INTEGER: {
                _VerifySourceData(_src, JSON_TYPE_INTEGER | JSON_TYPE_FLOAT, false);
                *reinterpret_cast<JSONInteger*>(_dst.val_ptr) = _CastVariantNumber<JSONInteger, JSONNumber>(
                    std::any_cast<std::variant<JSONInteger, JSONNumber>>(_src->values.back().second));
                break;
            }

            case GLTF_TYPE_FLOAT:
                _VerifySourceData(_src, JSON_TYPE_FLOAT | JSON_TYPE_FLOAT, false);
                *reinterpret_cast<JSONNumber*>(_dst.val_ptr) = _CastVariantNumber<JSONNumber, JSONInteger>(
                    std::any_cast<std::variant<JSONInteger, JSONNumber>>(_src->values.back().second));
                break;

            case GLTF_TYPE_INTEGER_ARRAY:
                _VerifySourceData(_src, JSON_TYPE_INTEGER | JSON_TYPE_FLOAT, true);
                *reinterpret_cast<std::vector<JSONInteger>*>(_dst.val_ptr) = _NumericalJsonValueToVectorCast<JSONInteger>(_src->values);
                break;

            case GLTF_TYPE_FLOAT_ARRAY:
                _VerifySourceData(_src, JSON_TYPE_FLOAT | JSON_TYPE_INTEGER, true);
                *reinterpret_cast<std::vector<JSONNumber>*>(_dst.val_ptr) = _NumericalJsonValueToVectorCast<JSONNumber>(_src->values);
                break;

            case GLTF_TYPE_STRING_ARRAY:
                _VerifySourceData(_src, JSON_TYPE_STRING, true);
                *reinterpret_cast<std::vector<JSONString>*>(_dst.val_ptr) = _JsonValueToVectorCast<JSONString>(_src->values);
                break;

            case GLTF_TYPE_ACCESSOR_SPARSE:
                _VerifySourceData(_src, JSON_TYPE_OBJECT, false);
                _ReadAccessorSparse(std::any_cast<JSONNode>(&_src->values.back().second), *reinterpret_cast<GLTFAccessorSparse*>(_dst.val_ptr));
                break;

            case GLTF_TYPE_ACCESSOR_SPARSE_INDICES:
                _VerifySourceData(_src, JSON_TYPE_OBJECT, false);
                _ReadAccessorSparseIndices(std::any_cast<JSONNode>(&_src->values.back().second), *reinterpret_cast<GLTFAccessorSparseIndices*>(_dst.val_ptr));
                break;

            case GLTF_TYPE_ACCESSOR_SPARSE_VALUES:
                _VerifySourceData(_src, JSON_TYPE_OBJECT, false);
                _ReadAccessorSparseValues(std::any_cast<JSONNode>(&_src->values.back().second), *reinterpret_cast<GLTFAccessorSparseValues*>(_dst.val_ptr));
                break;

            case GLTF_TYPE_ANIMATION_CHANNELS:
                _VerifySourceData(_src, JSON_TYPE_OBJECT, true);
                _ReadAnimationChannels(_src, *reinterpret_cast<std::vector<GLTFAnimationChannel>*>(_dst.val_ptr));
                break;

            case GLTF_TYPE_ANIMATION_CHANNEL_TARGET:
                _VerifySourceData(_src, JSON_TYPE_OBJECT, false);
                _ReadAnimationChannelTarget(std::any_cast<JSONNode>(&_src->values.back().second), *reinterpret_cast<GLTFAnimationChannelTarget*>(_dst.val_ptr));
                break;

            case GLTF_TYPE_ANIMATION_SAMPLERS:
                _VerifySourceData(_src, JSON_TYPE_OBJECT, true);
                _ReadAnimationSamplers(_src, *reinterpret_cast<std::vector<GLTFAnimationSampler>*>(_dst.val_ptr));
                break;

            case GLTF_TYPE_CAMERA_ORTHOGRAPHIC:
                _VerifySourceData(_src, JSON_TYPE_OBJECT, false);
                _ReadCameraOrthographic(_src, *reinterpret_cast<GLTFCameraOrthographic*>(_dst.val_ptr));
                break;

            case GLTF_TYPE_CAMERA_PERSPECTIVE:
                _VerifySourceData(_src, JSON_TYPE_OBJECT, false);
                _ReadCameraPerspective(_src, *reinterpret_cast<GLTFCameraPerspective*>(_dst.val_ptr));
                break;

            case GLTF_TYPE_MATERIAL_PBR_METALLIC_ROUGHNESS:
                _VerifySourceData(_src, JSON_TYPE_OBJECT, false);
                _ReadMaterialPbrMetallicRoughness(_src, *reinterpret_cast<GLTFpbrMetallicRoughness*>(_dst.val_ptr));
                break;

            case GLTF_TYPE_MATERIAL_NORMAL_TEXTURE:
                _VerifySourceData(_src, JSON_TYPE_OBJECT, false);
                _ReadMaterialNormalTexture(_src, *reinterpret_cast<GLTFNormalTextureInfo*>(_dst.val_ptr));
                break;

            case GLTF_TYPE_MATERIAL_OCCLUSION_TEXTURE:
                _VerifySourceData(_src, JSON_TYPE_OBJECT, false);
                _ReadMaterialOcclusionTexture(_src, *reinterpret_cast<GLTFOcclusionTextureInfo*>(_dst.val_ptr));
                break;

            case GLTF_TYPE_MATERIAL_TEXTURE_INFO:
                _VerifySourceData(_src, JSON_TYPE_OBJECT, false);
                _ReadMaterialTextureInfo(_src, *reinterpret_cast<GLTFTextureInfo*>(_dst.val_ptr));
                break;

            case GLTF_TYPE_MESH_PRIMITIVES:
                _VerifySourceData(_src, JSON_TYPE_OBJECT, true);
                _ReadMeshPrimitives(_src, *reinterpret_cast<std::vector<GLTFMeshPrimitive>*>(_dst.val_ptr));
                break;

            default:
                break;
        }
    }


    void GLTFParser::_IterateSubNodes(JSONNode *_node, std::unordered_map<std::string, GLTFUniversalScopeValue> &_val_map) {
        // iterate through all subscopes
        for(auto it = _node->sub_nodes.begin(); it != _node->sub_nodes.end(); it++) {
            // error no subscope name found
            if(_val_map.find(it->first) == _val_map.end())
                m_error.Error(LIBDAS_ERROR_INVALID_KEYWORD, it->second->key_val_decl_line, it->first);

            _CopyJSONDataToGLTFRoot(it->second.get(), _val_map[it->first]);
        }
    }


    void GLTFParser::_ReadAccessors(JSONNode *_node) {
        GLTFAccessor accessor;
        // map with key and pointer of the element
        std::unordered_map<std::string, GLTFUniversalScopeValue> values = {
            std::make_pair("bufferView", GLTFUniversalScopeValue{ &accessor.buffer_view, GLTF_TYPE_INTEGER }),
            std::make_pair("byteOffset", GLTFUniversalScopeValue{ &accessor.byte_offset, GLTF_TYPE_INTEGER }),
            std::make_pair("componentType", GLTFUniversalScopeValue{ &accessor.component_type, GLTF_TYPE_INTEGER }),
            std::make_pair("normalized", GLTFUniversalScopeValue{ &accessor.normalized, GLTF_TYPE_BOOLEAN }),
            std::make_pair("count", GLTFUniversalScopeValue{ &accessor.count, GLTF_TYPE_INTEGER }),
            std::make_pair("type", GLTFUniversalScopeValue{ &accessor.type, GLTF_TYPE_STRING }),
            std::make_pair("max", GLTFUniversalScopeValue{ &accessor.max, GLTF_TYPE_FLOAT_ARRAY }),
            std::make_pair("min", GLTFUniversalScopeValue{ &accessor.min, GLTF_TYPE_FLOAT_ARRAY }),
            std::make_pair("name", GLTFUniversalScopeValue{ &accessor.name, GLTF_TYPE_STRING }),
            std::make_pair("extensions", GLTFUniversalScopeValue{ &accessor.extensions, GLTF_TYPE_EXTRAS_OR_EXTENSIONS }),
            std::make_pair("extras", GLTFUniversalScopeValue{ &accessor.extras, GLTF_TYPE_EXTRAS_OR_EXTENSIONS }),
            std::make_pair("sparse", GLTFUniversalScopeValue{ &accessor.sparse, GLTF_TYPE_ACCESSOR_SPARSE })
        };

        _IterateValueObjects<GLTFAccessor>(_node, values, accessor, m_root.accessors);
    }


    void GLTFParser::_ReadAccessorSparse(JSONNode *_node, GLTFAccessorSparse &_sparse) {
        std::unordered_map<std::string, GLTFUniversalScopeValue> values = {
            std::make_pair("count", GLTFUniversalScopeValue { &_sparse.count, GLTF_TYPE_INTEGER } ),
            std::make_pair("indices", GLTFUniversalScopeValue { &_sparse.indices, GLTF_TYPE_ACCESSOR_SPARSE_INDICES } ),
            std::make_pair("values", GLTFUniversalScopeValue { &_sparse.values, GLTF_TYPE_ACCESSOR_SPARSE_VALUES } ),
            std::make_pair("extensions", GLTFUniversalScopeValue { &_sparse.extensions, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } ),
            std::make_pair("extras", GLTFUniversalScopeValue { &_sparse.extras, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } )
        };

        _IterateSubNodes(_node, values);
    }


    void GLTFParser::_ReadAccessorSparseIndices(JSONNode *_node, GLTFAccessorSparseIndices &_indices) {
        std::unordered_map<std::string, GLTFUniversalScopeValue> values = {
            std::make_pair("bufferView", GLTFUniversalScopeValue { &_indices.buffer_view, GLTF_TYPE_INTEGER } ),
            std::make_pair("byteOffset", GLTFUniversalScopeValue { &_indices.byte_offset, GLTF_TYPE_INTEGER } ),
            std::make_pair("componentType", GLTFUniversalScopeValue { &_indices.component_type, GLTF_TYPE_INTEGER } ),
            std::make_pair("extensions", GLTFUniversalScopeValue { &_indices.extension, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } ),
            std::make_pair("extras", GLTFUniversalScopeValue { &_indices.extras, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } ),
        };

        _IterateSubNodes(_node, values);
    }


    void GLTFParser::_ReadAccessorSparseValues(JSONNode *_node, GLTFAccessorSparseValues &_values) {
        std::unordered_map<std::string, GLTFUniversalScopeValue> values = {
            std::make_pair("bufferView", GLTFUniversalScopeValue { &_values.buffer_view, GLTF_TYPE_INTEGER } ),
            std::make_pair("byteOffset", GLTFUniversalScopeValue { &_values.byte_offset, GLTF_TYPE_INTEGER } ),
            std::make_pair("extensions", GLTFUniversalScopeValue { &_values.extensions, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } ),
            std::make_pair("extras", GLTFUniversalScopeValue { &_values.extras, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } )
        };

        _IterateSubNodes(_node, values);
    }


    void GLTFParser::_ReadAnimations(JSONNode *_node) {
        GLTFAnimation animation;
        std::unordered_map<std::string, GLTFUniversalScopeValue> values = {
            std::make_pair("channels", GLTFUniversalScopeValue { &animation.channels, GLTF_TYPE_ANIMATION_CHANNELS } ),
            std::make_pair("samplers", GLTFUniversalScopeValue { &animation.samplers, GLTF_TYPE_ANIMATION_SAMPLERS } ),
            std::make_pair("name", GLTFUniversalScopeValue { &animation.name, GLTF_TYPE_STRING } ),
            std::make_pair("channels", GLTFUniversalScopeValue { &animation.extensions, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } ),
            std::make_pair("channels", GLTFUniversalScopeValue { &animation.extras, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } ),
        };

        _IterateValueObjects<GLTFAnimation>(_node, values, animation, m_root.animations);
    }


    void GLTFParser::_ReadAnimationChannels(JSONNode *_node, std::vector<GLTFAnimationChannel> &_root) {
        GLTFAnimationChannel channel;
        std::unordered_map<std::string, GLTFUniversalScopeValue> values = {
            std::make_pair("sampler", GLTFUniversalScopeValue { &channel.sampler, GLTF_TYPE_INTEGER } ),
            std::make_pair("target", GLTFUniversalScopeValue { &channel.target, GLTF_TYPE_ANIMATION_CHANNEL_TARGET } ),
            std::make_pair("extensions", GLTFUniversalScopeValue { &channel.extensions, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } ),
            std::make_pair("extras", GLTFUniversalScopeValue { &channel.extras, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } ),
        };

        _IterateValueObjects<GLTFAnimationChannel>(_node, values, channel, _root);
    }


    void GLTFParser::_ReadAnimationChannelTarget(JSONNode *_node, GLTFAnimationChannelTarget &_target) {
        std::unordered_map<std::string, GLTFUniversalScopeValue> values = {
            std::make_pair("node", GLTFUniversalScopeValue { &_target.node, GLTF_TYPE_INTEGER } ),
            std::make_pair("path", GLTFUniversalScopeValue { &_target.path, GLTF_TYPE_STRING } ),
            std::make_pair("extensions", GLTFUniversalScopeValue { &_target.extensions, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } ),
            std::make_pair("extras", GLTFUniversalScopeValue { &_target.extras, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } ),
        };

        _IterateSubNodes(_node, values);
    }


    void GLTFParser::_ReadAnimationSamplers(JSONNode *_node, std::vector<GLTFAnimationSampler> &_root) {
        GLTFAnimationSampler sampler;
        std::unordered_map<std::string, GLTFUniversalScopeValue> values = {
            std::make_pair("input", GLTFUniversalScopeValue { &sampler.input, GLTF_TYPE_INTEGER } ),
            std::make_pair("interpolation", GLTFUniversalScopeValue { &sampler.interpolation, GLTF_TYPE_STRING } ),
            std::make_pair("output", GLTFUniversalScopeValue { &sampler.output, GLTF_TYPE_INTEGER } ),
            std::make_pair("extensions", GLTFUniversalScopeValue { &sampler.extensions, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } ),
            std::make_pair("extras", GLTFUniversalScopeValue { &sampler.extras, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } ),
        };

        _IterateValueObjects<GLTFAnimationSampler>(_node, values, sampler, _root);
    }


    void GLTFParser::_ReadAsset(JSONNode *_node) {
        std::unordered_map<std::string, GLTFUniversalScopeValue> values = {
            std::make_pair("copyright", GLTFUniversalScopeValue { &m_root.asset.copyright, GLTF_TYPE_STRING } ),
            std::make_pair("generator", GLTFUniversalScopeValue { &m_root.asset.generator, GLTF_TYPE_STRING } ),
            std::make_pair("version", GLTFUniversalScopeValue { &m_root.asset.version, GLTF_TYPE_STRING } ),
            std::make_pair("minVersion", GLTFUniversalScopeValue { &m_root.asset.min_version, GLTF_TYPE_STRING } ),
            std::make_pair("extensions", GLTFUniversalScopeValue { &m_root.asset.extensions, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } ),
            std::make_pair("extras", GLTFUniversalScopeValue { &m_root.asset.extras, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } ),
        };

        _IterateSubNodes(_node, values);
    }


    void GLTFParser::_ReadBuffers(JSONNode *_node) {
        GLTFBuffer buffer;
        std::unordered_map<std::string, GLTFUniversalScopeValue> values = {
            std::make_pair("uri", GLTFUniversalScopeValue { &buffer.uri, GLTF_TYPE_STRING } ),
            std::make_pair("byteLength", GLTFUniversalScopeValue { &buffer.byte_length, GLTF_TYPE_INTEGER } ),
            std::make_pair("name", GLTFUniversalScopeValue { &buffer.name, GLTF_TYPE_STRING } ),
            std::make_pair("extensions", GLTFUniversalScopeValue { &buffer.extensions, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } ),
            std::make_pair("extras", GLTFUniversalScopeValue { &buffer.extras, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } ),
        };

        _IterateValueObjects<GLTFBuffer>(_node, values, buffer, m_root.buffers);
    }


    void GLTFParser::_ReadBufferviews(JSONNode *_node) {
        GLTFBufferView buffer_view;
        std::unordered_map<std::string, GLTFUniversalScopeValue> values = {
            std::make_pair("buffer", GLTFUniversalScopeValue { &buffer_view.buffer, GLTF_TYPE_INTEGER } ),
            std::make_pair("byteOffset", GLTFUniversalScopeValue { &buffer_view.byte_offset, GLTF_TYPE_INTEGER } ),
            std::make_pair("byteLength", GLTFUniversalScopeValue { &buffer_view.byte_length, GLTF_TYPE_INTEGER } ),
            std::make_pair("byteStride", GLTFUniversalScopeValue { &buffer_view.byte_stride, GLTF_TYPE_INTEGER } ),
            std::make_pair("target", GLTFUniversalScopeValue { &buffer_view.target, GLTF_TYPE_INTEGER } ),
            std::make_pair("name", GLTFUniversalScopeValue { &buffer_view.name, GLTF_TYPE_STRING } ),
            std::make_pair("extensions", GLTFUniversalScopeValue { &buffer_view.extensions, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } ),
            std::make_pair("extras", GLTFUniversalScopeValue { &buffer_view.extras, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } )
        };

        _IterateValueObjects<GLTFBufferView>(_node, values, buffer_view, m_root.buffer_views);
    }


    void GLTFParser::_ReadCameras(JSONNode *_node) {
        GLTFCamera camera;
        std::unordered_map<std::string, GLTFUniversalScopeValue> values = {
            std::make_pair("orthographic", GLTFUniversalScopeValue { &camera.orthographic, GLTF_TYPE_CAMERA_ORTHOGRAPHIC } ),
            std::make_pair("perspective", GLTFUniversalScopeValue { &camera.perspective, GLTF_TYPE_CAMERA_PERSPECTIVE } ),
            std::make_pair("type", GLTFUniversalScopeValue { &camera.type, GLTF_TYPE_STRING } ),
            std::make_pair("name", GLTFUniversalScopeValue { &camera.name, GLTF_TYPE_STRING } ),
            std::make_pair("extensions", GLTFUniversalScopeValue { &camera.extensions, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } ),
            std::make_pair("extras", GLTFUniversalScopeValue { &camera.orthographic, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } ),
        };

        _IterateValueObjects<GLTFCamera>(_node, values, camera, m_root.cameras);
    }


    void GLTFParser::_ReadCameraOrthographic(JSONNode *_node, GLTFCameraOrthographic &_orthographic) {
        std::unordered_map<std::string, GLTFUniversalScopeValue> values = {
            std::make_pair("xmag", GLTFUniversalScopeValue { &_orthographic.xmag, GLTF_TYPE_FLOAT } ),
            std::make_pair("ymag", GLTFUniversalScopeValue { &_orthographic.ymag, GLTF_TYPE_FLOAT } ),
            std::make_pair("zfar", GLTFUniversalScopeValue { &_orthographic.zfar, GLTF_TYPE_FLOAT } ),
            std::make_pair("znear", GLTFUniversalScopeValue { &_orthographic.znear, GLTF_TYPE_FLOAT } ),
            std::make_pair("extensions", GLTFUniversalScopeValue { &_orthographic.extensions, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } ),
            std::make_pair("extras", GLTFUniversalScopeValue { &_orthographic.extras, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } ),
        };

        _IterateSubNodes(_node, values);
    }


    void GLTFParser::_ReadCameraPerspective(JSONNode *_node, GLTFCameraPerspective &_perspective) {
        std::unordered_map<std::string, GLTFUniversalScopeValue> values = {
            std::make_pair("aspectRatio", GLTFUniversalScopeValue { &_perspective.aspect_ratio, GLTF_TYPE_FLOAT } ),
            std::make_pair("yfov", GLTFUniversalScopeValue { &_perspective.yfov, GLTF_TYPE_FLOAT } ),
            std::make_pair("zfar", GLTFUniversalScopeValue { &_perspective.zfar, GLTF_TYPE_FLOAT } ),
            std::make_pair("znear", GLTFUniversalScopeValue { &_perspective.znear, GLTF_TYPE_FLOAT } ),
            std::make_pair("extensions", GLTFUniversalScopeValue { &_perspective.extensions, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } ),
            std::make_pair("extras", GLTFUniversalScopeValue { &_perspective.extras, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } )
        };

        _IterateSubNodes(_node, values);
    }


    void GLTFParser::_ReadImages(JSONNode *_node) {
        GLTFImage image;
        std::unordered_map<std::string, GLTFUniversalScopeValue> values = {
            std::make_pair("uri", GLTFUniversalScopeValue { &image.uri, GLTF_TYPE_STRING } ),
            std::make_pair("mimeType", GLTFUniversalScopeValue { &image.mime_type, GLTF_TYPE_STRING } ),
            std::make_pair("bufferView", GLTFUniversalScopeValue { &image.buffer_view, GLTF_TYPE_INTEGER } ),
            std::make_pair("name", GLTFUniversalScopeValue { &image.name, GLTF_TYPE_STRING } ),
            std::make_pair("extensions", GLTFUniversalScopeValue { &image.extensions, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } ),
            std::make_pair("extras", GLTFUniversalScopeValue { &image.extras, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } )
        };

        _IterateValueObjects<GLTFImage>(_node, values, image, m_root.images);
    }


    void GLTFParser::_ReadMaterials(JSONNode *_node) {
        GLTFMaterial material;
        std::vector<JSONNumber> emissive_factor;
        std::unordered_map<std::string, GLTFUniversalScopeValue> values = {
            std::make_pair("name", GLTFUniversalScopeValue { &material.name, GLTF_TYPE_STRING } ),
            std::make_pair("extensions", GLTFUniversalScopeValue { &material.extensions, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } ),
            std::make_pair("extras", GLTFUniversalScopeValue { &material.extras, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } ),
            std::make_pair("pbrMetallicRoughness", GLTFUniversalScopeValue { &material.pbr_metallic_roughness, GLTF_TYPE_MATERIAL_PBR_METALLIC_ROUGHNESS } ),
            std::make_pair("normalTexture", GLTFUniversalScopeValue { &material.normal_texture, GLTF_TYPE_MATERIAL_NORMAL_TEXTURE } ),
            std::make_pair("occlusionTexture", GLTFUniversalScopeValue { &material.occlusion_texture, GLTF_TYPE_MATERIAL_OCCLUSION_TEXTURE } ),
            std::make_pair("emissiveTexture", GLTFUniversalScopeValue { &material.emissive_texture, GLTF_TYPE_MATERIAL_TEXTURE_INFO } ),
            std::make_pair("emissiveFactor", GLTFUniversalScopeValue { &emissive_factor, GLTF_TYPE_FLOAT_ARRAY } ),
            std::make_pair("alphaMode", GLTFUniversalScopeValue { &material.alpha_mode, GLTF_TYPE_STRING } ),
            std::make_pair("alphaCutoff", GLTFUniversalScopeValue { &material.alpha_cutoff, GLTF_TYPE_FLOAT } ),
            std::make_pair("doubleSided", GLTFUniversalScopeValue { &material.double_sided, GLTF_TYPE_BOOLEAN } )
        };

        // for each element in values
        for(size_t i = 0; i < _node->values.size(); i++) {
            // error: invalid element type, only JSON objects are supported
            if(_node->values[i].first != JSON_TYPE_OBJECT)
                m_error.Error(LIBDAS_ERROR_INVALID_TYPE, _node->key_val_decl_line, _node->name, "JSON object");

            _IterateSubNodes(std::any_cast<JSONNode>(&_node->values[i].second), values);

            // check if emissive factor should be considered
            if(emissive_factor.size() == 3)
                material.emissive_factor = *reinterpret_cast<Point3D<JSONNumber>*>(emissive_factor.data());
            m_root.materials.push_back(material);
            material = GLTFMaterial();
        }
    }


    void GLTFParser::_ReadMaterialPbrMetallicRoughness(JSONNode *_node, GLTFpbrMetallicRoughness &_met_roughness) {
        std::vector<JSONNumber> base_color_factor;
        std::unordered_map<std::string, GLTFUniversalScopeValue> values = {
            std::make_pair("baseColorFactor", GLTFUniversalScopeValue { &base_color_factor, GLTF_TYPE_FLOAT_ARRAY } ),
            std::make_pair("baseColorTexture", GLTFUniversalScopeValue { &_met_roughness.base_color_texture, GLTF_TYPE_MATERIAL_TEXTURE_INFO } ),
            std::make_pair("metallicFactor", GLTFUniversalScopeValue { &_met_roughness.metallic_factor, GLTF_TYPE_FLOAT } ),
            std::make_pair("roughnessFactor", GLTFUniversalScopeValue { &_met_roughness.roughness_factor, GLTF_TYPE_FLOAT } ),
            std::make_pair("metallicRoughnessTexture", GLTFUniversalScopeValue { &_met_roughness.metallic_roughness_texture, GLTF_TYPE_MATERIAL_TEXTURE_INFO } ),
            std::make_pair("extensions", GLTFUniversalScopeValue { &_met_roughness.extensions, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } ),
            std::make_pair("extras", GLTFUniversalScopeValue { &_met_roughness.extras, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } ),
        };

        _IterateSubNodes(_node, values);

        if(base_color_factor.size() == 4)
            _met_roughness.base_color_factor = *reinterpret_cast<Point4D<float>*>(&base_color_factor);
    }


    void GLTFParser::_ReadMaterialNormalTexture(JSONNode *_node, GLTFNormalTextureInfo &_norm_tex) {
        std::unordered_map<std::string, GLTFUniversalScopeValue> values = {
            std::make_pair("index", GLTFUniversalScopeValue { &_norm_tex.index, GLTF_TYPE_INTEGER } ),
            std::make_pair("texCoord", GLTFUniversalScopeValue { &_norm_tex.tex_coord, GLTF_TYPE_INTEGER } ),
            std::make_pair("scale", GLTFUniversalScopeValue { &_norm_tex.scale, GLTF_TYPE_FLOAT } ),
            std::make_pair("extensions", GLTFUniversalScopeValue { &_norm_tex.extensions, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } ),
            std::make_pair("extras", GLTFUniversalScopeValue { &_norm_tex.extras, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } )
        };

        _IterateSubNodes(_node, values);
    }


    void GLTFParser::_ReadMaterialOcclusionTexture(JSONNode *_node, GLTFOcclusionTextureInfo &_occlusion_tex) {
        std::unordered_map<std::string, GLTFUniversalScopeValue> values = {
            std::make_pair("index", GLTFUniversalScopeValue { &_occlusion_tex.index, GLTF_TYPE_INTEGER } ),
            std::make_pair("texCoord", GLTFUniversalScopeValue { &_occlusion_tex.tex_coord, GLTF_TYPE_INTEGER } ),
            std::make_pair("strength", GLTFUniversalScopeValue { &_occlusion_tex.strength, GLTF_TYPE_FLOAT } ),
            std::make_pair("extensions", GLTFUniversalScopeValue { &_occlusion_tex.extensions, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } ),
            std::make_pair("extras", GLTFUniversalScopeValue { &_occlusion_tex.extras, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } )
        };

        _IterateSubNodes(_node, values);
    }


    void GLTFParser::_ReadMaterialTextureInfo(JSONNode *_node, GLTFTextureInfo &_tex_info) {
        std::unordered_map<std::string, GLTFUniversalScopeValue> values = {
            std::make_pair("index", GLTFUniversalScopeValue { &_tex_info.index, GLTF_TYPE_INTEGER } ),
            std::make_pair("texCoord", GLTFUniversalScopeValue { &_tex_info.tex_coord, GLTF_TYPE_INTEGER } ),
            std::make_pair("extensions", GLTFUniversalScopeValue { &_tex_info.extensions, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } ),
            std::make_pair("extras", GLTFUniversalScopeValue { &_tex_info.extras, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } ),
        };
    }


    void GLTFParser::_ReadMeshes(JSONNode *_node) {
        GLTFMesh mesh;
        std::unordered_map<std::string, GLTFUniversalScopeValue> values = {
            std::make_pair("primitives", GLTFUniversalScopeValue { &mesh.primitives, GLTF_TYPE_MESH_PRIMITIVES } ),
            std::make_pair("weights", GLTFUniversalScopeValue { &mesh.weights, GLTF_TYPE_FLOAT_ARRAY } ),
            std::make_pair("name", GLTFUniversalScopeValue { &mesh.name, GLTF_TYPE_STRING } ),
            std::make_pair("extensions", GLTFUniversalScopeValue { &mesh.extensions, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } ),
            std::make_pair("extras", GLTFUniversalScopeValue { &mesh.extras, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } ),
        };

        _IterateValueObjects<GLTFMesh>(_node, values, mesh, m_root.meshes);
    }


    void GLTFParser::_ReadMeshPrimitives(JSONNode *_node, std::vector<GLTFMeshPrimitive> &_primitives) {
        GLTFMeshPrimitive primitive;
        std::unordered_map<std::string, GLTFUniversalScopeValue> values = {
            std::make_pair("attributes", GLTFUniversalScopeValue { &primitive.attributes, GLTF_TYPE_MESH_PRIMITIVE_ATTRIBUTES } ),
            std::make_pair("indices", GLTFUniversalScopeValue { &primitive.indices, GLTF_TYPE_INTEGER } ),
            std::make_pair("material", GLTFUniversalScopeValue { &primitive.material, GLTF_TYPE_INTEGER } ),
            std::make_pair("mode", GLTFUniversalScopeValue { &primitive.mode, GLTF_TYPE_INTEGER } ),
            std::make_pair("targets", GLTFUniversalScopeValue { &primitive.targets, GLTF_TYPE_MESH_PRIMITIVE_ATTRIBUTES } ),
            std::make_pair("extensions", GLTFUniversalScopeValue { &primitive.extensions, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } ),
            std::make_pair("extras", GLTFUniversalScopeValue { &primitive.extras, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } ),
        };

        _IterateValueObjects<GLTFMeshPrimitive>(_node, values, primitive, _primitives);
    }


    void GLTFParser::_ReadMeshPrimitiveAttributes(JSONNode *_node, GLTFMeshPrimitive::AttributesType &_attrs) {
        // iterate through each subnode now
        for(auto it = _node->sub_nodes.begin(); it != _node->sub_nodes.end(); it++) {
            _VerifySourceData(it->second.get(), JSON_TYPE_INTEGER, false);
            _attrs.push_back(std::make_pair(it->first, std::any_cast<uint32_t>(it->second->values.back())));
        }
    }


    void GLTFParser::_ReadNodes(JSONNode *_node) {
        GLTFNode node;
        std::vector<float> matrix;      // size must be 16 elements 
        std::vector<float> rotation;    // size must be 4 elements
        std::vector<float> scale;       // size must be 3 elements
        std::vector<float> translation; // size must be 3 elements

        std::unordered_map<std::string, GLTFUniversalScopeValue> values = {
            std::make_pair("camera", GLTFUniversalScopeValue { &node.camera, GLTF_TYPE_INTEGER } ),
            std::make_pair("children", GLTFUniversalScopeValue { &node.children, GLTF_TYPE_INTEGER_ARRAY } ),
            std::make_pair("skin", GLTFUniversalScopeValue { &node.skin, GLTF_TYPE_INTEGER } ),
            std::make_pair("matrix", GLTFUniversalScopeValue { &matrix, GLTF_TYPE_FLOAT_ARRAY } ),
            std::make_pair("mesh", GLTFUniversalScopeValue { &node.mesh, GLTF_TYPE_INTEGER } ),
            std::make_pair("rotation", GLTFUniversalScopeValue { &rotation, GLTF_TYPE_FLOAT_ARRAY } ),
            std::make_pair("scale", GLTFUniversalScopeValue { &scale, GLTF_TYPE_FLOAT_ARRAY } ),
            std::make_pair("translation", GLTFUniversalScopeValue { &translation, GLTF_TYPE_FLOAT_ARRAY } ),
            std::make_pair("weights", GLTFUniversalScopeValue { &node.weights, GLTF_TYPE_FLOAT_ARRAY } ),
            std::make_pair("name", GLTFUniversalScopeValue { &node.name, GLTF_TYPE_STRING } ),
            std::make_pair("extensions", GLTFUniversalScopeValue { &node.extensions, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } ),
            std::make_pair("extras", GLTFUniversalScopeValue { &node.extras, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } )
        };

        // for each node in nodes
        for(size_t i = 0; i < _node->values.size(); i++) {
            // error: invalid value type, expected JSON object
            if(_node->values[i].first != JSON_TYPE_OBJECT)
                m_error.Error(LIBDAS_ERROR_INVALID_TYPE, _node->key_val_decl_line, _node->name, "JSON object");

            _IterateSubNodes(std::any_cast<JSONNode>(&_node->values[i].second), values);

            // append matrix data into correct data structure if possible
            if(matrix.size() == 16) {
                Matrix4<float> *data = reinterpret_cast<Matrix4<float>*>(matrix.data());
                node.matrix = data->Transpose(); // transpose from column major to row major matrix
            }

            // append rotation data into correct data structure if possible
            if(rotation.size() == 4)
                node.rotation = *reinterpret_cast<Vector4<float>*>(rotation.data());

            // append scale data into correct data structure if possible
            if(scale.size() == 3)
                node.scale = *reinterpret_cast<Point3D<float>*>(scale.data());

            // append translation data into correct data structure if possible
            if(translation.size() == 3)
                node.translation = *reinterpret_cast<Point3D<float>*>(translation.data());

            // clear all tmp structures
            matrix.clear();
            rotation.clear();
            scale.clear();
            translation.clear();
            m_root.nodes.push_back(node);
            node = GLTFNode();
        }
    }


    void GLTFParser::_ReadSamplers(JSONNode *_node) {
        GLTFSampler sampler;
        std::unordered_map<std::string, GLTFUniversalScopeValue> values = {
            std::make_pair("magFilter", GLTFUniversalScopeValue { &sampler.mag_filter, GLTF_TYPE_INTEGER } ),
            std::make_pair("minFilter", GLTFUniversalScopeValue { &sampler.min_filter, GLTF_TYPE_INTEGER } ),
            std::make_pair("wrapS", GLTFUniversalScopeValue { &sampler.wrap_s, GLTF_TYPE_INTEGER } ),
            std::make_pair("wrapT", GLTFUniversalScopeValue { &sampler.wrap_t, GLTF_TYPE_INTEGER } ),
            std::make_pair("name", GLTFUniversalScopeValue { &sampler.name, GLTF_TYPE_STRING } ),
            std::make_pair("extensions", GLTFUniversalScopeValue { &sampler.extensions, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } ),
            std::make_pair("extras", GLTFUniversalScopeValue { &sampler.extras, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } ),
        };

        _IterateValueObjects<GLTFSampler>(_node, values, sampler, m_root.samplers);
    }


    void GLTFParser::_ReadScenes(JSONNode *_node, bool is_root) {
        GLTFScene scene;
        std::unordered_map<std::string, GLTFUniversalScopeValue> values = {
            std::make_pair("nodes", GLTFUniversalScopeValue { &scene.nodes, GLTF_TYPE_INTEGER_ARRAY } ),
            std::make_pair("name", GLTFUniversalScopeValue { &scene.name, GLTF_TYPE_STRING } ),
            std::make_pair("extensions", GLTFUniversalScopeValue { &scene.extensions, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } ),
            std::make_pair("extras", GLTFUniversalScopeValue { &scene.extras, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } )
        };

        if(!is_root) 
            _IterateValueObjects<GLTFScene>(_node, values, scene, m_root.scenes);
        else {
            _IterateSubNodes(_node, values);
            m_root.load_time_scene = scene;
        }
    }


    void GLTFParser::_ReadSkins(JSONNode *_node) {
        GLTFSkin skin;
        std::unordered_map<std::string, GLTFUniversalScopeValue> values = {
            std::make_pair("inverseBindMatrices", GLTFUniversalScopeValue { &skin.inverse_bind_matrices, GLTF_TYPE_INTEGER } ),
            std::make_pair("skeleton", GLTFUniversalScopeValue { &skin.skeleton, GLTF_TYPE_INTEGER } ),
            std::make_pair("joints", GLTFUniversalScopeValue { &skin.joints, GLTF_TYPE_INTEGER_ARRAY } ),
            std::make_pair("name", GLTFUniversalScopeValue { &skin.name, GLTF_TYPE_STRING } ),
            std::make_pair("extensions", GLTFUniversalScopeValue { &skin.extensions, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } ),
            std::make_pair("extras", GLTFUniversalScopeValue { &skin.extras, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } )
        };

        _IterateValueObjects<GLTFSkin>(_node, values, skin, m_root.skins);
    }


    void GLTFParser::_ReadTextures(JSONNode *_node) {
        GLTFTexture texture;
        std::unordered_map<std::string, GLTFUniversalScopeValue> values = {
            std::make_pair("sampler", GLTFUniversalScopeValue { &texture.sampler, GLTF_TYPE_INTEGER } ),
            std::make_pair("source", GLTFUniversalScopeValue { &texture.source, GLTF_TYPE_INTEGER } ),
            std::make_pair("name", GLTFUniversalScopeValue { &texture.name, GLTF_TYPE_STRING } ),
            std::make_pair("extensions", GLTFUniversalScopeValue { &texture.extensions, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } ),
            std::make_pair("extras", GLTFUniversalScopeValue { &texture.extras, GLTF_TYPE_EXTRAS_OR_EXTENSIONS } )
        };

        _IterateValueObjects<GLTFTexture>(_node, values, texture, m_root.textures);
    }


    void GLTFParser::_RootObjectParserCaller(GLTFObjectType _type, JSONNode *_node) {
        switch(_type) {
            case GLTF_OBJECT_ACCESSORS:
                _ReadAccessors(_node);
                break;

            case GLTF_OBJECT_ANIMATIONS:
                _ReadAnimations(_node);
                break;

            case GLTF_OBJECT_ASSET:
                _ReadAsset(_node);
                break;

            case GLTF_OBJECT_BUFFERS:
                _ReadBuffers(_node);
                break;

            case GLTF_OBJECT_BUFFERVIEWS:
                _ReadBufferviews(_node);
                break;

            case GLTF_OBJECT_CAMERAS:
                _ReadCameras(_node);
                break;

            case GLTF_OBJECT_EXTENSIONS:
            case GLTF_OBJECT_EXTRAS:
                break;

            case GLTF_OBJECT_IMAGES:
                _ReadImages(_node);
                break;

            case GLTF_OBJECT_MATERIALS:
                _ReadMaterials(_node);
                break;

            case GLTF_OBJECT_MESHES:
                _ReadMeshes(_node);
                break;

            case GLTF_OBJECT_NODES:
                _ReadNodes(_node);
                break;

            case GLTF_OBJECT_SAMPLERS:
                _ReadSamplers(_node);
                break;

            case GLTF_OBJECT_SCENE:
                _ReadScenes(_node, true);
                break;

            case GLTF_OBJECT_SCENES:
                _ReadScenes(_node, false);
                break;

            case GLTF_OBJECT_SKINS:
                _ReadSkins(_node);
                break;

            case GLTF_OBJECT_TEXTURES:
                _ReadTextures(_node);
                break;

            default:
                LIBDAS_ASSERT(false);
                break;
        }
    }


    void GLTFParser::Parse(const std::string &_file_name) {
        // parse json files with JSON parser
        JSONParser::Parse(_file_name);

        JSONNode &root = JSONParser::GetRootNode();

        // traverse the root node for data
        for(auto it = root.sub_nodes.begin(); it != root.sub_nodes.end(); it++) {
            GLTFObjectType type = _FindRootObjectType(it->first, it->second->key_val_decl_line);
            _RootObjectParserCaller(type, it->second.get());
        }
    }


    GLTFRoot &GLTFParser::GetRootObject() {
        return m_root;
    }
}
