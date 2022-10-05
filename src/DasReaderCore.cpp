// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: DasReaderCore.h - DAS file format reader class header
// author: Karl-Mihkel Ott

#define DAS_READER_CORE_CPP
#include "das/DasReaderCore.h"

namespace Libdas {

    DasReaderCore::DasReaderCore(const std::string &_file_name) : 
        MAR::AsciiLineReader(_file_name, DEFAULT_CHUNK, std::string("ENDSCOPE") + LIBDAS_DAS_NEWLINE), 
        m_error(MODEL_FORMAT_DAS) 
    {
        _CreateScopeNameMap();
        _CreateScopeValueTypeMap();
        _SetLineBounds(std::make_pair(m_buffer, m_buffer + m_buffer_size - 1));
    }


    DasReaderCore::DasReaderCore(DasReaderCore &&_drc) noexcept :
        MAR::AsciiLineReader(std::move(_drc)),
        m_error(std::move(_drc.m_error)),
        m_scope_name_map(std::move(_drc.m_scope_name_map)),
        m_unique_val_map(std::move(_drc.m_unique_val_map)),
        m_buffer_blobs(std::move(_drc.m_buffer_blobs)) {}


    DasReaderCore::~DasReaderCore() {
        for(char *buf : m_buffer_blobs)
            std::free(buf); 
    }


    void DasReaderCore::_CreateScopeNameMap() {
        m_scope_name_map["PROPERTIES"] = LIBDAS_DAS_SCOPE_PROPERTIES;
        m_scope_name_map["BUFFER"] = LIBDAS_DAS_SCOPE_BUFFER;
        m_scope_name_map["MORPHTARGET"] = LIBDAS_DAS_SCOPE_MORPH_TARGET;
        m_scope_name_map["MESHPRIMITIVE"] = LIBDAS_DAS_SCOPE_MESH_PRIMITIVE;
        m_scope_name_map["MESH"] = LIBDAS_DAS_SCOPE_MESH;
        m_scope_name_map["NODE"] = LIBDAS_DAS_SCOPE_NODE;
        m_scope_name_map["SCENE"] = LIBDAS_DAS_SCOPE_SCENE;
        m_scope_name_map["JOINT"] = LIBDAS_DAS_SCOPE_SKELETON_JOINT;
        m_scope_name_map["SKELETON"] = LIBDAS_DAS_SCOPE_SKELETON;
        m_scope_name_map["ANIMATION"] = LIBDAS_DAS_SCOPE_ANIMATION; 
        m_scope_name_map["ANIMATIONCHANNEL"] = LIBDAS_DAS_SCOPE_ANIMATION_CHANNEL;
    }


