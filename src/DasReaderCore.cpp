/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: DasReaderCore.h - DAS file format reader class header
/// author: Karl-Mihkel Ott

#define DAS_READER_CORE_CPP
#include <DasReaderCore.h>

namespace Libdas {

    DasReaderCore::DasReaderCore(const std::string &_file_name) : 
        AsciiLineReader(_file_name), m_file_name(_file_name), m_error(MODEL_FORMAT_DAS) 
    {
        // open file stream if file name was specified
        if(m_file_name != "")
            m_in_stream.open(m_file_name, std::ios_base::binary);

        _CreateScopeNameMap();
    }


    void DasReaderCore::_CreateScopeNameMap() {
        m_scope_name_map["PROPERTIES"] = LIBDAS_DAS_SCOPE_PROPERTIES;
        m_scope_name_map["MODEL"] = LIBDAS_DAS_SCOPE_MODEL;
        m_scope_name_map["ANIMATION"] = LIBDAS_DAS_SCOPE_ANIMATION;
        m_scope_name_map["KEYFRAME"] = LIBDAS_DAS_SCOPE_KEYFRAME;
        m_scope_name_map["SCENE"] = LIBDAS_DAS_SCOPE_SCENE;
        m_scope_name_map["NODE"] = LIBDAS_DAS_SCOPE_SCENE_NODE;
    }


    void DasReaderCore::_CreateScopeValueTypeMap() {
        // PROPERTIES 
        m_unique_val_map["MODEL"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_MODEL;
        m_unique_val_map["MODDATE"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_MODDATE;
        m_unique_val_map["COMPRESSION"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_COMPRESSION;

        // BUFFER
        m_unique_val_map["BUFFERTYPE"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_BUFFER_TYPE;
        m_unique_val_map["DATALEN"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_DATA_LEN;

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
        m_unique_val_map["TRANSFORM"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_TRANSFORM;
        m_unique_val_map["VERTEXBUFFERID"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_BUFFER_ID;
        m_unique_val_map["VERTEXBUFFEROFFSET"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_BUFFER_OFFSET;
        m_unique_val_map["TEXTUREMAPBUFFERID"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_TEXTURE_MAP_BUFFER_ID;
        m_unique_val_map["TEXTUREMAPBUFFEROFFSET"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_TEXTURE_MAP_BUFFER_OFFSET;
        m_unique_val_map["VERTEXNORMALBUFFERID"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_NORMAL_BUFFER_ID;
        m_unique_val_map["VERTEXNORMALOFFSET"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_NORMAL_BUFFER_OFFSET;
    }


    DasScopeType DasReaderCore::_FindScopeType() {
        SkipSkippableCharacters(true);
        char *beg = GetReadPtr();
        char *end = ExtractWord();

        std::string scope_name = std::string(beg, end - beg);

        // check if the element with key scope_name was not found 
        if(m_scope_name_map.find(scope_name) == m_scope_name_map.end())
            return LIBDAS_DAS_SCOPE_SCENE_UNDEFINED;

        return m_scope_name_map[scope_name];
    }


    DasUniqueValueType DasReaderCore::_FindUniqueValueType(const std::string &_value) {
        if(m_unique_val_map.find(_value) == m_unique_val_map.end())
            return LIBDAS_DAS_UNIQUE_VALUE_TYPE_UNKNOWN;

        return m_unique_val_map[_value];
    }


    void DasReaderCore::NewFile(const std::string &_file_name) {
        LIBDAS_ASSERT(_file_name != "");

        // close the previous stream if it was opened
        CloseStream();

        // open the file stream
        m_in_stream.open(_file_name, std::ios_base::binary);
    }


    void DasReaderCore::CloseStream() {
        if(m_in_stream.is_open())
            m_in_stream.close();
    }


    void DasReaderCore::ReadSignature() {
        DasSignature exp_sig;
        DasSignature sig;

        m_in_stream.read(reinterpret_cast<char*>(&sig), sizeof(DasSignature));
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

        if(is_pad != exp_sig.magic) m_error.Error(LIBDAS_ERROR_INVALID_SIGNATURE);
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


    void DasReaderCore::_ReadPropertiesValue(DasProperties &_props, DasProperties::ValueType _type) {
        switch(_type) {
            case DasProperties::LIBDAS_PROPERTIES_MODEL: 
                _props.model = ExtractString();
                break;

            case DasProperties::LIBDAS_PROPERTIES_AUTHOR:
                _props.author = ExtractString();
                break;

            case DasProperties::LIBDAS_PROPERTIES_COPYRIGHT:
                _props.copyright = ExtractString();
                break;

            case DasProperties::LIBDAS_PROPERTIES_MODDATE:
                _props.moddate = *reinterpret_cast<uint64_t*>(GetReadPtr());
                SkipData(sizeof(uint64_t));
                break;

            case DasProperties::LIBDAS_PROPERTIES_COMPRESSION:
                _props.compression = reinterpret_cast<bool*>(GetReadPtr());
                SkipData(sizeof(bool));
                break;

            default:
                LIBDAS_ASSERT(false);
        }
    }


    void DasReaderCore::_ReadBufferValue(DasBuffer &_buffer, DasBuffer::ValueType _type) {
        switch(_type) {
            case DasBuffer::LIBDAS_BUFFER_BUFFER_TYPE:
                _buffer.type = *reinterpret_cast<BufferType*>(GetReadPtr());
                break;
        }
    }


    DasProperties DasReaderCore::ReadProperties() {
        DasProperties props;
        std::string val_decl, val_statement;

        do {
            SkipSkippableCharacters();
            char *beg = GetReadPtr(), *end = ExtractWord();
            val_decl = std::string(beg, end - beg);
            val_statement = val_decl.substr(0, val_decl.size() - 1);

            // check if value declaration is present
            // in ReadProperties method there can be only single non-value statement "ENDSCOPE"
            if(val_decl[val_decl.size() - 1] != ':') {
                if(val_decl != "ENDSCOPE")
                    m_error.Error(LIBDAS_ERROR_INVALID_DATA);
            }
                
            std::pair<std::any, size_t> value_info = _GetValueInformation(LIBDAS_DAS_SCOPE_PROPERTIES, val_statement);

            // check if expected value type was not found
            if(value_info.first.type() != typeid(DasProperties::ValueType))
                m_error.Error(LIBDAS_ERROR_INVALID_DATA);
            
            // skip the whitespace following the declaration
            SkipData(1);

            // data is in correct type thus read its value
            if(value_info.first.type() == typeid(DasUniqueValueType))
                _ReadPropertyValue(props, std::any_cast<DasProperties::ValueType>(value_info.first));

        } while(val_decl != "ENDSCOPE");

        return DasProperties();
    }
}
