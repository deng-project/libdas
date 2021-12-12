/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: DasReaderCore.h - DAS file format reader class header
/// author: Karl-Mihkel Ott

#define DAS_READER_CORE_CPP
#include <DasReaderCore.h>

namespace Libdas {

    DasReaderCore::DasReaderCore(const std::string &_file_name) : 
        AsciiLineReader(_file_name, DEFAULT_CHUNK, "ENDSCOPE\n"), 
        m_error(MODEL_FORMAT_DAS) 
    {
        _CreateScopeNameMap();
        _CreateScopeValueTypeMap();
        _SetLineBounds(std::make_pair(m_buffer, m_buffer + m_buffer_size - 1));
    }


    void DasReaderCore::_CreateScopeNameMap() {
        m_scope_name_map["PROPERTIES"] = LIBDAS_DAS_SCOPE_PROPERTIES;
        m_scope_name_map["BUFFER"] = LIBDAS_DAS_SCOPE_BUFFER;
        m_scope_name_map["MODEL"] = LIBDAS_DAS_SCOPE_MODEL;
        m_scope_name_map["ANIMATION"] = LIBDAS_DAS_SCOPE_ANIMATION;
        m_scope_name_map["KEYFRAME"] = LIBDAS_DAS_SCOPE_KEYFRAME;
        m_scope_name_map["SCENE"] = LIBDAS_DAS_SCOPE_SCENE;
        m_scope_name_map["NODE"] = LIBDAS_DAS_SCOPE_SCENE_NODE;
    }