    void DasReaderCore::_CreateScopeValueTypeMap() {
        // PROPERTIES
        m_unique_val_map["MODEL"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_MODEL;
        m_unique_val_map["AUTHOR"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_AUTHOR;
        m_unique_val_map["COPYRIGHT"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_COPYRIGHT;
        m_unique_val_map["MODDATE"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_MODDATE;
        m_unique_val_map["DEFAULTSCENE"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_DEFAULT_SCENE;

        // BUFFER
        m_unique_val_map["BUFFERTYPE"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_BUFFER_TYPE;
        m_unique_val_map["DATALEN"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_DATA_LEN;
        m_unique_val_map["DATA"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_DATA;

        // MESH
        m_unique_val_map["PRIMITIVECOUNT"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_PRIMITIVE_COUNT;
        m_unique_val_map["PRIMITIVES"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_PRIMITIVES;

        // MESHPRIMITIVE
        m_unique_val_map["INDEXBUFFERID"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_INDEX_BUFFER_ID;
        m_unique_val_map["INDEXBUFFEROFFSET"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_INDEX_BUFFER_OFFSET;
        m_unique_val_map["INDICESCOUNT"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_INDICES_COUNT;
        m_unique_val_map["TEXTURECOUNT"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_TEXTURE_COUNT;
        m_unique_val_map["TEXTUREIDS"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_TEXTURE_IDS;
        m_unique_val_map["COLORMULCOUNT"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_COLOR_MUL_COUNT;
        m_unique_val_map["COLORMULBUFFERIDS"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_COLOR_MUL_BUFFER_IDS;
        m_unique_val_map["COLORMULBUFFEROFFSETS"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_COLOR_MUL_BUFFER_OFFSETS;
        m_unique_val_map["JOINTSETCOUNT"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_JOINT_SET_COUNT;
        m_unique_val_map["JOINTINDEXBUFFERIDS"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_JOINT_INDEX_BUFFER_IDS;
        m_unique_val_map["JOINTINDEXBUFFEROFFSETS"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_JOINT_INDEX_BUFFER_OFFSETS;
        m_unique_val_map["JOINTWEIGHTBUFFERIDS"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_JOINT_WEIGHT_BUFFER_IDS;
        m_unique_val_map["JOINTWEIGHTBUFFEROFFSETS"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_JOINT_WEIGHT_BUFFER_OFFSETS;
        m_unique_val_map["MORPHTARGETCOUNT"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_MORPH_TARGET_COUNT;
        m_unique_val_map["MORPHTARGETS"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_MORPH_TARGETS;
        m_unique_val_map["MORPHWEIGHTS"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_MORPH_WEIGHTS;

        // NODE
        m_unique_val_map["MESH"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_MESH;
        m_unique_val_map["SKELETON"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_SKELETON;

        // SCENE
        m_unique_val_map["NODECOUNT"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_NODE_COUNT;
        m_unique_val_map["NODES"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_NODES;

        // SKELETON
        m_unique_val_map["JOINTCOUNT"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_JOINT_COUNT;
        m_unique_val_map["JOINTS"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_JOINTS;

        // JOINT
        m_unique_val_map["INVERSEBINDPOS"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_INVERSE_BIND_POS;
        m_unique_val_map["PARENT"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_PARENT;
        m_unique_val_map["SCALE"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_SCALE;
        m_unique_val_map["ROTATION"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_ROTATION;
        m_unique_val_map["TRANSLATION"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_TRANSLATION;

        // ANIMATION
        m_unique_val_map["CHANNELCOUNT"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_CHANNEL_COUNT;
        m_unique_val_map["CHANNELS"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_CHANNELS;

        // ANIMATIONCHANNEL
        m_unique_val_map["NODEID"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_NODE_ID;
        m_unique_val_map["JOINTID"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_JOINT_ID;
        m_unique_val_map["TARGET"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_TARGET;
        m_unique_val_map["INTERPOLATION"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_INTERPOLATION;
        m_unique_val_map["KEYFRAMECOUNT"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_KEYFRAME_COUNT;
        m_unique_val_map["WEIGHTCOUNT"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_WEIGHT_COUNT;
        m_unique_val_map["KEYFRAMES"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_KEYFRAMES;
        m_unique_val_map["TANGENTS"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_TANGENTS;
        m_unique_val_map["TARGETVALUES"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_TARGET_VALUES;

        // Not so unique value types, since these values can be present in multiple scopes
        m_unique_val_map["NAME"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_NAME;
        m_unique_val_map["TRANSFORM"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_TRANSFORM;
        m_unique_val_map["VERTEXBUFFERID"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_BUFFER_ID;
        m_unique_val_map["VERTEXBUFFEROFFSET"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_BUFFER_OFFSET;
        m_unique_val_map["UVBUFFERIDS"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_UV_BUFFER_IDS;
        m_unique_val_map["UVBUFFEROFFSETS"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_UV_BUFFER_OFFSETS;
        m_unique_val_map["VERTEXNORMALBUFFERID"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_NORMAL_BUFFER_ID;
        m_unique_val_map["VERTEXNORMALBUFFEROFFSET"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_NORMAL_BUFFER_OFFSET;
        m_unique_val_map["VERTEXTANGENTBUFFERID"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_TANGENT_BUFFER_ID;
        m_unique_val_map["VERTEXTANGENTBUFFEROFFSET"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_TANGENT_BUFFER_OFFSET;
        m_unique_val_map["CHILDRENCOUNT"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_CHILDREN_COUNT;
        m_unique_val_map["CHILDREN"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_CHILDREN;
    }


    DasUniqueValueType DasReaderCore::_FindUniqueValueType(const std::string &_value) {
        if(m_unique_val_map.find(_value) == m_unique_val_map.end())
            return LIBDAS_DAS_UNIQUE_VALUE_TYPE_UNKNOWN;

        return m_unique_val_map[_value];
    }


    void DasReaderCore::_ReadPropertiesValue(DasProperties *_props, DasUniqueValueType _type) {
        switch(_type) {
            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_MODEL:
                _props->model = _ExtractString();
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_AUTHOR:
                _props->author = _ExtractString();
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_COPYRIGHT:
                _props->copyright = _ExtractString();
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_MODDATE:
                _props->moddate = *reinterpret_cast<uint64_t*>(_GetReadPtr());
                _SkipData(sizeof(uint64_t));
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_DEFAULT_SCENE:
                _props->default_scene = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _SkipData(sizeof(uint32_t));
                break;

            default:
                LIBDAS_ASSERT(false);
                break;
        }
    }


    void DasReaderCore::_ReadBufferValue(DasBuffer *_buffer, DasUniqueValueType _type) {
        switch(_type) {
            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_BUFFER_TYPE:
                _buffer->type = *reinterpret_cast<BufferType*>(_GetReadPtr());
                _SkipData(sizeof(BufferType));
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_DATA_LEN:
                _buffer->data_len = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _SkipData(sizeof(uint32_t));
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_DATA:
                _buffer->data_ptrs.push_back(std::make_pair(_ExtractBlob(_buffer->data_len), _buffer->data_len));

                // check if blob reading was successful
                if(!_buffer->data_ptrs.back().first)
                    m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                m_buffer_blobs.push_back(const_cast<char*>(_buffer->data_ptrs.back().first));
                break;

            default:
                LIBDAS_ASSERT(false);
                break;
        }
    }


    void DasReaderCore::_ReadMeshPrimitiveValue(DasMeshPrimitive *_primitive, DasUniqueValueType _type) {
        switch(_type) {
            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_INDEX_BUFFER_ID:
                _primitive->index_buffer_id = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_INDEX_BUFFER_OFFSET:
                _primitive->index_buffer_offset = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_INDICES_COUNT:
                _primitive->indices_count = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_BUFFER_ID:
                _primitive->vertex_buffer_id = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_BUFFER_OFFSET:
                _primitive->vertex_buffer_offset = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_NORMAL_BUFFER_ID:
                _primitive->vertex_normal_buffer_id = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_NORMAL_BUFFER_OFFSET:
                _primitive->vertex_normal_buffer_offset = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_TANGENT_BUFFER_ID:
                _primitive->vertex_tangent_buffer_id = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_TANGENT_BUFFER_OFFSET:
                _primitive->vertex_tangent_buffer_offset = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_TEXTURE_COUNT:
                _primitive->texture_count = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);

                // allocate memory for uv buffer ids and offsets
                if(_primitive->texture_count) {
                    _primitive->uv_buffer_ids = new uint32_t[_primitive->texture_count];
                    _primitive->uv_buffer_offsets = new uint32_t[_primitive->texture_count];
                }
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_UV_BUFFER_IDS:
                for(uint32_t i = 0; i < _primitive->texture_count; i++) {
                    _primitive->uv_buffer_ids[i] = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                    if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                }
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_UV_BUFFER_OFFSETS:
                for(uint32_t i = 0; i < _primitive->texture_count; i++) {
                    _primitive->uv_buffer_offsets[i] = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                    if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                }
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_TEXTURE_IDS:
                // allocate memory for texture ids
                if(_primitive->texture_count) {
                    _primitive->texture_ids = new uint32_t[_primitive->texture_count];
                    
                    for(uint32_t i = 0; i < _primitive->texture_count; i++) {
                        _primitive->texture_ids[i] = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                        if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                    }
                }
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_COLOR_MUL_COUNT:
                _primitive->color_mul_count = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);

                // allocate enough memory for buffer ids / offsets
                if(_primitive->color_mul_count) {
                    _primitive->color_mul_buffer_ids = new uint32_t[_primitive->color_mul_count];
                    _primitive->color_mul_buffer_offsets = new uint32_t[_primitive->color_mul_count];
                }
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_COLOR_MUL_BUFFER_IDS:
                for(uint32_t i = 0; i < _primitive->color_mul_count; i++) {
                    _primitive->color_mul_buffer_ids[i] = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                    if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                }
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_COLOR_MUL_BUFFER_OFFSETS:
                for(uint32_t i = 0; i < _primitive->color_mul_count; i++) {
                    _primitive->color_mul_buffer_offsets[i] = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                    if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                }
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_JOINT_SET_COUNT:
                _primitive->joint_set_count = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);

                // allocate memory for index and weight buffer ids / offsets
                if(_primitive->joint_set_count) {
                    _primitive->joint_index_buffer_ids = new uint32_t[_primitive->joint_set_count];
                    _primitive->joint_index_buffer_offsets = new uint32_t[_primitive->joint_set_count];
                    _primitive->joint_weight_buffer_ids = new uint32_t[_primitive->joint_set_count];
                    _primitive->joint_weight_buffer_offsets = new uint32_t[_primitive->joint_set_count];
                }
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_JOINT_INDEX_BUFFER_IDS:
                for(uint32_t i = 0; i < _primitive->joint_set_count; i++) {
                    _primitive->joint_index_buffer_ids[i] = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                    if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                }
                break;
                
            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_JOINT_INDEX_BUFFER_OFFSETS:
                for(uint32_t i = 0; i < _primitive->joint_set_count; i++) {
                    _primitive->joint_index_buffer_offsets[i] = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                    if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                }
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_JOINT_WEIGHT_BUFFER_IDS:
                for(uint32_t i = 0; i < _primitive->joint_set_count; i++) {
                    _primitive->joint_weight_buffer_ids[i] = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                    if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                }
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_JOINT_WEIGHT_BUFFER_OFFSETS:
                for(uint32_t i = 0; i < _primitive->joint_set_count; i++) {
                    _primitive->joint_weight_buffer_offsets[i] = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                    if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                }
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_MORPH_TARGET_COUNT:
                _primitive->morph_target_count = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);

                // allocate memory for morph target references
                if(_primitive->morph_target_count) {
                    _primitive->morph_targets = new uint32_t[_primitive->morph_target_count];
                    _primitive->morph_weights = new float[_primitive->morph_target_count];
                }
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_MORPH_TARGETS:
                for(uint32_t i = 0; i < _primitive->morph_target_count; i++) {
                    _primitive->morph_targets[i] = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                    if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                }
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_MORPH_WEIGHTS:
                for(uint32_t i = 0; i < _primitive->morph_target_count; i++) {
                    _primitive->morph_weights[i] = *reinterpret_cast<float*>(_GetReadPtr());
                    if(!_SkipData(sizeof(float))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                }
                break;

            default:
                LIBDAS_ASSERT(false);
                break;
        }
    }


    void DasReaderCore::_ReadMorphTargetValue(DasMorphTarget *_morph_target, DasUniqueValueType _type) {
        switch(_type) {
            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_BUFFER_ID:
                {
                    _morph_target->vertex_buffer_id = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                    if(!_SkipData(sizeof(uint32_t))) {
                        m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                    }
                }
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_BUFFER_OFFSET:
                _morph_target->vertex_buffer_offset = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_TEXTURE_COUNT:
                _morph_target->texture_count = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);

                // allocate memory for buffer ids and offsets
                if(_morph_target->texture_count) {
                    _morph_target->uv_buffer_ids = new uint32_t[_morph_target->texture_count];
                    _morph_target->uv_buffer_offsets = new uint32_t[_morph_target->texture_count];
                }
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_UV_BUFFER_IDS:
                for(uint32_t i = 0; i < _morph_target->texture_count; i++) {
                    _morph_target->uv_buffer_ids[i] = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                    if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                }
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_UV_BUFFER_OFFSETS:
                for(uint32_t i = 0; i < _morph_target->texture_count; i++) {
                    _morph_target->uv_buffer_offsets[i] = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                    if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                }
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_COLOR_MUL_COUNT:
                _morph_target->color_mul_count = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);

                // allocate memory for buffer ids and offsets
                if(_morph_target->color_mul_count) {
                    _morph_target->color_mul_buffer_ids = new uint32_t[_morph_target->color_mul_count];
                    _morph_target->color_mul_buffer_offsets = new uint32_t[_morph_target->color_mul_count];
                }
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_COLOR_MUL_BUFFER_IDS:
                for(uint32_t i = 0; i < _morph_target->color_mul_count; i++) {
                    _morph_target->color_mul_buffer_ids[i] = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                    if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                }
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_COLOR_MUL_BUFFER_OFFSETS:
                for(uint32_t i = 0; i < _morph_target->color_mul_count; i++) {
                    _morph_target->color_mul_buffer_offsets[i] = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                    if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                }
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_NORMAL_BUFFER_ID:
                _morph_target->vertex_normal_buffer_id = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_NORMAL_BUFFER_OFFSET:
                _morph_target->vertex_normal_buffer_offset = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_TANGENT_BUFFER_ID:
                _morph_target->vertex_tangent_buffer_id = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_TANGENT_BUFFER_OFFSET:
                _morph_target->vertex_tangent_buffer_offset = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                break;

            default:
                LIBDAS_ASSERT(false);
                break;

        }
    }


    void DasReaderCore::_ReadMeshValue(DasMesh *_mesh, DasUniqueValueType _type) {
        switch(_type) {
            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_NAME:
                _mesh->name = _ExtractString();
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_PRIMITIVE_COUNT:
                _mesh->primitive_count = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);

                // allocate memory for primitive references
                _mesh->primitives = new uint32_t[_mesh->primitive_count];
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_PRIMITIVES:
                for(uint32_t i = 0; i < _mesh->primitive_count; i++) {
                    _mesh->primitives[i] = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                    if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                }
                break;

            default:
                LIBDAS_ASSERT(false);
                break;
        }
    }


    void DasReaderCore::_ReadNodeValue(DasNode *_node, DasUniqueValueType _type) {
        switch(_type) {
            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_NAME:
                _node->name = _ExtractString();
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_CHILDREN_COUNT:
                _node->children_count = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _SkipData(sizeof(uint32_t));

                // allocate memory for children
                _node->children = new uint32_t[_node->children_count];
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_CHILDREN:
                for(uint32_t i = 0; i < _node->children_count; i++) {
                    _node->children[i] = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                    if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                }
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_MESH:
                _node->mesh = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_SKELETON:
                _node->skeleton = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_TRANSFORM:
                for(struct { TRS::Matrix4<float>::iterator it; int i; } s = {_node->transform.BeginRowMajor(), 0}; s.it != _node->transform.EndRowMajor(); s.it++, s.i++)
                    *s.it = reinterpret_cast<float*>(_GetReadPtr())[s.i];

                _SkipData(sizeof(float[16]));
                break;

            default:
                LIBDAS_ASSERT(false);
                break;
        }
    }


    void DasReaderCore::_ReadSceneValue(DasScene *_scene, DasUniqueValueType _type) {
        switch(_type) {
            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_NAME:
                _scene->name = _ExtractString();
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_NODE_COUNT:
                _scene->node_count = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _SkipData(sizeof(uint32_t));

                // allocate memory for scene nodes
                _scene->nodes = new uint32_t[_scene->node_count];
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_NODES:
                for(uint32_t i = 0; i < _scene->node_count; i++) {
                    _scene->nodes[i] = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                    if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                }
                break;

            default:
                LIBDAS_ASSERT(false);
                break;
        }
    }


    void DasReaderCore::_ReadSkeletonValue(DasSkeleton *_skeleton, DasUniqueValueType _type) {
        switch(_type) {
            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_NAME:
                _skeleton->name = _ExtractString();
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_PARENT:
                _skeleton->parent = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_JOINT_COUNT:
                _skeleton->joint_count = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);

                // allocate memory for skeletons
                _skeleton->joints = new uint32_t[_skeleton->joint_count];
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_JOINTS:
                for(uint32_t i = 0; i < _skeleton->joint_count; i++) {
                    _skeleton->joints[i] = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                    if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                }
                break;

            default:
                break;
        }
    }


    void DasReaderCore::_ReadSkeletonJointValue(DasSkeletonJoint *_joint, DasUniqueValueType _type) {
        switch(_type) {
            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_INVERSE_BIND_POS:
                for(struct { TRS::Matrix4<float>::iterator it; int i; } s = {_joint->inverse_bind_pos.BeginRowMajor(), 0}; s.it != _joint->inverse_bind_pos.EndRowMajor(); s.it++, s.i++)
                    *s.it = reinterpret_cast<float*>(_GetReadPtr())[s.i];
                _SkipData(sizeof(float[16]));
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_NAME:
                _joint->name = _ExtractString();
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_CHILDREN_COUNT:
                _joint->children_count = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);

                // allocate memory for children
                _joint->children = new uint32_t[_joint->children_count];
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_CHILDREN:
                for(uint32_t i = 0; i < _joint->children_count; i++) {
                    _joint->children[i] = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                    if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                }
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_SCALE:
                _joint->scale = *reinterpret_cast<float*>(_GetReadPtr());
                _SkipData(sizeof(float));
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_ROTATION:
                _joint->rotation = *reinterpret_cast<TRS::Quaternion*>(_GetReadPtr());
                if(!_SkipData(sizeof(TRS::Quaternion))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_TRANSLATION:
                _joint->translation = *reinterpret_cast<TRS::Point3D<float>*>(_GetReadPtr());
                if(!_SkipData(sizeof(TRS::Point3D<float>))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                break;

            default:
                break;
        }
    }


    void DasReaderCore::_ReadAnimationValue(DasAnimation *_animation, DasUniqueValueType _type) {
        switch(_type) {
            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_NAME:
                _animation->name = _ExtractString();
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_CHANNEL_COUNT:
                _animation->channel_count = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);

                // allocate memory for channels
                _animation->channels = new uint32_t[_animation->channel_count];
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_CHANNELS:
                for(uint32_t i = 0; i < _animation->channel_count; i++) {
                    _animation->channels[i] = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                    if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                }
                break;

            default:
                LIBDAS_ASSERT(false);
                break;
        }
    }


    void DasReaderCore::_ReadAnimationChannelValue(DasAnimationChannel *_channel, DasUniqueValueType _type) {
        switch(_type) {
            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_NODE_ID:
                _channel->node_id = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_JOINT_ID:
                _channel->joint_id = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_TARGET:
                _channel->target = *reinterpret_cast<AnimationTarget*>(_GetReadPtr());
                if(!_SkipData(sizeof(AnimationTarget))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_INTERPOLATION:
                _channel->interpolation = *reinterpret_cast<InterpolationType*>(_GetReadPtr());
                if(!_SkipData(sizeof(InterpolationType))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_KEYFRAME_COUNT:
                _channel->keyframe_count = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);

                // allocate memory for keyframe data
                LIBDAS_ASSERT(_channel->keyframe_count);
                if(_channel->keyframe_count) {
                    _channel->keyframes = new float[_channel->keyframe_count];
                    uint32_t type_stride = 0;
                    switch(_channel->target) {
                        case LIBDAS_ANIMATION_TARGET_TRANSLATION:
                            type_stride = static_cast<uint32_t>(sizeof(TRS::Vector3<float>));
                            break;

                        case LIBDAS_ANIMATION_TARGET_ROTATION:
                            type_stride = static_cast<uint32_t>(sizeof(TRS::Quaternion));
                            break;

                        case LIBDAS_ANIMATION_TARGET_SCALE:
                            type_stride = static_cast<uint32_t>(sizeof(float));
                            break;

                        default:
                            break;
                    }

                    if(type_stride) {
                        if(_channel->interpolation == LIBDAS_INTERPOLATION_VALUE_CUBICSPLINE)
                            _channel->tangents = new char[type_stride * _channel->keyframe_count * 2];
                        _channel->target_values = new char[type_stride * _channel->keyframe_count];
                    }
                }
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_WEIGHT_COUNT:
                _channel->weight_count = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_KEYFRAMES:
                for(uint32_t i = 0; i < _channel->keyframe_count; i++) {
                    _channel->keyframes[i] = *reinterpret_cast<float*>(_GetReadPtr());
                    if(!_SkipData(sizeof(float))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                }
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_TANGENTS:
                if(_channel->keyframe_count) {
                    uint32_t type_stride = 0;
                    switch(_channel->target) {
                        case LIBDAS_ANIMATION_TARGET_WEIGHTS:
                            type_stride = static_cast<uint32_t>(sizeof(float)) * _channel->weight_count;

                            // allocate memory for tangents 
                            _channel->tangents = new char[type_stride * 2 * _channel->keyframe_count];
                            break;

                        case LIBDAS_ANIMATION_TARGET_TRANSLATION:
                            type_stride = static_cast<uint32_t>(sizeof(TRS::Vector3<float>));
                            break;

                        case LIBDAS_ANIMATION_TARGET_ROTATION:
                            type_stride = static_cast<uint32_t>(sizeof(TRS::Quaternion));
                            break;

                        case LIBDAS_ANIMATION_TARGET_SCALE:
                            type_stride = static_cast<uint32_t>(sizeof(float));
                            break;

                        default:
                            LIBDAS_ASSERT(false);
                            break;
                    }

                    for(uint32_t i = 0; i < type_stride * 2 * _channel->keyframe_count; i++) {
                        _channel->tangents[i] = *_GetReadPtr();
                        if(!_SkipData(sizeof(char))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                    }
                }
                break;

            case LIBDAS_DAS_UNIQUE_VALUE_TYPE_TARGET_VALUES:
                if(_channel->keyframe_count) {
                    uint32_t type_stride = 0;
                    switch(_channel->target) {
                        case LIBDAS_ANIMATION_TARGET_WEIGHTS:
                            type_stride = static_cast<uint32_t>(sizeof(float)) * _channel->weight_count;

                            // allocate memory for target values 
                            _channel->target_values = new char[type_stride * _channel->keyframe_count];
                            break;

                        case LIBDAS_ANIMATION_TARGET_TRANSLATION:
                            type_stride = static_cast<uint32_t>(sizeof(TRS::Vector3<float>));
                            break;

                        case LIBDAS_ANIMATION_TARGET_ROTATION:
                            type_stride = static_cast<uint32_t>(sizeof(TRS::Quaternion));
                            break;

                        case LIBDAS_ANIMATION_TARGET_SCALE:
                            type_stride = static_cast<uint32_t>(sizeof(float));
                            break;

                        default:
                            LIBDAS_ASSERT(false);
                            break;
                    }

                    for(uint32_t i = 0; i < type_stride * _channel->keyframe_count; i++) {
                        _channel->target_values[i] = *_GetReadPtr();
                        if(!_SkipData(sizeof(char))) m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                    }
                }
                break;

            default:
                LIBDAS_ASSERT(false);
                break;
        }
    }


    void DasReaderCore::_ReadScopeValueDataCaller(std::any &_scope, DasScopeType _type, DasUniqueValueType _value_type, const std::string &_val_str) {
        // any type check is necessary for correct error output
        switch(_type) {
            case LIBDAS_DAS_SCOPE_PROPERTIES:
                _ReadPropertiesValue(std::any_cast<DasProperties>(&_scope), _value_type);
                break;

            case LIBDAS_DAS_SCOPE_BUFFER:
                _ReadBufferValue(std::any_cast<DasBuffer>(&_scope), _value_type);
                break;

            case LIBDAS_DAS_SCOPE_MESH_PRIMITIVE:
                _ReadMeshPrimitiveValue(std::any_cast<DasMeshPrimitive>(&_scope), _value_type);
                break;

            case LIBDAS_DAS_SCOPE_MORPH_TARGET:
                _ReadMorphTargetValue(std::any_cast<DasMorphTarget>(&_scope), _value_type);
                break;

            case LIBDAS_DAS_SCOPE_MESH:
                _ReadMeshValue(std::any_cast<DasMesh>(&_scope), _value_type);
                break;

            case LIBDAS_DAS_SCOPE_NODE:
                _ReadNodeValue(std::any_cast<DasNode>(&_scope), _value_type);
                break;

            case LIBDAS_DAS_SCOPE_SCENE:
                _ReadSceneValue(std::any_cast<DasScene>(&_scope), _value_type);
                break;

            case LIBDAS_DAS_SCOPE_SKELETON:
                _ReadSkeletonValue(std::any_cast<DasSkeleton>(&_scope), _value_type);
                break;

            case LIBDAS_DAS_SCOPE_SKELETON_JOINT:
                _ReadSkeletonJointValue(std::any_cast<DasSkeletonJoint>(&_scope), _value_type);
                break;

            case LIBDAS_DAS_SCOPE_ANIMATION:
                _ReadAnimationValue(std::any_cast<DasAnimation>(&_scope), _value_type);
                break;

            case LIBDAS_DAS_SCOPE_ANIMATION_CHANNEL:
                _ReadAnimationChannelValue(std::any_cast<DasAnimationChannel>(&_scope), _value_type);
                break;

            default:
                LIBDAS_ASSERT(false);
                break;
        }
    }


    inline std::any DasReaderCore::_GetAnyScopeStructure(DasScopeType _type) {
        switch(_type) {
            case LIBDAS_DAS_SCOPE_PROPERTIES:
                return std::any(DasProperties());

            case LIBDAS_DAS_SCOPE_BUFFER:
                return std::any(DasBuffer());

            case LIBDAS_DAS_SCOPE_MORPH_TARGET:
                return std::any(DasMorphTarget());

            case LIBDAS_DAS_SCOPE_MESH_PRIMITIVE:
                return std::any(DasMeshPrimitive());

            case LIBDAS_DAS_SCOPE_MESH:
                return std::any(DasMesh());

            case LIBDAS_DAS_SCOPE_SCENE:
                return std::any(DasScene());

            case LIBDAS_DAS_SCOPE_NODE:
                return std::any(DasNode());

            case LIBDAS_DAS_SCOPE_SKELETON:
                return std::any(DasSkeleton());

            case LIBDAS_DAS_SCOPE_SKELETON_JOINT:
                return std::any(DasSkeletonJoint());

            case LIBDAS_DAS_SCOPE_ANIMATION:
                return std::any(DasAnimation());

            case LIBDAS_DAS_SCOPE_ANIMATION_CHANNEL:
                return std::any(DasAnimationChannel());

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
        std::any scope(_GetAnyScopeStructure(_type));
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
                    m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                else if(val_decl == "ENDSCOPE") 
                    break;
                else 
                    m_error.Error(LIBDAS_ERROR_INVALID_DATA_LENGTH);
            }
                
            // skip the whitespace following the declaration
            _SkipData(1);

            DasUniqueValueType val = _FindUniqueValueType(val_statement);
            
            // data is in correct format type thus read its value
            _ReadScopeValueDataCaller(scope, _type, val, val_statement);

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


    void DasReaderCore::Clear() {
        CloseFile();
        m_scope_name_map.clear();
        m_unique_val_map.clear();
    }
}
