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
        m_scope_name_map["MESH"] = LIBDAS_DAS_SCOPE_MESH;
        m_scope_name_map["NODE"] = LIBDAS_DAS_SCOPE_NODE;
        m_scope_name_map["SCENE"] = LIBDAS_DAS_SCOPE_SCENE;
        m_scope_name_map["JOINT"] = LIBDAS_DAS_SCOPE_SKELETON_JOINT;
        m_scope_name_map["SKELETON"] = LIBDAS_DAS_SCOPE_SKELETON;
        m_scope_name_map["ANIMATION"] = LIBDAS_DAS_SCOPE_ANIMATION; }


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
        m_unique_val_map["INDEXBUFFERID"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_INDEX_BUFFER_ID;
        m_unique_val_map["INDEXBUFFEROFFSET"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_INDEX_BUFFER_OFFSET;
        m_unique_val_map["INDICESCOUNT"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_INDICES_COUNT;
        m_unique_val_map["TEXTUREID"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_TEXTURE_ID;
        m_unique_val_map["VERTEXBUFFERID"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_BUFFER_ID;
        m_unique_val_map["VERTEXBUFFEROFFSET"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_BUFFER_OFFSET;
        m_unique_val_map["TEXTUREMAPBUFFERID"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_TEXTURE_MAP_BUFFER_ID;
        m_unique_val_map["TEXTUREMAPBUFFEROFFSET"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_TEXTURE_MAP_BUFFER_OFFSET;
        m_unique_val_map["VERTEXNORMALBUFFERID"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_NORMAL_BUFFER_ID;
        m_unique_val_map["VERTEXNORMALBUFFEROFFSET"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_NORMAL_BUFFER_OFFSET;
        m_unique_val_map["VERTEXTANGENTBUFFERID"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_TANGENT_BUFFER_ID;
        m_unique_val_map["VERTEXTANGENTBUFFEROFFSET"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_TANGENT_BUFFER_OFFSET;

        // NODE
        m_unique_val_map["CHILDRENCOUNT"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_CHILDREN_COUNT;
        m_unique_val_map["CHILDREN"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_CHILDREN;
        m_unique_val_map["MESHCOUNT"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_MESH_COUNT;
        m_unique_val_map["MESHES"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_MESHES;
        m_unique_val_map["ANIMATIONCOUNT"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_ANIMATION_COUNT;
        m_unique_val_map["ANIMATIONS"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_ANIMATIONS;
        m_unique_val_map["SKELETONCOUNT"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_SKELETON_COUNT;
        m_unique_val_map["SKELETONS"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_SKELETONS;

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
        m_unique_val_map["NODEID"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_NODE_ID;
        m_unique_val_map["DURATION"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_DURATION;
        m_unique_val_map["KEYFRAMECOUNT"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_KEYFRAME_COUNT;
        m_unique_val_map["KEYFRAMETIMESTAMPS"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_KEYFRAME_TIMESTAMPS;
        m_unique_val_map["INTERPOLATIONTYPES"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_INTERPOLATION_TYPES;
        m_unique_val_map["ANIMATIONTARGETS"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_ANIMATION_TARGETS;
        m_unique_val_map["KEYFRAMEBUFFERID"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_KEYFRAME_BUFFER_ID;
        m_unique_val_map["KEYFRAMEBUFFEROFFSET"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_KEYFRAME_BUFFER_OFFSET;

        // Not so unique value types, since these values can be present in multiple scopes
        m_unique_val_map["NAME"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_NAME;
        m_unique_val_map["TRANSFORM"] = LIBDAS_DAS_UNIQUE_VALUE_TYPE_TRANSFORM;
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

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_DEFAULT_SCENE:
                        return DasProperties::LIBDAS_PROPERTIES_DEFAULT_SCENE;

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

            case LIBDAS_DAS_SCOPE_MESH:
                switch(type) {
                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_NAME:
                        return DasMesh::LIBDAS_MESH_NAME;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_INDEX_BUFFER_ID:
                        return DasMesh::LIBDAS_MESH_INDEX_BUFFER_ID;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_INDEX_BUFFER_OFFSET:
                        return DasMesh::LIBDAS_MESH_INDEX_BUFFER_OFFSET;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_INDICES_COUNT:
                        return DasMesh::LIBDAS_MESH_INDICES_COUNT;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_BUFFER_ID:
                        return DasMesh::LIBDAS_MESH_VERTEX_BUFFER_ID;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_BUFFER_OFFSET:
                        return DasMesh::LIBDAS_MESH_VERTEX_BUFFER_OFFSET;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_TEXTURE_ID:
                        return DasMesh::LIBDAS_MESH_TEXTURE_ID;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_TEXTURE_MAP_BUFFER_ID:
                        return DasMesh::LIBDAS_MESH_TEXTURE_MAP_BUFFER_ID;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_TEXTURE_MAP_BUFFER_OFFSET:
                        return DasMesh::LIBDAS_MESH_TEXTURE_MAP_BUFFER_OFFSET;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_NORMAL_BUFFER_ID:
                        return DasMesh::LIBDAS_MESH_VERTEX_NORMAL_BUFFER_ID;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_NORMAL_BUFFER_OFFSET:
                        return DasMesh::LIBDAS_MESH_VERTEX_NORMAL_BUFFER_OFFSET;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_TANGENT_BUFFER_ID:
                        return DasMesh::LIBDAS_MESH_VERTEX_TANGENT_BUFFER_ID;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_TANGENT_BUFFER_OFFSET:
                        return DasMesh::LIBDAS_MESH_VERTEX_TANGENT_BUFFER_OFFSET;

                    default:
                        return LIBDAS_DAS_UNIQUE_VALUE_TYPE_UNKNOWN;
                }
                break;

            case LIBDAS_DAS_SCOPE_NODE:
                switch(type) {
                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_NAME:
                        return DasNode::LIBDAS_NODE_NAME;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_CHILDREN_COUNT:
                        return DasNode::LIBDAS_NODE_CHILDREN_COUNT;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_CHILDREN:
                        return DasNode::LIBDAS_NODE_CHILDREN;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_MESH_COUNT:
                        return DasNode::LIBDAS_NODE_MESH_COUNT;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_MESHES:
                        return DasNode::LIBDAS_NODE_MESHES;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_ANIMATION_COUNT:
                        return DasNode::LIBDAS_NODE_ANIMATION_COUNT;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_ANIMATIONS:
                        return DasNode::LIBDAS_NODE_ANIMATIONS;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_SKELETON_COUNT:
                        return DasNode::LIBDAS_NODE_SKELETON_COUNT;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_SKELETONS:
                        return DasNode::LIBDAS_NODE_SKELETONS;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_TRANSFORM:
                        return DasNode::LIBDAS_NODE_TRANSFORM;

                    default:
                        return LIBDAS_DAS_UNIQUE_VALUE_TYPE_UNKNOWN;
                }
                break;

            case LIBDAS_DAS_SCOPE_SCENE:
                switch(type) {
                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_NAME:
                        return DasScene::LIBDAS_SCENE_NAME;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_NODE_COUNT:
                        return DasScene::LIBDAS_SCENE_NODE_COUNT;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_NODES:
                        return DasScene::LIBDAS_SCENE_NODES;

                    default:
                        return LIBDAS_DAS_UNIQUE_VALUE_TYPE_UNKNOWN;
                }
                break;

            case LIBDAS_DAS_SCOPE_SKELETON_JOINT:
                switch(type) {
                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_INVERSE_BIND_POS:
                        return DasSkeletonJoint::LIBDAS_SKELETON_JOINT_INVERSE_BIND_POS;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_NAME:
                        return DasSkeletonJoint::LIBDAS_SKELETON_JOINT_NAME;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_PARENT:
                        return DasSkeletonJoint::LIBDAS_SKELETON_JOINT_PARENT;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_SCALE:
                        return DasSkeletonJoint::LIBDAS_SKELETON_JOINT_SCALE;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_ROTATION:
                        return DasSkeletonJoint::LIBDAS_SKELETON_JOINT_ROTATION;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_TRANSLATION:
                        return DasSkeletonJoint::LIBDAS_SKELETON_JOINT_TRANSLATION;

                    default:
                        return LIBDAS_DAS_UNIQUE_VALUE_TYPE_UNKNOWN;
                }
                break;

            case LIBDAS_DAS_SCOPE_ANIMATION:
                switch(type) {
                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_NAME:
                        return DasAnimation::LIBDAS_ANIMATION_NAME;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_NODE_ID:
                        return DasAnimation::LIBDAS_ANIMATION_NODE_ID;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_DURATION:
                        return DasAnimation::LIBDAS_ANIMATION_DURATION;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_KEYFRAME_COUNT:
                        return DasAnimation::LIBDAS_ANIMATION_KEYFRAME_COUNT;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_KEYFRAME_TIMESTAMPS:
                        return DasAnimation::LIBDAS_ANIMATION_KEYFRAME_TIMESTAMPS;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_INTERPOLATION_TYPES:
                        return DasAnimation::LIBDAS_ANIMATION_INTERPOLATION_TYPES;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_ANIMATION_TARGETS:
                        return DasAnimation::LIBDAS_ANIMATION_TARGETS;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_KEYFRAME_BUFFER_ID:
                        return DasAnimation::LIBDAS_ANIMATION_KEYFRAME_BUFFER_ID;

                    case LIBDAS_DAS_UNIQUE_VALUE_TYPE_KEYFRAME_BUFFER_OFFSET:
                        return DasAnimation::LIBDAS_ANIMATION_KEYFRAME_COUNT;

                    default:
                        return LIBDAS_DAS_UNIQUE_VALUE_TYPE_UNKNOWN;
                }
                break;

            default:
                return LIBDAS_DAS_UNIQUE_VALUE_TYPE_UNKNOWN;
        }
    }


    void DasReaderCore::_CheckValueLimit(uint32_t _max, uint32_t _val) {
        if(!_val || _max < _val) {
            m_error.Error(LIBDAS_ERROR_INVALID_DATA);
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

            case DasProperties::LIBDAS_PROPERTIES_DEFAULT_SCENE:
                _props->default_scene = *reinterpret_cast<uint32_t*>(_GetReadPtr());
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


    void DasReaderCore::_ReadMeshValue(DasMesh *_model, DasMesh::ValueType _type) {
        switch(_type) {
            case DasMesh::LIBDAS_MESH_NAME:
                _model->name = _ExtractString();
                break;

            case DasMesh::LIBDAS_MESH_INDEX_BUFFER_ID:
                _model->index_buffer_id = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _SkipData(sizeof(uint32_t));
                break;

            case DasMesh::LIBDAS_MESH_INDEX_BUFFER_OFFSET:
                _model->index_buffer_offset = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _SkipData(sizeof(uint32_t));
                break;

            case DasMesh::LIBDAS_MESH_INDICES_COUNT:
                _model->indices_count = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _SkipData(sizeof(uint32_t));
                break;

            case DasMesh::LIBDAS_MESH_VERTEX_BUFFER_ID:
                _model->vertex_buffer_id = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _SkipData(sizeof(uint32_t));
                break;

            case DasMesh::LIBDAS_MESH_VERTEX_BUFFER_OFFSET:
                _model->vertex_buffer_offset = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _SkipData(sizeof(uint32_t));
                break;

            case DasMesh::LIBDAS_MESH_TEXTURE_ID:
                _model->texture_id = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _SkipData(sizeof(uint32_t));
                break;

            case DasMesh::LIBDAS_MESH_TEXTURE_MAP_BUFFER_ID:
                _model->texture_map_buffer_id = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _SkipData(sizeof(uint32_t));
                break;

            case DasMesh::LIBDAS_MESH_TEXTURE_MAP_BUFFER_OFFSET:
                _model->texture_map_buffer_offset = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _SkipData(sizeof(uint32_t));
                break;

            case DasMesh::LIBDAS_MESH_VERTEX_NORMAL_BUFFER_ID:
                _model->vertex_normal_buffer_id = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _SkipData(sizeof(uint32_t));
                break;

            case DasMesh::LIBDAS_MESH_VERTEX_NORMAL_BUFFER_OFFSET:
                _model->vertex_normal_buffer_offset = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _SkipData(sizeof(uint32_t));
                break;

            default:
                LIBDAS_ASSERT(false);
                break;
        }
    }


    void DasReaderCore::_ReadNodeValue(DasNode *_node, DasNode::ValueType _type) {
        switch(_type) {
            case DasNode::LIBDAS_NODE_NAME:
                _node->name = _ExtractString();
                break;

            case DasNode::LIBDAS_NODE_CHILDREN_COUNT:
                _node->children_count = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _CheckValueLimit(MAX_CHILDREN_PER_NODE, _node->children_count);
                _SkipData(sizeof(uint32_t));

                // allocate memory for children
                _node->children = new uint32_t[_node->children_count];
                break;

            case DasNode::LIBDAS_NODE_CHILDREN:
                for(uint32_t i = 0; i < _node->children_count; i++) {
                    _node->children[i] = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                    if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA);
                }
                break;

            case DasNode::LIBDAS_NODE_MESH_COUNT:
                _node->mesh_count = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _CheckValueLimit(MAX_MESHES_PER_NODE, _node->mesh_count);
                _SkipData(sizeof(uint32_t));

                // allocate memory for meshes
                _node->meshes = new uint32_t[_node->mesh_count];
                break;

            case DasNode::LIBDAS_NODE_MESHES:
                for(uint32_t i = 0; i < _node->mesh_count; i++) {
                    _node->meshes[i] = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                    if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA);
                }
                break;

            case DasNode::LIBDAS_NODE_ANIMATION_COUNT:
                _node->animation_count = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _CheckValueLimit(MAX_ANIMATIONS_PER_NODE, _node->animation_count);
                _SkipData(sizeof(uint32_t));

                // allocate memory for animations
                _node->animations = new uint32_t[_node->mesh_count];
                break;

            case DasNode::LIBDAS_NODE_ANIMATIONS:
                for(uint32_t i = 0; i < _node->animation_count; i++) {
                    _node->animations[i] = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                    if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA);
                }
                break;

            case DasNode::LIBDAS_NODE_SKELETON_COUNT:
                _node->skeleton_count = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _CheckValueLimit(MAX_SKELETONS_PER_NODE, _node->skeleton_count);
                _SkipData(sizeof(uint32_t));

                // allocate memory for skeletons
                _node->skeletons = new uint32_t[_node->skeleton_count];
                break;

            case DasNode::LIBDAS_NODE_SKELETONS:
                for(uint32_t i = 0; i < _node->skeleton_count; i++) {
                    _node->skeletons[i] = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                    if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA);
                }
                break;

            case DasNode::LIBDAS_NODE_TRANSFORM:
                for(struct { Matrix4<float>::iterator it; int i; } s = {_node->transform.BeginRowMajor(), 0}; s.it != _node->transform.EndRowMajor(); s.it++, s.i++)
                    *s.it = reinterpret_cast<float*>(_GetReadPtr())[s.i];

                _SkipData(sizeof(float[16]));
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

            case DasScene::LIBDAS_SCENE_NODE_COUNT:
                _scene->node_count = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _CheckValueLimit(MAX_NODES_PER_SCENE, _scene->node_count);
                _SkipData(sizeof(uint32_t));

                // allocate memory for scene nodes
                _scene->nodes = new uint32_t[_scene->node_count];
                break;

            case DasScene::LIBDAS_SCENE_NODES:
                for(uint32_t i = 0; i < _scene->node_count; i++) {
                    _scene->nodes[i] = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                    if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA);
                }
                break;

            default:
                LIBDAS_ASSERT(false);
                break;
        }
    }


    void DasReaderCore::_ReadSkeletonValue(DasSkeleton *_skeleton, DasSkeleton::ValueType _type) {
        switch(_type) {
            case DasSkeleton::LIBDAS_SKELETON_NAME:
                _skeleton->name = _ExtractString();
                break;

            case DasSkeleton::LIBDAS_SKELETON_JOINT_COUNT:
                _skeleton->joint_count = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _CheckValueLimit(MAX_JOINTS_PER_SKELETON, _skeleton->joint_count);
                _SkipData(sizeof(uint32_t));

                // allocate memory for skeletons
                _skeleton->joints = new uint32_t[_skeleton->joint_count];
                break;

            case DasSkeleton::LIBDAS_SKELETON_JOINTS:
                for(uint32_t i = 0; i < _skeleton->joint_count; i++) {
                    _skeleton->joints[i] = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                    if(!_SkipData(sizeof(uint32_t))) m_error.Error(LIBDAS_ERROR_INVALID_DATA);
                }
                break;

            default:
                break;
        }
    }


    void DasReaderCore::_ReadSkeletonJointValue(DasSkeletonJoint *_joint, DasSkeletonJoint::ValueType _type) {
        switch(_type) {
            case DasSkeletonJoint::LIBDAS_SKELETON_JOINT_INVERSE_BIND_POS:
                for(struct { Matrix4<float>::iterator it; int i; } s = {_joint->inverse_bind_pos.BeginRowMajor(), 0}; s.it != _joint->inverse_bind_pos.EndRowMajor(); s.it++, s.i++)
                    *s.it = reinterpret_cast<float*>(_GetReadPtr())[s.i];
                _SkipData(sizeof(float[16]));
                break;

            case DasSkeletonJoint::LIBDAS_SKELETON_JOINT_NAME:
                _joint->name = _ExtractString();
                break;

            case DasSkeletonJoint::LIBDAS_SKELETON_JOINT_PARENT:
                _joint->parent = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _SkipData(sizeof(uint32_t));
                break;

            case DasSkeletonJoint::LIBDAS_SKELETON_JOINT_SCALE:
                _joint->scale = *reinterpret_cast<float*>(_GetReadPtr());
                _SkipData(sizeof(float));
                break;

            case DasSkeletonJoint::LIBDAS_SKELETON_JOINT_ROTATION:
                _joint->rotation.x = *reinterpret_cast<float*>(_GetReadPtr());
                _SkipData(sizeof(float));
                _joint->rotation.y = *reinterpret_cast<float*>(_GetReadPtr());
                _SkipData(sizeof(float));
                _joint->rotation.z = *reinterpret_cast<float*>(_GetReadPtr());
                _SkipData(sizeof(float));
                _joint->rotation.w = *reinterpret_cast<float*>(_GetReadPtr());
                _SkipData(sizeof(float));
                break;

            case DasSkeletonJoint::LIBDAS_SKELETON_JOINT_TRANSLATION:
                _joint->translation.first = *reinterpret_cast<float*>(_GetReadPtr());
                _SkipData(sizeof(float));
                _joint->translation.second = *reinterpret_cast<float*>(_GetReadPtr());
                _SkipData(sizeof(float));
                _joint->translation.third = *reinterpret_cast<float*>(_GetReadPtr());
                _SkipData(sizeof(float));
                break;

            default:
                break;
        }
    }


    void DasReaderCore::_ReadAnimationValue(DasAnimation *_animation, DasAnimation::ValueType _type) {
        switch(_type) {
            case DasAnimation::LIBDAS_ANIMATION_NAME:
                _animation->name = _ExtractString();
                break;

            case DasAnimation::LIBDAS_ANIMATION_NODE_ID:
                _animation->node_id = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _SkipData(sizeof(uint32_t));
                break;

            case DasAnimation::LIBDAS_ANIMATION_DURATION:
                _animation->duration = *reinterpret_cast<float*>(_GetReadPtr());
                _SkipData(sizeof(float));
                break;

            case DasAnimation::LIBDAS_ANIMATION_KEYFRAME_COUNT:
                _animation->keyframe_count = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _CheckValueLimit(MAX_KEYFRAMES_PER_ANIMATION, _animation->keyframe_count);
                _SkipData(sizeof(uint32_t));

                // allocate memory for keyframe specifiers
                _animation->keyframe_timestamps = new float[_animation->keyframe_count];
                _animation->interpolation_types = new InterpolationType[_animation->keyframe_count];
                _animation->animation_targets = new AnimationTarget[_animation->keyframe_count];
                break;

            case DasAnimation::LIBDAS_ANIMATION_KEYFRAME_TIMESTAMPS:
                for(uint32_t i = 0; i < _animation->keyframe_count; i++) {
                    _animation->keyframe_timestamps[i] = *reinterpret_cast<float*>(_GetReadPtr());
                    if(!_SkipData(sizeof(float))) m_error.Error(LIBDAS_ERROR_INVALID_DATA);
                }
                break;

            case DasAnimation::LIBDAS_ANIMATION_INTERPOLATION_TYPES:
                for(uint32_t i = 0; i < _animation->keyframe_count; i++) {
                    _animation->interpolation_types[i] = *reinterpret_cast<float*>(_GetReadPtr());
                    if(!_SkipData(sizeof(float))) m_error.Error(LIBDAS_ERROR_INVALID_VALUE);
                }
                break;

            case DasAnimation::LIBDAS_ANIMATION_TARGETS:
                for(uint32_t i = 0; i < _animation->keyframe_count; i++) {
                    _animation->animation_targets[i] = *reinterpret_cast<float*>(_GetReadPtr());
                    if(!_SkipData(sizeof(float))) m_error.Error(LIBDAS_ERROR_INVALID_VALUE);
                }
                break;

            case DasAnimation::LIBDAS_ANIMATION_KEYFRAME_BUFFER_ID:
                _animation->keyframe_buffer_id = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _SkipData(sizeof(uint32_t));
                break;

            case DasAnimation::LIBDAS_ANIMATION_KEYFRAME_BUFFER_OFFSET:
                _animation->keyframe_buffer_offset = *reinterpret_cast<uint32_t*>(_GetReadPtr());
                _SkipData(sizeof(uint32_t));
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

            case LIBDAS_DAS_SCOPE_MESH:
                if(_value_type.type() != typeid(DasMesh::ValueType))
                    m_error.Error(LIBDAS_ERROR_INVALID_VALUE, _val_str);
                _ReadMeshValue(std::any_cast<DasMesh>(&_scope), std::any_cast<DasMesh::ValueType>(_value_type));
                break;

            case LIBDAS_DAS_SCOPE_NODE:
                if(_value_type.type() != typeid(DasNode::ValueType))
                    m_error.Error(LIBDAS_ERROR_INVALID_VALUE, _val_str);
                _ReadNodeValue(std::any_cast<DasNode>(&_scope), std::any_cast<DasNode::ValueType>(_value_type));
                break;

            case LIBDAS_DAS_SCOPE_SCENE:
                if(_value_type.type() != typeid(DasScene::ValueType))
                    m_error.Error(LIBDAS_ERROR_INVALID_VALUE, _val_str);
                _ReadSceneValue(std::any_cast<DasScene>(&_scope), std::any_cast<DasScene::ValueType>(_value_type));
                break;

            case LIBDAS_DAS_SCOPE_SKELETON:
                if(_value_type.type() != typeid(DasSkeleton::ValueType))
                    m_error.Error(LIBDAS_ERROR_INVALID_VALUE, _val_str);
                _ReadSkeletonValue(std::any_cast<DasSkeleton>(&_scope), std::any_cast<DasSkeleton::ValueType>(_value_type));
                break;

            case LIBDAS_DAS_SCOPE_SKELETON_JOINT:
                if(_value_type.type() != typeid(DasSkeletonJoint::ValueType))
                    m_error.Error(LIBDAS_ERROR_INVALID_VALUE, _val_str);
                _ReadSkeletonJointValue(std::any_cast<DasSkeletonJoint>(&_scope), std::any_cast<DasSkeletonJoint::ValueType>(_value_type));
                break;

            case LIBDAS_DAS_SCOPE_ANIMATION:
                if(_value_type.type() != typeid(DasAnimation::ValueType))
                    m_error.Error(LIBDAS_ERROR_INVALID_VALUE, _val_str);
                _ReadAnimationValue(std::any_cast<DasAnimation>(&_scope), std::any_cast<DasAnimation::ValueType>(_value_type));
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
                    m_error.Error(LIBDAS_ERROR_INVALID_DATA);
                else if(val_decl == "ENDSCOPE") 
                    break;
                else m_error.Error(LIBDAS_ERROR_INVALID_DATA);
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