    void DasReaderCore::_CreateScopeValueTypeMap() {
        // PROPERTIES 
        m_unique_val_map["MODEL"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_MODEL;
        m_unique_val_map["AUTHOR"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_AUTHOR;
        m_unique_val_map["COPYRIGHT"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_COPYRIGHT;
        m_unique_val_map["MODDATE"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_MODDATE;
        m_unique_val_map["COMPRESSION"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_COMPRESSION;

        // BUFFER
        m_unique_val_map["BUFFERTYPE"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_BUFFER_TYPE;
        m_unique_val_map["DATALEN"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_DATA_LEN;
        m_unique_val_map["DATA"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_DATA;

        // MODEL
        m_unique_val_map["INDEXBUFFERID"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_INDEX_BUFFER_ID;
        m_unique_val_map["INDEXBUFFEROFFSET"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_INDEX_BUFFER_OFFSET;
        m_unique_val_map["INDICESCOUNT"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_INDICES_COUNT;
        m_unique_val_map["TEXTUREID"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_TEXTURE_ID;

        // ANIMATION
        m_unique_val_map["MODEL"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_MODEL;
        m_unique_val_map["LENGTH"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_LENGTH;
        m_unique_val_map["INTERPOLATION"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_INTERPOLATION;

        // KEYFRAME
        m_unique_val_map["TIMESTAMP"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_TIMESTAMP;

        // NODE
        m_unique_val_map["CHILDRENCOUNT"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_CHILDREN_COUNT;
        m_unique_val_map["MODELCOUNT"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_MODEL_COUNT;
        m_unique_val_map["ANIMATIONCOUNT"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_ANIMATION_COUNT;

        // universal
        m_unique_val_map["NAME"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_NAME;
        m_unique_val_map["TRANSFORM"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_TRANSFORM;
        m_unique_val_map["VERTEXBUFFERID"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_BUFFER_ID;
        m_unique_val_map["VERTEXBUFFEROFFSET"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_BUFFER_OFFSET;
        m_unique_val_map["TEXTUREMAPBUFFERID"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_TEXTURE_MAP_BUFFER_ID;
        m_unique_val_map["TEXTUREMAPBUFFEROFFSET"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_TEXTURE_MAP_BUFFER_OFFSET;
        m_unique_val_map["VERTEXNORMALBUFFERID"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_NORMAL_BUFFER_ID;
        m_unique_val_map["VERTEXNORMALBUFFEROFFSET"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_NORMAL_BUFFER_OFFSET;
    }


    DasUniqueValueType DasReaderCore::_FindUniqueValueType(const std::string &_value) {
        if(m_unique_val_map.find(_value) == m_unique_val_map.end())
            return LIBDAS_DAS_UNIQUE_VALUE_TYPE_UNKNOWN;

        return m_unique_val_map[_value];
    }


    std::any DasReaderCore::_GetValueInformation(const DasScopeType _parent, const std::string &_value) {
        DasUniqueValueType type = _FindUniqueValueType(_value);

        // nested switch statement mess
        switch(_parent) {
            case LIBDAS_DAS_SCOPE_PROPERTIES:
                switch(type) {
                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_MODEL:
                        return DasProperties::LIBDAS_PROPERTIES_MODEL;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_AUTHOR:
                        return DasProperties::LIBDAS_PROPERTIES_AUTHOR;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_COPYRIGHT:
                        return DasProperties::LIBDAS_PROPERTIES_COPYRIGHT;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_MODDATE:
                        return DasProperties::LIBDAS_PROPERTIES_MODDATE;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_COMPRESSION:
                        return DasProperties::LIBDAS_PROPERTIES_COMPRESSION;

                    default:
                        return LIBDAS_DAS_UNIQUE_VALUE_TYPE_UNKNOWN;
                }
                break;

            case LIBDAS_DAS_SCOPE_BUFFER:
                switch(type) {
                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_BUFFER_TYPE:
                        return DasBuffer::LIBDAS_BUFFER_BUFFER_TYPE;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_DATA_LEN:
                        return DasBuffer::LIBDAS_BUFFER_DATA_LEN;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_DATA:
                        return DasBuffer::LIBDAS_BUFFER_DATA;

                    default:
                        return LIBDAS_DAS_UNIQUE_VALUE_TYPE_UNKNOWN;
                }
                break;

            case LIBDAS_DAS_SCOPE_MODEL:
                switch(type) {
                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_NAME:
                        return DasModel::LIBDAS_MODEL_NAME;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_INDEX_BUFFER_ID:
                        return DasModel::LIBDAS_MODEL_INDEX_BUFFER_ID;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_INDEX_BUFFER_OFFSET:
                        return DasModel::LIBDAS_MODEL_INDEX_BUFFER_OFFSET;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_INDICES_COUNT:
                        return DasModel::LIBDAS_MODEL_INDICES_COUNT;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_BUFFER_ID:
                        return DasModel::LIBDAS_MODEL_VERTEX_BUFFER_ID;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_BUFFER_OFFSET:
                        return DasModel::LIBDAS_MODEL_VERTEX_BUFFER_OFFSET;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_TEXTURE_ID:
                        return DasModel::LIBDAS_MODEL_TEXTURE_ID;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_TEXTURE_MAP_BUFFER_ID:
                        return DasModel::LIBDAS_MODEL_TEXTURE_MAP_BUFFER_ID;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_TEXTURE_MAP_BUFFER_OFFSET:
                        return DasModel::LIBDAS_MODEL_TEXTURE_MAP_BUFFER_OFFSET;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_NORMAL_BUFFER_ID:
                        return DasModel::LIBDAS_MODEL_VERTEX_NORMAL_BUFFER_ID;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_NORMAL_BUFFER_OFFSET:
                        return DasModel::LIBDAS_MODEL_VERTEX_NORMAL_BUFFER_OFFSET;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_TRANSFORM:
                        return DasModel::LIBDAS_MODEL_TRANSFORM;

                    default:
                        return LIBDAS_DAS_UNIQUE_VALUE_TYPE_UNKNOWN;
                }
                break;

            case LIBDAS_DAS_SCOPE_ANIMATION:
                switch(type) {
                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_NAME:
                        return DasAnimation::LIBDAS_ANIMATION_NAME;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_MODEL:
                        return DasAnimation::LIBDAS_ANIMATION_NAME;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_LENGTH:
                        return DasAnimation::LIBDAS_ANIMATION_NAME;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_INTERPOLATION:
                        return DasAnimation::LIBDAS_ANIMATION_NAME;

                    default:
                        return LIBDAS_DAS_UNIQUE_VALUE_TYPE_UNKNOWN;
                }
                break;

            case LIBDAS_DAS_SCOPE_KEYFRAME:
                switch(type) {
                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_TIMESTAMP:
                        return DasKeyframe::LIBDAS_KEYFRAME_TIMESTAMP;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_BUFFER_ID:
                        return DasKeyframe::LIBDAS_KEYFRAME_VERTEX_BUFFER_ID;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_BUFFER_OFFSET:
                        return DasKeyframe::LIBDAS_KEYFRAME_VERTEX_BUFFER_OFFSET;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_TEXTURE_MAP_BUFFER_ID:
                        return DasKeyframe::LIBDAS_KEYFRAME_VERTEX_BUFFER_ID;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_TEXTURE_MAP_BUFFER_OFFSET:
                        return DasKeyframe::LIBDAS_KEYFRAME_VERTEX_BUFFER_ID;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_NORMAL_BUFFER_ID:
                        return DasKeyframe::LIBDAS_KEYFRAME_VERTEX_BUFFER_ID;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_NORMAL_BUFFER_OFFSET:
                        return DasKeyframe::LIBDAS_KEYFRAME_VERTEX_BUFFER_ID;

                    default:
                        return LIBDAS_DAS_UNIQUE_VALUE_TYPE_UNKNOWN;
                }
                break;

            case LIBDAS_DAS_SCOPE_SCENE:
                switch(type) {
                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_NAME:
                        return DasScene::LIBDAS_SCENE_NAME;

                    default:
                        return LIBDAS_DAS_UNIQUE_VALUE_TYPE_UNKNOWN;
                }
                break;

            case LIBDAS_DAS_SCOPE_SCENE_NODE:
                switch(type) {
                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_NAME:
                        return DasSceneNode::LIBDAS_SCENE_NODE_NAME;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_CHILDREN_COUNT:
                        return DasSceneNode::LIBDAS_SCENE_NODE_CHILDREN_COUNT;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_CHILDREN:
                        return DasSceneNode::LIBDAS_SCENE_NODE_CHILDREN;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_MODEL_COUNT:
                        return DasSceneNode::LIBDAS_SCENE_NODE_MODEL_COUNT;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_MODELS:
                        return DasSceneNode::LIBDAS_SCENE_NODE_MODELS;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_ANIMATION_COUNT:
                        return DasSceneNode::LIBDAS_SCENE_NODE_ANIMATION_COUNT;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_ANIMATIONS:
                        return DasSceneNode::LIBDAS_SCENE_NODE_ANIMATIONS;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_TRANSFORM:
                        return DasSceneNode::LIBDAS_SCENE_NODE_TRANSFORM;

                    default:
                        return LIBDAS_DAS_UNIQUE_VALUE_TYPE_UNKNOWN;
                }
                break;

            default:
                return LIBDAS_DAS_UNIQUE_VALUE_TYPE_UNKNOWN;
        }
    }


    void DasReaderCore::_ReadPropertiesValue(DasProperties *_props, DasProperties::ValueType _type) {
        switch(_type) {
            case DasProperties::LIBDAS_PROPERTIES_MODEL: 
                _props->model = _ExtractString();
                break;

            case DasProperties::LIBDAS_PROPERTIES_AUTHOR:
                _props->author = _ExtractString();
                break;

            case DasProperties::LIBDAS_PROPERTIES_COPYRIGHT:
                _props->copyright = _ExtractString();
                break;

            case DasProperties::LIBDAS_PROPERTIES_MODDATE:
                _props->moddate = *reinterpret_cast<uint64_t*>(_GetReadPtr());
                _SkipData(sizeof(uint64_t));
                break;

            case DasProperties::LIBDAS_PROPERTIES_COMPRESSION:
                _props->compression = reinterpret_cast<bool*>(_GetReadPtr());
                _SkipData(sizeof(bool));
                break;

            default:
                LIBDAS_ASSERT(false);
                break;
        }
    }


    void DasReaderCore::_ReadBufferValue(DasBuffer *_buffer, DasBuffer::ValueType _type) {
        switch(_type) {
            case DasBuffer::LIBDAS_BUFFER_BUFFER_TYPE:
                _buffer->type = *reinterpret_cast<BufferType*>(_GetReadPtr());
                _SkipData(sizeof(BufferType));
                break;

            case DasBuffer::LIBDAS_BUFFER_DATA_LEN:
                _buffer->data_len = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _SkipData(sizeof(uint32_t));
                break;

            case DasBuffer::LIBDAS_BUFFER_DATA:
                _buffer->data_ptrs.push_back(std::make_pair(_ExtractBlob(_buffer->data_len), _buffer->data_len));

                // check if blob reading was successful
                if(!_buffer->data_ptrs.back().first)
                    m_error.Error(LIBDAS_ERROR_INVALID_DATA);
                break;

            default:
                LIBDAS_ASSERT(false);
                break;
        }
    }


    void DasReaderCore::_ReadModelValue(DasModel *_model, DasModel::ValueType _type) {
        switch(_type) {
            case DasModel::LIBDAS_MODEL_NAME:
                _model->name = _ExtractString();
                break;

            case DasModel::LIBDAS_MODEL_INDEX_BUFFER_ID:
                _model->index_buffer_id = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _SkipData(sizeof(uint32_t));
                break;

            case DasModel::LIBDAS_MODEL_INDEX_BUFFER_OFFSET:
                _model->index_buffer_offset = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _SkipData(sizeof(uint32_t));
                break;

            case DasModel::LIBDAS_MODEL_INDICES_COUNT:
                _model->indices_count = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _SkipData(sizeof(uint32_t));
                break;

            case DasModel::LIBDAS_MODEL_VERTEX_BUFFER_ID:
                _model->vertex_buffer_id = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _SkipData(sizeof(uint32_t));
                break;

            case DasModel::LIBDAS_MODEL_VERTEX_BUFFER_OFFSET:
                _model->vertex_buffer_offset = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _SkipData(sizeof(uint32_t));
                break;

            case DasModel::LIBDAS_MODEL_TEXTURE_ID:
                _model->texture_id = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _SkipData(sizeof(uint32_t));
                break;

            case DasModel::LIBDAS_MODEL_TEXTURE_MAP_BUFFER_ID:
                _model->texture_map_buffer_id = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _SkipData(sizeof(uint32_t));
                break;

            case DasModel::LIBDAS_MODEL_TEXTURE_MAP_BUFFER_OFFSET:
                _model->texture_map_buffer_offset = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _SkipData(sizeof(uint32_t));
                break;

            case DasModel::LIBDAS_MODEL_VERTEX_NORMAL_BUFFER_ID:
                _model->vertex_normal_buffer_id = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _SkipData(sizeof(uint32_t));
                break;

            case DasModel::LIBDAS_MODEL_VERTEX_NORMAL_BUFFER_OFFSET:
                _model->vertex_normal_buffer_offset = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _SkipData(sizeof(uint32_t));
                break;

            case DasModel::LIBDAS_MODEL_TRANSFORM:
                _model->transform = *reinterpret_cast<Matrix4<float>*>(_GetReadPtr());
                _SkipData(sizeof(Matrix4<float>));
                break;

            default:
                LIBDAS_ASSERT(false);
                break;
        }
    }


    void DasReaderCore::_ReadAnimationValue(DasAnimation *_animation, DasAnimation::ValueType _type) {
        switch(_type) {
            case DasAnimation::LIBDAS_ANIMATION_NAME:
                _animation->name = _ExtractString();
                break;

            case DasAnimation::LIBDAS_ANIMATION_MODEL:
                _animation->model = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _SkipData(sizeof(uint32_t));
                break;

            case DasAnimation::LIBDAS_ANIMATION_LENGTH:
                _animation->length = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _SkipData(sizeof(uint32_t));
                break;

            case DasAnimation::LIBDAS_ANIMATION_INTERPOLATION:
                _animation->interpolation = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _SkipData(sizeof(InterpolationValue));
                break;

            default:
                LIBDAS_ASSERT(false);
                break;
        }
    }


    void DasReaderCore::_ReadKeyframeValue(DasKeyframe *_keyframe, DasKeyframe::ValueType _type) {
        switch(_type) {
            case DasKeyframe::LIBDAS_KEYFRAME_TIMESTAMP:
                _keyframe->timestamp = *reinterpret_cast<uint64_t*>(_GetReadPtr());
                _SkipData(sizeof(uint64_t));
                break;

            case DasKeyframe::LIBDAS_KEYFRAME_VERTEX_BUFFER_ID:
                _keyframe->vertex_buffer_id = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _SkipData(sizeof(uint32_t));
                break;

            case DasKeyframe::LIBDAS_KEYFRAME_VERTEX_BUFFER_OFFSET:
                _keyframe->vertex_buffer_offset = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _SkipData(sizeof(uint32_t));
                break;

            case DasKeyframe::LIBDAS_KEYFRAME_TEXTURE_MAP_VERTICES_BUFFER_ID:
                _keyframe->texture_map_buffer_id = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _SkipData(sizeof(uint32_t));
                break;

            case DasKeyframe::LIBDAS_KEYFRAME_TEXTURE_MAP_VERTICES_BUFFER_OFFSET:
                _keyframe->texture_map_buffer_offset = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _SkipData(sizeof(uint32_t));
                break;

            case DasKeyframe::LIBDAS_KEYFRAME_VERTEX_NORMAL_BUFFER_ID:
                _keyframe->vertex_normal_buffer_id = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _SkipData(sizeof(uint32_t));
                break;

            case DasKeyframe::LIBDAS_KEYFRAME_VERTEX_NORMAL_BUFFER_OFFSET:
                _keyframe->vertex_normal_buffer_offset = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _SkipData(sizeof(uint32_t));
                break;

            default:
                LIBDAS_ASSERT(false);
                break;
        }
    }


    void DasReaderCore::_ReadSceneValue(DasScene *_scene, DasScene::ValueType _type) {
        switch(_type) {
            case DasScene::LIBDAS_SCENE_NAME:
                _scene->name = _ExtractString();
                break;

            default:
                LIBDAS_ASSERT(false);
                break;
        }
    }


    void DasReaderCore::_ReadSceneNodeValue(DasSceneNode *_node, DasSceneNode::ValueType _type) {
        switch(_type) {
            case DasSceneNode::LIBDAS_SCENE_NODE_NAME:
                _node->name = _ExtractString();
                break;

            case DasSceneNode::LIBDAS_SCENE_NODE_CHILDREN_COUNT:
                _node->children_count = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _SkipData(sizeof(uint32_t));
                break;

            case DasSceneNode::LIBDAS_SCENE_NODE_CHILDREN:
                _node->children = reinterpret_cast<uint32_t*>(malloc(sizeof(uint32_t) * _node->children_count));
                memcpy(_node->children, _GetReadPtr(), sizeof(uint32_t) * _node->children_count);
                _SkipData(sizeof(uint32_t) * _node->children_count);
                break;

            case DasSceneNode::LIBDAS_SCENE_NODE_MODEL_COUNT:
                _node->model_count = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _SkipData(sizeof(uint32_t));
                break;

            case DasSceneNode::LIBDAS_SCENE_NODE_MODELS:
                _node->models = reinterpret_cast<uint32_t*>(malloc(sizeof(uint32_t) * _node->model_count));
                memcpy(_node->models, _GetReadPtr(), sizeof(uint32_t) * _node->model_count);
                _SkipData(sizeof(uint32_t) * _node->model_count);
                break;

            case DasSceneNode::LIBDAS_SCENE_NODE_ANIMATION_COUNT:
                _node->animation_count = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _SkipData(sizeof(uint32_t));
                break;

            case DasSceneNode::LIBDAS_SCENE_NODE_ANIMATIONS:
                _node->animations = reinterpret_cast<uint32_t*>(malloc(sizeof(uint32_t) * _node->animation_count));
                memcpy(_node->animations, _GetReadPtr(), sizeof(uint32_t) * _node->animation_count);
                _SkipData(sizeof(uint32_t) * _node->animation_count);
                break;

            case DasSceneNode::LIBDAS_SCENE_NODE_TRANSFORM:
                _node->transform = *reinterpret_cast<Matrix4<float>*>(_GetReadPtr());
                _SkipData(sizeof(Matrix4<float>));
                break;
            
            default:
                LIBDAS_ASSERT(false);
                break;
        }
    }


    void DasReaderCore::_ReadScopeValueDataCaller(std::any &_scope, DasScopeType _type, std::any &_value_type, const std::string &_val_str) {
        // any type check is necessary for correct error output
        switch(_type) {
            case LIBDAS_DAS_SCOPE_PROPERTIES:
                if(_value_type.type() != typeid(DasProperties::ValueType))
                    m_error.Error(LIBDAS_ERROR_INVALID_VALUE, _val_str);
                _ReadPropertiesValue(std::any_cast<DasProperties>(&_scope), std::any_cast<DasProperties::ValueType>(_value_type));
                break;

            case LIBDAS_DAS_SCOPE_BUFFER:
                if(_value_type.type() != typeid(DasBuffer::ValueType))
                    m_error.Error(LIBDAS_ERROR_INVALID_VALUE, _val_str);
                _ReadBufferValue(std::any_cast<DasBuffer>(&_scope), std::any_cast<DasBuffer::ValueType>(_value_type));
                break;

            case LIBDAS_DAS_SCOPE_MODEL:
                if(_value_type.type() != typeid(DasModel::ValueType))
                    m_error.Error(LIBDAS_ERROR_INVALID_VALUE, _val_str);
                _ReadModelValue(std::any_cast<DasModel>(&_scope), std::any_cast<DasModel::ValueType>(_value_type));
                break;

            case LIBDAS_DAS_SCOPE_ANIMATION:
                if(_value_type.type() != typeid(DasAnimation::ValueType))
                    m_error.Error(LIBDAS_ERROR_INVALID_VALUE, _val_str);
                _ReadAnimationValue(std::any_cast<DasAnimation>(&_scope), std::any_cast<DasAnimation::ValueType>(_value_type));
                break;

            case LIBDAS_DAS_SCOPE_KEYFRAME:
                if(_value_type.type() != typeid(DasKeyframe::ValueType))
                    m_error.Error(LIBDAS_ERROR_INVALID_VALUE, _val_str);
                _ReadKeyframeValue(std::any_cast<DasKeyframe>(&_scope), std::any_cast<DasKeyframe::ValueType>(_value_type));
                break;

            case LIBDAS_DAS_SCOPE_SCENE:
                if(_value_type.type() != typeid(DasScene::ValueType))
                    m_error.Error(LIBDAS_ERROR_INVALID_VALUE, _val_str);
                _ReadSceneValue(std::any_cast<DasScene>(&_scope), std::any_cast<DasScene::ValueType>(_value_type));
                break;

            case LIBDAS_DAS_SCOPE_SCENE_NODE:
                if(_value_type.type() != typeid(DasSceneNode::ValueType))
                    m_error.Error(LIBDAS_ERROR_INVALID_VALUE, _val_str);
                _ReadSceneNodeValue(std::any_cast<DasSceneNode>(&_scope), std::any_cast<DasSceneNode::ValueType>(_value_type));
                break;

            default:
                LIBDAS_ASSERT(false);
                break;
        }
    }


    void DasReaderCore::_VerifySubScope(DasScopeType _type, DasScopeType _sub_type, const std::any &_scope, const std::any &_sub_scope) {
        // parent scopes that have no subscopes will throw an incomplete scope error
        switch(_type) {
            case LIBDAS_DAS_SCOPE_ANIMATION:
                LIBDAS_ASSERT(_scope.type() == typeid(DasAnimation));

                // only one subscope type allowed thus check it for errors
                if(_sub_scope.type() != typeid(DasKeyframe))
                    m_error.Error(LIBDAS_ERROR_INVALID_DATA);

                std::any_cast<DasAnimation>(_scope).keyframes.push_back(std::any_cast<DasKeyframe>(_sub_scope));
                break;

            case LIBDAS_DAS_SCOPE_SCENE:
                LIBDAS_ASSERT(_scope.type() == typeid(DasScene));

                // ... //
                if(_sub_scope.type() != typeid(DasSceneNode))
                    m_error.Error(LIBDAS_ERROR_INVALID_DATA);

                std::any_cast<DasScene>(_scope).nodes.push_back(std::any_cast<DasSceneNode>(_sub_scope));
                break;

            default:
                m_error.Error(LIBDAS_ERROR_INCOMPLETE_SCOPE);
                break;
        }
    }


    std::any DasReaderCore::_GetAnyScopeStructure(DasScopeType _type) {
        switch(_type) {
            case LIBDAS_DAS_SCOPE_PROPERTIES:
                return DasProperties();

            case LIBDAS_DAS_SCOPE_BUFFER:
                return DasBuffer();

            case LIBDAS_DAS_SCOPE_MODEL:
                return DasModel();

            case LIBDAS_DAS_SCOPE_ANIMATION:
                return DasAnimation();

            case LIBDAS_DAS_SCOPE_KEYFRAME:
                return DasKeyframe();

            case LIBDAS_DAS_SCOPE_SCENE:
                return DasScene();

            case LIBDAS_DAS_SCOPE_SCENE_NODE:
                return DasSceneNode();

            default:
                LIBDAS_ASSERT(false);
                break;
        }

        return std::any();
    }


    void DasReaderCore::ReadSignature() {
        DasSignature exp_sig;
        DasSignature sig;

        char *sig_ptr = _ExtractBlob(sizeof(DasSignature), reinterpret_cast<char*>(&sig));
        if(!sig_ptr)
            m_error.Error(LIBDAS_ERROR_INVALID_SIGNATURE);

        bool is_pad = false;
        
        // verify signature integrity
        if(sig.magic == exp_sig.magic) {
            // verify padding values, zero byte padding
            if(strncmp(sig.padding, exp_sig.padding, sizeof(sig.padding))) {
                // check newline byte padding
                memset(exp_sig.padding, 0x0a, 12);
                if(strncmp(sig.padding, exp_sig.padding, 12)) {
                    // check whitespace byte padding
                    memset(exp_sig.padding, 0x20, 12);
                    if(!strncmp(sig.padding, exp_sig.padding, 12))
                        is_pad = true;
                }

                else is_pad = true;
            }

            else is_pad = true;
        }

        if(!is_pad || sig.magic != exp_sig.magic) 
            m_error.Error(LIBDAS_ERROR_INVALID_SIGNATURE);
    }


    std::any DasReaderCore::ReadScopeData(DasScopeType _type) {
        std::any scope = _GetAnyScopeStructure(_type);
        std::string val_decl, val_statement;

        do {
            _SkipSkippableCharacters(true);
            char *beg = _GetReadPtr();
            char *end = _ExtractWord();
            _SetReadPtr(end);

            val_decl = std::string(beg, end - beg);

            // no more data in buffer chunk
            if(val_decl == "") {
                bool is_read = _ReadNewChunk();
                _SetReadPtr(m_buffer);
                if(!is_read)
                    m_error.Error(LIBDAS_ERROR_INCOMPLETE_SCOPE);

                _SkipSkippableCharacters(true);
                beg = _GetReadPtr();
                end = _ExtractWord();
                _SetReadPtr(end);
                val_decl = std::string(beg, end - beg);
            }

            val_statement = val_decl.substr(0, val_decl.size() - 1);

            // check if value declaration is present
            // in ReadProperties method there can be only single non-value statement "ENDSCOPE"
            if(val_decl[val_decl.size() - 1] != ':') {
                DasScopeType sub_scope = ParseScopeDeclaration(val_decl);
                if(sub_scope == LIBDAS_DAS_SCOPE_UNDEFINED && val_decl != "ENDSCOPE") 
                    m_error.Error(LIBDAS_ERROR_INVALID_DATA);
                else if(val_decl == "ENDSCOPE") 
                    break;
                else {
                    std::any sub_scope_val = ReadScopeData(sub_scope);
                    _VerifySubScope(_type, sub_scope, sub_scope_val, scope);
                }
            }
                
            std::any value_info = _GetValueInformation(_type, val_statement);
            
            // skip the whitespace following the declaration
            _SkipData(1);

            // data is in correct type thus read its value
            _ReadScopeValueDataCaller(scope, _type, value_info, val_statement);

        } while(_GetReadPtr() < m_buffer + m_buffer_size);

        return scope;
    }


    DasScopeType DasReaderCore::ParseScopeDeclaration(const std::string &_scope_str) {
        if(_scope_str == "") {
            _SkipSkippableCharacters(true);
            char *beg = _GetReadPtr();
            char *end = _ExtractWord();
            _SetReadPtr(end);

            std::string decl = std::string(beg, end - beg);
            if(decl == "") {
                bool is_read = _ReadNewChunk();
                _SetReadPtr(m_buffer);
                if(!is_read) return LIBDAS_DAS_SCOPE_END;

                _SkipSkippableCharacters(true);
                beg = _GetReadPtr();
                end = _ExtractWord();
                _SetReadPtr(end);
                decl = std::string(beg, end - beg);
            }

            if(m_scope_name_map.find(decl) == m_scope_name_map.end())
                return LIBDAS_DAS_SCOPE_UNDEFINED;

            return m_scope_name_map[decl];
        }

        else {
            if(_scope_str == "")
                return LIBDAS_DAS_SCOPE_END;

            else if(m_scope_name_map.find(_scope_str) == m_scope_name_map.end())
                return LIBDAS_DAS_SCOPE_UNDEFINED;

            return m_scope_name_map[_scope_str];
        }
    }
}
