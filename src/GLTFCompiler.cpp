// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: GLTFCompiler.cpp - GLTF format to DAS compiler implementation
// author: Karl-Mihkel Ott

#define GLTF_COMPILER_CPP
#include <GLTFCompiler.h>


namespace Libdas {

    GLTFCompiler::GLTFCompiler(const std::string &_in_path, const std::string &_out_file, bool _use_raw_textures) : 
        DasWriterCore(_out_file), m_use_raw_textures(_use_raw_textures), m_root_path(_in_path) {}

    GLTFCompiler::GLTFCompiler(const std::string &_in_path, GLTFRoot &_root, const DasProperties &_props, 
                               const std::string &_out_file, const std::vector<std::string> &_embedded_textures, bool _use_raw_textures) : 
        m_use_raw_textures(_use_raw_textures), m_root_path(_in_path)
    {
        Compile(_root, _props, _embedded_textures, _out_file);
    }


    GLTFCompiler::~GLTFCompiler() {
        _FreeSupplementedBuffers(m_supplemented_buffers);
    }


    GLTFCompiler::BufferAccessorData GLTFCompiler::_FindAccessorData(const GLTFRoot &_root, int32_t _accessor_id) {
        GLTFCompiler::BufferAccessorData accessor_data;
        accessor_data.buffer_id = static_cast<uint32_t>(_root.buffer_views[_root.accessors[_accessor_id].buffer_view].buffer);

        if(!_root.accessors[_accessor_id].accumulated_offset) {
            accessor_data.buffer_offset = static_cast<uint32_t>(_root.buffer_views[_root.accessors[_accessor_id].buffer_view].byte_offset +
                                                                _root.accessors[_accessor_id].byte_offset);
        } else {
            accessor_data.buffer_offset = _root.accessors[_accessor_id].accumulated_offset;
        }

        // component type being 0 means, that the entire used buffer size calculation is comprimised
        uint32_t component_mul = 0;
        switch(_root.accessors[_accessor_id].component_type) {
            case KHRONOS_BYTE:
            case KHRONOS_UNSIGNED_BYTE:
                component_mul = static_cast<uint32_t>(sizeof(char));
                break;

            case KHRONOS_SHORT:
            case KHRONOS_UNSIGNED_SHORT:
                component_mul = static_cast<uint32_t>(sizeof(short));
                break;

            case KHRONOS_UNSIGNED_INT:
                component_mul = static_cast<uint32_t>(sizeof(unsigned int));
                break;

            case KHRONOS_FLOAT:
                component_mul = static_cast<uint32_t>(sizeof(float));
                break;

            default:
                break;
        }

        uint32_t type_mul = 0;
        if(_root.accessors[_accessor_id].type == "SCALAR")
            type_mul = 1;
        else if(_root.accessors[_accessor_id].type == "VEC2")
            type_mul = 2;
        else if(_root.accessors[_accessor_id].type == "VEC3")
            type_mul = 3;
        else if(_root.accessors[_accessor_id].type == "VEC4")
            type_mul = 4;
        else if(_root.accessors[_accessor_id].type == "MAT2")
            type_mul = 4;
        else if(_root.accessors[_accessor_id].type == "MAT3")
            type_mul = 9;
        else if(_root.accessors[_accessor_id].type == "MAT4")
            type_mul = 16;

        accessor_data.used_size = _root.accessors[_accessor_id].count * type_mul * component_mul;
        accessor_data.component_type = _root.accessors[_accessor_id].component_type;
        accessor_data.unit_size = type_mul * component_mul;
        return accessor_data;
    }


    std::vector<std::vector<GLTFAccessor*>> GLTFCompiler::_GetAllBufferAccessorRegions(GLTFRoot &_root) {
        std::vector<std::vector<GLTFAccessor*>> all_regions(_root.buffers.size());

        // find buffer offsets from accessors
        for(int32_t i = 0; i < static_cast<int32_t>(_root.accessors.size()); i++) {
            BufferAccessorData accessor_data = _FindAccessorData(_root, i);
            all_regions[accessor_data.buffer_id].push_back(&_root.accessors[i]);
            all_regions[accessor_data.buffer_id].back()->accumulated_offset = accessor_data.buffer_offset;
        }

        // sort results
        for(size_t i = 0; i < all_regions.size(); i++)
            std::sort(all_regions[i].begin(), all_regions[i].end(), GLTFAccessor::less());


        return all_regions;
    }


    std::vector<std::vector<GLTFCompiler::BufferAccessorData>> GLTFCompiler::_GetBufferIndexRegions(GLTFRoot &_root) {
        std::vector<std::vector<BufferAccessorData>> index_regions(_root.buffers.size());

        // find all index regions
        for(auto it = _root.meshes.begin(); it != _root.meshes.end(); it++) {
            for(size_t i = 0; i < it->primitives.size(); i++) {
                // error if non-indexed geometry is used
                if(it->primitives[i].indices == INT32_MAX)
                    return index_regions;

                if(_root.accessors[it->primitives[i].indices].component_type == KHRONOS_BYTE ||
                   _root.accessors[it->primitives[i].indices].component_type == KHRONOS_UNSIGNED_BYTE ||
                   _root.accessors[it->primitives[i].indices].component_type == KHRONOS_SHORT ||
                   _root.accessors[it->primitives[i].indices].component_type == KHRONOS_UNSIGNED_SHORT) 
                {
                    BufferAccessorData accessor_data = _FindAccessorData(_root, it->primitives[i].indices);
                    index_regions[accessor_data.buffer_id].push_back(accessor_data);
                    _root.accessors[it->primitives[i].indices].component_type = KHRONOS_UNSIGNED_INT;
                }
            }
        }

        return index_regions;
    }


    std::vector<std::vector<GLTFCompiler::BufferAccessorData>> GLTFCompiler::_GetBufferJointRegions(GLTFRoot &_root) {
        std::vector<std::vector<BufferAccessorData>> joint_regions(_root.buffers.size());

        // find all joint regions
        for(auto mesh_it = _root.meshes.begin(); mesh_it != _root.meshes.end(); mesh_it++) {
            for(auto prim_it = mesh_it->primitives.begin(); prim_it != mesh_it->primitives.end(); prim_it++) {
                auto attr_it = prim_it->attributes.begin();
                // find JOINTS_0 object
                for(; attr_it != prim_it->attributes.end(); attr_it++) {
                    if(attr_it->first == "JOINTS_0")
                        break;
                }

                if(attr_it == prim_it->attributes.end()) continue;

                BufferAccessorData accessor_data = _FindAccessorData(_root, attr_it->second);
                joint_regions[accessor_data.buffer_id].push_back(accessor_data);
            }
        }

        return joint_regions;
    }


    std::vector<std::vector<GLTFCompiler::BufferAccessorData>> GLTFCompiler::_GetBufferWeightRegions(GLTFRoot &_root) {
        std::vector<std::vector<BufferAccessorData>> weight_regions(_root.buffers.size());

        // find all weight regions
        for(auto mesh_it = _root.meshes.begin(); mesh_it != _root.meshes.end(); mesh_it++) {
            for(auto prim_it = mesh_it->primitives.begin(); prim_it != mesh_it->primitives.end(); prim_it++) {
                auto attr_it = prim_it->attributes.begin();
                // find WEIGHTS_0 object
                for(; attr_it != prim_it->attributes.end(); attr_it++) {
                    if(attr_it->first == "WEIGHTS_0")
                        break;
                }

                if(attr_it == prim_it->attributes.end()) continue;

                BufferAccessorData accessor_data = _FindAccessorData(_root, attr_it->second);
                if(accessor_data.component_type != KHRONOS_FLOAT)
                    weight_regions[accessor_data.buffer_id].push_back(accessor_data);
            }
        }

        return weight_regions;
    }


    void GLTFCompiler::_GetUnindexedMeshPrimitives(GLTFRoot &_root) {
        m_unindexed_primitives.resize(_root.meshes.size());

        // find all unindexed mesh primitives
        for(size_t i = 0; i < _root.meshes.size(); i++) {
            uint32_t j = 0;
            for(auto prim_it = _root.meshes[i].primitives.begin(); prim_it != _root.meshes[i].primitives.end(); prim_it++, j++) {
                if(prim_it->indices == INT32_MAX)
                    m_unindexed_primitives[i].push_back(j);
            }
        }
    }


    void GLTFCompiler::_CorrectOffsets(std::vector<GLTFAccessor*> &_accessors, size_t _diff, size_t _offsets) {
        for(GLTFAccessor *accessor : _accessors) {
            if(accessor->accumulated_offset <= _offsets)
                continue;

            accessor->byte_offset += static_cast<uint32_t>(_diff);
            accessor->accumulated_offset += static_cast<uint32_t>(_diff);
        }
    }


    size_t GLTFCompiler::_FindPrimitiveCount(const GLTFRoot &_root) {
        size_t count = 0;
        for(size_t i = 0; i < _root.meshes.size(); i++)
            count += _root.meshes[i].primitives.size();

        return count;
    }


    std::vector<size_t> GLTFCompiler::_FindMeshNodes(const GLTFRoot &_root, size_t _mesh_index) {
        std::vector<size_t> nodes;
        nodes.reserve(_root.nodes.size());

        for(size_t i = 0; i < _root.nodes.size(); i++) {
            if(_root.nodes[i].mesh == static_cast<int32_t>(_mesh_index))
                nodes.push_back(i);
        }

        nodes.shrink_to_fit();
        return nodes;
    }


    const std::vector<float> GLTFCompiler::_FindMorphWeightsFromNodes(const GLTFRoot &_root, size_t _mesh_index) {
        std::vector<size_t> nodes = _FindMeshNodes(_root, _mesh_index);

        if(nodes.size() > 1)
            return std::vector<float>();

        return _root.nodes[nodes.back()].weights;
    }


    auto GLTFCompiler::_FindDataPtrFromOffset(const std::vector<std::pair<const char*, size_t>> &_ptrs, size_t &_offset) {
        size_t current_offset = 0;
        for(auto it = _ptrs.begin(); it != _ptrs.end(); it++) {
            if(current_offset <= _offset && current_offset + it->second > _offset) {
                _offset -= current_offset;
                return it;
            }

            current_offset += it->second;
        }

        LIBDAS_ASSERT(false);
        return _ptrs.end();
    }


    void GLTFCompiler::_FlagJointNodes(const GLTFRoot &_root) {
        std::vector<bool> skin_joint_table(_root.nodes.size());
        std::fill(skin_joint_table.begin(), skin_joint_table.end(), false);

        m_scene_node_id_table.resize(_root.nodes.size());
        m_skeleton_joint_id_table.resize(_root.nodes.size());

        for(auto skin_it = _root.skins.begin(); skin_it != _root.skins.end(); skin_it++) {
            for(size_t i = 0; i < skin_it->joints.size(); i++)
                skin_joint_table[skin_it->joints[i]] = true;
        }

        uint32_t skipped_nodes = 0;
        for(uint32_t i = 0; i < static_cast<uint32_t>(_root.nodes.size()); i++) {
            if(skin_joint_table[i]) {
                m_scene_node_id_table[i] = UINT32_MAX;
                m_skeleton_joint_id_table[i] = skipped_nodes;
                skipped_nodes++;
            }
            else if(_root.nodes[i].children.size() == 0 && _root.nodes[i].skin == INT32_MAX && _root.nodes[i].mesh == INT32_MAX) {
                m_scene_node_id_table[i] = UINT32_MAX;
                m_skeleton_joint_id_table[i] = UINT32_MAX;
                skipped_nodes++;
            }
            else {
                m_skeleton_joint_id_table[i] = UINT32_MAX;
                m_scene_node_id_table[i] = i - skipped_nodes;
            }
        }
    }


    bool GLTFCompiler::_IsRootNode(const GLTFRoot &_root, int32_t _node_id, const std::vector<int32_t> &_pool) {
        for(int32_t i = 0; i < static_cast<int32_t>(_root.nodes[_node_id].children.size()); i++) {
            std::vector<int32_t> npool = _pool;
            auto it = std::find(npool.begin(), npool.end(), _root.nodes[_node_id].children[i]);
            if(it == npool.end()) {
                std::cerr << "Invalid child object in children pool" << std::endl;
                std::exit(LIBDAS_ERROR_INVALID_DATA);
            }

            npool.erase(it);

            if(!npool.size()) return true;

            if(_IsRootNode(_root, _root.nodes[_node_id].children[i], npool))
                return true;
        }

        return false;
    }


    uint32_t GLTFCompiler::_FindCommonRootJoint(const GLTFRoot &_root, const GLTFSkin &_skin) {
        for(size_t i = 0; i < _skin.joints.size(); i++) {
            std::vector<int32_t> pool = _skin.joints;
            pool.erase(pool.begin() + i);
            if(_IsRootNode(_root, _skin.joints[i], pool))
                return static_cast<uint32_t>(i);
        }

        return UINT32_MAX;
    }


    uint32_t GLTFCompiler::_SupplementIndices(const char *_odata, BufferAccessorData &_suppl_info, DasBuffer &_buffer) {
        char *buf = nullptr;
        size_t len = 0; // in units not in bytes
        size_t diff = 0;

        switch(_suppl_info.component_type) {
            case KHRONOS_BYTE:
                len = (static_cast<size_t>(_suppl_info.used_size) / sizeof(char));
                buf = new char[len * sizeof(uint32_t)];
                diff = len * sizeof(uint32_t) - len * sizeof(char);

                for(size_t i = 0; i < len; i++)
                    reinterpret_cast<uint32_t*>(buf)[i] = static_cast<uint32_t>((_odata + _suppl_info.buffer_offset)[i]);
                break;

            case KHRONOS_UNSIGNED_BYTE:
                len = (static_cast<size_t>(_suppl_info.used_size) / sizeof(unsigned char));
                buf = new char[len * sizeof(uint32_t)];
                diff = len * sizeof(uint32_t) - len * sizeof(unsigned char);

                for(size_t i = 0; i < len; i++)
                    reinterpret_cast<uint32_t*>(buf)[i] = static_cast<uint32_t>((reinterpret_cast<const unsigned char*>(_odata + _suppl_info.buffer_offset))[i]);
                break;

            case KHRONOS_SHORT:
                len = (static_cast<size_t>(_suppl_info.used_size) / sizeof(short));
                buf = new char[len * sizeof(uint32_t)];
                diff = len * sizeof(uint32_t) - len * sizeof(short);

                for(size_t i = 0; i < len; i++)
                    reinterpret_cast<uint32_t*>(buf)[i] = static_cast<uint32_t>((reinterpret_cast<const short*>(_odata + _suppl_info.buffer_offset))[i]);
                break;

            case KHRONOS_UNSIGNED_SHORT:
                len = (static_cast<size_t>(_suppl_info.used_size) / sizeof(unsigned short));
                buf = new char[len * sizeof(uint32_t)];
                diff = len * sizeof(uint32_t) - len * sizeof(unsigned short);

                for(size_t i = 0; i < len; i++)
                    reinterpret_cast<uint32_t*>(buf)[i] = static_cast<uint32_t>((reinterpret_cast<const unsigned short*>(_odata + _suppl_info.buffer_offset))[i]);
                break;

            default:
                LIBDAS_ASSERT(false);
                break;
        }

        _buffer.data_ptrs.push_back(std::make_pair(reinterpret_cast<const char*>(buf), len * sizeof(uint32_t)));
        _buffer.data_len += static_cast<uint32_t>(diff);
        m_supplemented_buffers.push_back(buf);
        return static_cast<uint32_t>(diff);
    }


    uint32_t GLTFCompiler::_SupplementJointIndices(const char *_odata, BufferAccessorData &_suppl_info, DasBuffer &_buffer) {
        char *buf = nullptr;
        size_t len = 0; // units
        size_t diff = 0;

        switch(_suppl_info.component_type) {
            case KHRONOS_UNSIGNED_BYTE:
                len = (static_cast<size_t>(_suppl_info.used_size) / sizeof(unsigned char));
                buf = new char[len * sizeof(uint32_t)];
                diff = len * sizeof(uint32_t) - len * sizeof(unsigned char);

                for(size_t i = 0; i < len; i++)
                    reinterpret_cast<uint32_t*>(buf)[i] = static_cast<uint32_t>(reinterpret_cast<const unsigned char*>(_odata)[i]);
                break;

            case KHRONOS_UNSIGNED_SHORT:
                len = (static_cast<size_t>(_suppl_info.used_size) / sizeof(unsigned short));
                buf = new char[len * sizeof(uint32_t)];
                diff = len * sizeof(uint32_t) - len * sizeof(unsigned short);

                for(size_t i = 0; i < len; i++)
                    reinterpret_cast<uint32_t*>(buf)[i] = static_cast<uint32_t>(reinterpret_cast<const unsigned short*>(_odata)[i]);
                break;

            default:
                LIBDAS_ASSERT(false);
                break;
        }

        _buffer.data_ptrs.push_back(std::make_pair(reinterpret_cast<const char*>(buf), len * sizeof(uint32_t)));
        _buffer.data_len += static_cast<uint32_t>(diff);
        m_supplemented_buffers.push_back(buf);
        return static_cast<uint32_t>(diff);
    }


    uint32_t GLTFCompiler::_SupplementJointWeights(const char *_odata, BufferAccessorData &_suppl_info, DasBuffer &_buffer) {
        char *buf = nullptr;
        size_t len = 0;
        size_t diff = 0;

        switch(_suppl_info.component_type) {
            case KHRONOS_UNSIGNED_BYTE:
                len = (static_cast<size_t>(_suppl_info.used_size) / sizeof(unsigned char));
                buf = new char[len * sizeof(float)];
                diff = len * sizeof(float) - len * sizeof(unsigned char);

                for(size_t i = 0; i < len; i++)
                    reinterpret_cast<float*>(buf)[i] = static_cast<float>(reinterpret_cast<const unsigned char*>(_odata)[i]) / 255.0f;
                break;

            case KHRONOS_UNSIGNED_SHORT:
                len = (static_cast<size_t>(_suppl_info.used_size) / sizeof(unsigned short));
                buf = new char[len * sizeof(float)];
                diff = len * sizeof(float) - len * sizeof(unsigned short);

                for(size_t i = 0; i < len; i++)
                    reinterpret_cast<float*>(buf)[i] = static_cast<float>(reinterpret_cast<const unsigned short*>(_odata)[i]) / 65535.0f;
                break;

            default:
                LIBDAS_ASSERT(false);
                break;
        }

        _buffer.data_ptrs.push_back(std::make_pair(reinterpret_cast<const char*>(buf), len * sizeof(float)));
        _buffer.data_len += static_cast<uint32_t>(diff);
        m_supplemented_buffers.push_back(buf);
        return diff;
    }


    void GLTFCompiler::_SupplementVertexAttrs(const std::unordered_map<GenericVertexAttribute, uint32_t, Hash<GenericVertexAttribute>> &_map, uint32_t _max, BufferType _type, DasBuffer &_buffer) {
        char *buf = nullptr;
        size_t len = static_cast<size_t>(_max);

        switch(_type) {
            case LIBDAS_BUFFER_TYPE_VERTEX:
                buf = new char[len * sizeof(Vector3<float>)];
                for(auto it = _map.begin(); it != _map.end(); it++)
                    reinterpret_cast<Vector3<float>*>(buf)[it->second] = it->first.pos;
                len *= sizeof(Vector3<float>);
                break;

            case LIBDAS_BUFFER_TYPE_VERTEX_NORMAL:
                buf = new char[len * sizeof(Vector3<float>)];
                for(auto it = _map.begin(); it != _map.end(); it++)
                    reinterpret_cast<Vector3<float>*>(buf)[it->second] = it->first.norm;
                len *= sizeof(Vector3<float>);
                break;

            case LIBDAS_BUFFER_TYPE_VERTEX_TANGENT:
                buf = new char[len * sizeof(Vector4<float>)];
                for(auto it = _map.begin(); it != _map.end(); it++)
                    reinterpret_cast<Vector4<float>*>(buf)[it->second] = it->first.tangent;
                len *= sizeof(Vector4<float>);
                break;

            case LIBDAS_BUFFER_TYPE_TEXTURE_MAP:
                buf = new char[len * sizeof(Vector2<float>)];
                for(auto it = _map.begin(); it != _map.end(); it++)
                    reinterpret_cast<Vector2<float>*>(buf)[it->second] = it->first.tex0;
                len *= sizeof(Vector2<float>);
                break;

            case LIBDAS_BUFFER_TYPE_JOINTS:
                buf = new char[len * sizeof(Vector4<uint32_t>)];
                for(auto it = _map.begin(); it != _map.end(); it++)
                    reinterpret_cast<Vector4<uint32_t>*>(buf)[it->second] = it->first.joints;
                len *= sizeof(Vector4<uint32_t>);
                break;

            case LIBDAS_BUFFER_TYPE_WEIGHTS:
                buf = new char[len * sizeof(Vector4<float>)];
                for(auto it = _map.begin(); it != _map.end(); it++)
                    reinterpret_cast<Vector4<float>*>(buf)[it->second] = it->first.weights;
                len *= sizeof(Vector4<float>);
                break;

            default:
                break;
        }

        _buffer.data_ptrs.push_back(std::make_pair(reinterpret_cast<const char*>(buf), len));
        m_supplemented_buffers.push_back(buf);
    }


    void GLTFCompiler::_CopyToBuffer(const std::vector<std::pair<const char*, size_t>> &_optrs, char *_dst, size_t _len, size_t _offset, DasBuffer &_buffer) {
        auto it = _FindDataPtrFromOffset(_optrs, _offset);
        size_t orig_len = _len;
        size_t dst_offset = 0;

        while(_len) {
            // invalid iterator reached
            if(it == _optrs.end()) {
                LIBDAS_ASSERT(false);
            }

            size_t written = _len <= it->second ? _len : it->second - _offset;
            std::memcpy(_dst + dst_offset, it->first + _offset, written);
            _offset = 0;
            dst_offset = written;
            _len -= written;
            it++;
        }

        _buffer.data_ptrs.push_back(std::make_pair(_dst, orig_len));
        m_supplemented_buffers.push_back(_dst);
    }


    void GLTFCompiler::_StrideBuffer(GLTFCompiler::GLTFAccessors &_accessors, GLTFCompiler::BufferAccessorDatas &_regions, std::vector<DasBuffer> &_buffers, Supplement_PFN _suppl_fn) {
        std::vector<char*> prev_suppl = m_supplemented_buffers;
        m_supplemented_buffers.clear();
        bool is_aug = false;
        uint32_t accumulated_diff = 0;

        // for each buffer with index regions, supplement its data
        for(size_t i = 0; i < _regions.size(); i++) {
            uint32_t olen = _buffers[i].data_len; 
            // sort by offset size
            std::sort(_regions[i].begin(), _regions[i].end(), BufferAccessorData::less());
            Algorithm::RemoveDuplicates(_regions[i], BufferAccessorData::IsDuplicate);

            if(!_regions[i].size())
                continue;

            std::vector<std::pair<const char*, size_t>> optrs = _buffers[i].data_ptrs;
            _buffers[i].data_ptrs.clear();

            for(size_t j = 0; j < _regions[i].size(); j++) {
                is_aug = true;
                char *buf = nullptr;
                size_t len = 0;
                size_t offset;

                // copy the area before first element
                if(j == 0 && _regions[i][j].buffer_offset > 0) {
                    len = _regions[i][j].buffer_offset;
                    buf = new char[len];
                    offset = 0;

                    _CopyToBuffer(optrs, buf, len, offset, _buffers[i]);
                    buf = nullptr;
                    len = 0;
                }

                // copy the area between two index regions
                else if(_regions[i][j - 1].buffer_offset + _regions[i][j - 1].used_size < _regions[i][j].buffer_offset) {
                    offset = _regions[i][j - 1].buffer_offset + _regions[i][j - 1].used_size;
                    len = _regions[i][j].buffer_offset - offset;
                    buf = new char[len];

                    _CopyToBuffer(optrs, buf, len, offset, _buffers[i]);
                    buf = nullptr;
                    len = 0;
                    offset = 0;
                }

                // supplement indices
                offset = static_cast<size_t>(_regions[i][j].buffer_offset);
                auto it = _FindDataPtrFromOffset(optrs, offset);
                uint32_t old_offset = _regions[i][j].buffer_offset;
                len = _buffers[i].data_len;
                _regions[i][j].buffer_offset = static_cast<uint32_t>(offset);
                uint32_t diff = (this->*_suppl_fn)(it->first, _regions[i][j], _buffers[i]);
                _regions[i][j].buffer_offset = old_offset;

                // correct changed offsets
                _CorrectOffsets(_accessors[i], diff, _regions[i][j].buffer_offset + accumulated_diff);
                accumulated_diff += diff;

                // copy the area between last element and the end of the buffer
                if(j == _regions[i].size() - 1 && _regions[i][j].buffer_offset + _regions[i][j].used_size < olen) {
                    offset = _regions[i][j].buffer_offset + _regions[i][j].used_size;
                    len -= offset;
                    buf = new char[len];

                    _CopyToBuffer(optrs, buf, len, offset, _buffers[i]);
                    buf = nullptr;
                    len = 0;
                    offset = 0;
                }
            }
        }

        if(is_aug) _FreeSupplementedBuffers(prev_suppl);
        else m_supplemented_buffers = prev_suppl;
    }


    // TODO: add accessor buffer offset correction to the implementation
    // right now only supplementation is done
    void GLTFCompiler::_StrideBuffers(GLTFRoot &_root, std::vector<DasBuffer> &_buffers) {
        std::vector<std::vector<GLTFAccessor*>> all_regions(_GetAllBufferAccessorRegions(_root));

        std::vector<std::vector<BufferAccessorData>> index_regions(std::move(_GetBufferIndexRegions(_root)));
        _StrideBuffer(all_regions, index_regions, _buffers, &GLTFCompiler::_SupplementIndices);

        std::vector<std::vector<BufferAccessorData>> joint_regions(std::move(_GetBufferJointRegions(_root)));
        _StrideBuffer(all_regions, joint_regions, _buffers, &GLTFCompiler::_SupplementJointIndices);

        std::vector<std::vector<BufferAccessorData>> joint_weight_regions(std::move(_GetBufferJointRegions(_root)));
        _StrideBuffer(all_regions, joint_weight_regions, _buffers, &GLTFCompiler::_SupplementJointWeights);

        _GetUnindexedMeshPrimitives(_root);
        _IndexGeometry(_root, all_regions, _buffers);
    }


    void GLTFCompiler::_WriteIndexedData(GLTFAccessors &_accessors, std::vector<DasBuffer> &_buffers, std::unordered_map<GenericVertexAttribute, uint32_t, Hash<GenericVertexAttribute>> &_map, 
                                         const std::vector<uint32_t> &_indices, std::array<std::pair<uint64_t, BufferType>, 6> &_attr_array, uint32_t _max) 
    {
        std::sort(_attr_array.begin(), _attr_array.end(), FirstLess<uint64_t, BufferType>());

        // for each element in array
        uint32_t accumulated_diff = 0;
        uint32_t prev_id = UINT32_MAX;
        uint32_t olen = 0;
        size_t prev_unit_size = 0;
        std::vector<std::pair<const char*, size_t>> optrs;

        for(auto it = _attr_array.begin(); it != _attr_array.end(); it++) {
            if(it->first == UINT64_MAX) 
                break;
            char *buf = nullptr;
            size_t len = 0;
            size_t offset = 0;
            size_t unit_size = 0;

            // check the attribute type and determine unit size
            switch(it->second) {
                case LIBDAS_BUFFER_TYPE_VERTEX:
                    unit_size = sizeof(Vector3<float>);
                    break;

                case LIBDAS_BUFFER_TYPE_VERTEX_NORMAL:
                    unit_size = sizeof(Vector3<float>);
                    break;

                case LIBDAS_BUFFER_TYPE_VERTEX_TANGENT:
                    unit_size = sizeof(Vector4<float>);
                    break;

                case LIBDAS_BUFFER_TYPE_TEXTURE_MAP:
                    unit_size = sizeof(Vector2<float>);
                    break;

                case LIBDAS_BUFFER_TYPE_JOINTS:
                    unit_size = sizeof(Vector4<uint32_t>);
                    break;

                case LIBDAS_BUFFER_TYPE_WEIGHTS:
                    unit_size = sizeof(Vector4<float>);
                    break;

                default:
                    break;
            }

            uint32_t buffer_id = static_cast<uint32_t>(it->first >> 32);
            uint32_t buffer_offset = static_cast<uint32_t>(it->first);
            
            if(prev_id != buffer_id) {
                optrs = _buffers[buffer_id].data_ptrs;
                _buffers[buffer_id].data_ptrs.clear();
                olen = _buffers[buffer_id].data_len;
            }

            // copy the area before augmented region
            if(it == _attr_array.begin() && buffer_offset > 0) {
                len = static_cast<size_t>(buffer_offset);
                buf = new char[len];
                offset = 0;

                _CopyToBuffer(optrs, buf, len, offset, _buffers[buffer_id]);
                buf = nullptr;
                len = 0;
            }

            // copy the area between two index regions
            else if(static_cast<uint32_t>((it - 1)->first >> 32) == buffer_id && 
                    static_cast<size_t>((it - 1)->first % UINT32_MAX) + (_indices.size() * prev_unit_size) < static_cast<size_t>(buffer_offset))
            {
                offset = static_cast<size_t>((it - 1)->first % UINT32_MAX) + (_indices.size() * prev_unit_size);
                len = static_cast<size_t>(buffer_offset) - offset;
                buf = new char[len];

                _CopyToBuffer(optrs, buf, len, offset, _buffers[buffer_id]);
                buf = nullptr;
                len = 0;
                offset = 0;
            }

            // supplement attribute data
            _SupplementVertexAttrs(_map, _max, it->second, _buffers[buffer_id]);

            size_t diff = static_cast<uint32_t>(_max) * unit_size - _indices.size() * unit_size;
            _buffers[buffer_id].data_len += diff;
            _CorrectOffsets(_accessors[buffer_id], static_cast<uint32_t>(diff), buffer_offset + accumulated_diff);
            accumulated_diff += static_cast<uint32_t>(diff);

            // copy the area between last element and the end of buffer
            if((it == _attr_array.end() - 1 || ((it + 1)->first >> 32) != buffer_id) && buffer_offset + static_cast<uint32_t>(_indices.size()) * static_cast<uint32_t>(unit_size) < olen) {
                offset = static_cast<size_t>(buffer_offset) + _indices.size() * unit_size;
                len = static_cast<size_t>(olen) - offset;
                buf = new char[len];

                _CopyToBuffer(optrs, buf, len, offset, _buffers[buffer_id]);
                buf = nullptr;
                len = 0;
                offset = 0;
            }
            prev_id = buffer_id;
            prev_unit_size = unit_size;
        }
    }


    void GLTFCompiler::_IndexMeshPrimitive(GLTFRoot &_root, GLTFMeshPrimitive &_prim, std::vector<DasBuffer> &_buffers, GLTFAccessors &_accessors, uint32_t _vert_accessor, uint32_t _norm_accessor, 
                                           uint32_t _tang_accessor, uint32_t _tex_map_accessor, uint32_t _joints_accessor, uint32_t _weights_accessor) 
    {
        std::array<uint32_t, 6> offsets;
        std::fill(offsets.begin(), offsets.end(), UINT32_MAX);

        std::unordered_map<GenericVertexAttribute, uint32_t, Hash<GenericVertexAttribute>> m;
        BufferAccessorData vert_data, norm_data, tang_data, tex_map_data, joints_data, weights_data;
        std::pair<const char*, size_t> vert_buf = {}, norm_buf = {}, tang_buf = {}, tex_map_buf = {}, joints_buf = {}, weights_buf = {};
        if(_vert_accessor != UINT32_MAX) {
            vert_data = _FindAccessorData(_root, _vert_accessor);
            size_t offset = static_cast<size_t>(vert_data.buffer_offset);
            offsets[0] = vert_data.buffer_offset;
            vert_buf = *_FindDataPtrFromOffset(_buffers[vert_data.buffer_id].data_ptrs, offset);
            vert_data.buffer_offset = static_cast<uint32_t>(offset);
        }
        if(_norm_accessor != UINT32_MAX) {
            norm_data = _FindAccessorData(_root, _norm_accessor);
            size_t offset = static_cast<size_t>(norm_data.buffer_offset);
            offsets[1] = norm_data.buffer_offset;
            norm_buf = *_FindDataPtrFromOffset(_buffers[norm_data.buffer_id].data_ptrs, offset);
            norm_data.buffer_offset = static_cast<uint32_t>(offset);
        }
        if(_tang_accessor != UINT32_MAX) {
            tang_data = _FindAccessorData(_root, _norm_accessor);
            size_t offset = static_cast<size_t>(tang_data.buffer_offset);
            offsets[2] = tang_data.buffer_offset;
            tang_buf = *_FindDataPtrFromOffset(_buffers[tang_data.buffer_id].data_ptrs, offset);
            tang_data.buffer_offset = static_cast<uint32_t>(offset);
        }
        if(_tex_map_accessor != UINT32_MAX) {
            tex_map_data = _FindAccessorData(_root, _tex_map_accessor);
            size_t offset = static_cast<size_t>(tex_map_data.buffer_offset);
            offsets[3] = tex_map_data.buffer_offset;
            tex_map_buf = *_FindDataPtrFromOffset(_buffers[tex_map_data.buffer_id].data_ptrs, offset);
            tex_map_data.buffer_offset = static_cast<uint32_t>(offset);
        }
        if(_joints_accessor != UINT32_MAX) {
            joints_data = _FindAccessorData(_root, _joints_accessor);
            size_t offset = static_cast<size_t>(joints_data.buffer_offset);
            offsets[4] = joints_data.buffer_offset;
            joints_buf = *_FindDataPtrFromOffset(_buffers[joints_data.buffer_id].data_ptrs, offset);
            joints_data.buffer_offset = static_cast<uint32_t>(offset);
        }
        if(_weights_accessor != UINT32_MAX) {
            weights_data = _FindAccessorData(_root, _weights_accessor);
            size_t offset = static_cast<size_t>(weights_data.buffer_offset);
            offsets[5] = weights_data.buffer_offset;
            weights_buf = *_FindDataPtrFromOffset(_buffers[weights_data.buffer_id].data_ptrs, offset);
            weights_data.buffer_offset = static_cast<uint32_t>(offset);
        }

        uint32_t max = 0;
        m_supplemented_indices.emplace_back();
        m_supplemented_indices.back().reserve(static_cast<size_t>(vert_data.used_size / vert_data.unit_size));

        for(uint32_t i = 0; i < vert_data.used_size / vert_data.unit_size; i++) {
            GenericVertexAttribute attr;
            if(_vert_accessor != UINT32_MAX)
                attr.pos = reinterpret_cast<const Vector3<float>*>(vert_buf.first + vert_data.buffer_offset)[i];
            if(_norm_accessor != UINT32_MAX)
                attr.norm = reinterpret_cast<const Vector3<float>*>(norm_buf.first + norm_data.buffer_offset)[i];
            if(_tang_accessor != UINT32_MAX)
                attr.tangent = reinterpret_cast<const Vector4<float>*>(tang_buf.first + tang_data.buffer_offset)[i];
            if(_tex_map_accessor != UINT32_MAX)
                attr.tex0 = reinterpret_cast<const Vector2<float>*>(tex_map_buf.first + tex_map_data.buffer_offset)[i];
            if(_joints_accessor != UINT32_MAX)
                attr.joints = reinterpret_cast<const Vector4<uint32_t>*>(joints_buf.first + joints_data.buffer_offset)[i];
            if(_weights_accessor != UINT32_MAX)
                attr.weights = reinterpret_cast<const Vector4<float>*>(weights_buf.first + weights_data.buffer_offset)[i];

            // check if index exists in the map
            if(m.find(attr) != m.end())
                m_supplemented_indices.back().push_back(m[attr]);
            else {
                m[attr] = max;
                m_supplemented_indices.back().push_back(max);
                max++;
            }
        }

        std::array<std::pair<uint64_t, BufferType>, 6> attr_array = {
            std::make_pair(static_cast<uint64_t>(offsets[0]) | (static_cast<uint64_t>(vert_data.buffer_id) << 32), LIBDAS_BUFFER_TYPE_VERTEX),
            std::make_pair(static_cast<uint64_t>(offsets[1]) | (static_cast<uint64_t>(norm_data.buffer_id) << 32), LIBDAS_BUFFER_TYPE_VERTEX_NORMAL),
            std::make_pair(static_cast<uint64_t>(offsets[2]) | (static_cast<uint64_t>(tang_data.buffer_id) << 32), LIBDAS_BUFFER_TYPE_VERTEX_TANGENT),
            std::make_pair(static_cast<uint64_t>(offsets[3]) | (static_cast<uint64_t>(tex_map_data.buffer_id) << 32), LIBDAS_BUFFER_TYPE_TEXTURE_MAP),
            std::make_pair(static_cast<uint64_t>(offsets[4]) | (static_cast<uint64_t>(joints_data.buffer_id) << 32), LIBDAS_BUFFER_TYPE_JOINTS),
            std::make_pair(static_cast<uint64_t>(offsets[5]) | (static_cast<uint64_t>(weights_data.buffer_id) << 32), LIBDAS_BUFFER_TYPE_WEIGHTS)
        };

        _WriteIndexedData(_accessors, _buffers, m, m_supplemented_indices.back(), attr_array, max);
        _CreateNewIndexRegion(_root, _accessors[0], _prim, _buffers[0], m_supplemented_indices.back());
    }


    void GLTFCompiler::_IndexGeometry(GLTFRoot &_root, GLTFAccessors &_accessors, std::vector<DasBuffer> &_buffers) {
        for(size_t i = 0; i < m_unindexed_primitives.size(); i++) {
            for(auto it = m_unindexed_primitives[i].begin(); it != m_unindexed_primitives[i].end(); it++) {
                uint32_t vertex_accessor = UINT32_MAX;
                uint32_t vertex_normal_accessor = UINT32_MAX;
                uint32_t vertex_tangent_accessor = UINT32_MAX;
                uint32_t texture_map_accessor = UINT32_MAX;
                uint32_t joints_accessor = UINT32_MAX;
                uint32_t weights_accessor = UINT32_MAX;

                // for each primitive attribute
                for(auto attr_it = _root.meshes[i].primitives[*it].attributes.begin(); attr_it != _root.meshes[i].primitives[*it].attributes.end(); attr_it++) {
                    if(m_attribute_type_map.find(attr_it->first) == m_attribute_type_map.end()) {
                        std::cerr << "Invalid attribute '" << attr_it->first << "'" << std::endl;
                        std::exit(LIBDAS_ERROR_INVALID_DATA);
                    }

                    switch(m_attribute_type_map.find(attr_it->first)->second) {
                        case LIBDAS_BUFFER_TYPE_VERTEX:
                            vertex_accessor = attr_it->second;
                            break;

                        case LIBDAS_BUFFER_TYPE_VERTEX_NORMAL:
                            vertex_accessor = attr_it->second;
                            break;

                        case LIBDAS_BUFFER_TYPE_VERTEX_TANGENT:
                            vertex_tangent_accessor = attr_it->second;
                            break;

                        case LIBDAS_BUFFER_TYPE_TEXTURE_MAP:
                            texture_map_accessor = attr_it->second;
                            break;

                        case LIBDAS_BUFFER_TYPE_JOINTS:
                            joints_accessor = attr_it->second;
                            break;

                        case LIBDAS_BUFFER_TYPE_WEIGHTS:
                            weights_accessor = attr_it->second;
                            break;

                        default:
                            break;
                    }
                }
                
                _IndexMeshPrimitive(_root, _root.meshes[i].primitives[*it], _buffers, _accessors, vertex_accessor, vertex_normal_accessor, 
                                    vertex_tangent_accessor, texture_map_accessor, joints_accessor, weights_accessor);
            }
        }
    }


    void GLTFCompiler::_CreateNewIndexRegion(GLTFRoot &_root, std::vector<GLTFAccessor*> &_accessors, GLTFMeshPrimitive &_prim, DasBuffer &_buffer, std::vector<uint32_t> &_indices) {
        _root.buffer_views.emplace_back();
        _root.buffer_views.back().byte_offset = _buffer.data_len;
        _root.buffer_views.back().byte_length = static_cast<uint32_t>(_indices.size() * sizeof(uint32_t));

        _root.accessors.emplace_back();
        _root.accessors.back().buffer_view = static_cast<int32_t>(_root.buffer_views.size() - 1);
        _root.accessors.back().byte_offset = 0;
        _root.accessors.back().component_type = KHRONOS_UNSIGNED_INT;
        _root.accessors.back().count = static_cast<int32_t>(_indices.size());
        _root.accessors.back().type = "SCALAR";

        _accessors.push_back(&_root.accessors.back());

        _buffer.data_ptrs.push_back(std::make_pair(reinterpret_cast<const char*>(_indices.data()), _indices.size() * sizeof(uint32_t)));
        _buffer.data_len += static_cast<uint32_t>(_indices.size() * sizeof(uint32_t));

        _prim.indices = static_cast<int32_t>(_root.accessors.size() - 1);
    }

    
    void GLTFCompiler::_FreeSupplementedBuffers(std::vector<char*> _mem_areas) {
        for(auto it = _mem_areas.begin(); it != _mem_areas.end(); it++)
            delete [] *it;

        _mem_areas.clear();
    }


    void GLTFCompiler::_CheckAndSupplementProperties(const GLTFRoot &_root, DasProperties &_props) {
        if(_props.author == "")
            _props.author = _root.asset.generator;
        if(_props.copyright == "")
            _props.copyright = _root.asset.copyright;

        // check if default scene exists
        if(_root.load_time_scene)
            _props.default_scene = static_cast<uint32_t>(_root.load_time_scene);
    }


    void GLTFCompiler::_FlagBuffersAccordingToMeshes(const GLTFRoot &_root, std::vector<DasBuffer> &_buffers) {
        // for each mesh in root
        for(auto it = _root.meshes.begin(); it != _root.meshes.end(); it++) {
            // for each primitive in mesh
            for(size_t i = 0; i < it->primitives.size(); i++) {
                const int32_t index_buffer_id = _root.buffer_views[_root.accessors[it->primitives[i].indices].buffer_view].buffer;

                if(it->primitives[i].indices != INT32_MAX)
                    _buffers[index_buffer_id].type |= LIBDAS_BUFFER_TYPE_INDICES;

                // check into attributes
                for(auto map_it = it->primitives[i].attributes.begin(); map_it != it->primitives[i].attributes.end(); map_it++) {
                    if(m_attribute_type_map.find(map_it->first) == m_attribute_type_map.end()) {
                        std::cerr << "GLTF error:  No valid attribute '" << map_it->first << "' available for current implementation" << std::endl;
                        EXIT_ON_ERROR(1);
                    }

                    const int32_t buffer_id = _root.buffer_views[_root.accessors[map_it->second].buffer_view].buffer;
                    _buffers[buffer_id].type |= m_attribute_type_map.find(map_it->first)->second;
                }
            }
        }
    }


    void GLTFCompiler::_FlagBuffersAccordingToAnimations(const GLTFRoot &_root, std::vector<DasBuffer> &_buffers) {
        for(auto it = _root.animations.begin(); it != _root.animations.end(); it++) {
            for(size_t i = 0; i < it->samplers.size(); i++) {
                const int32_t input = _root.buffer_views[_root.accessors[it->samplers[i].input].buffer_view].buffer;
                const int32_t output = _root.buffer_views[_root.accessors[it->samplers[i].output].buffer_view].buffer;
                _buffers[input].type |= LIBDAS_BUFFER_TYPE_TIMESTAMPS;
                _buffers[output].type |= LIBDAS_BUFFER_TYPE_KEYFRAME;
            }
        }
    }


    std::vector<DasBuffer> GLTFCompiler::_CreateBuffers(GLTFRoot &_root, const std::vector<std::string> &_embedded_textures) {
        std::vector<DasBuffer> buffers;

        // append buffers
        for(auto it = _root.buffers.begin(); it != _root.buffers.end(); it++) {
            m_uri_resolvers.push_back(URIResolver(it->uri, m_root_path));
            DasBuffer buffer;
            buffer.type = LIBDAS_BUFFER_TYPE_UNKNOWN;
            buffer.data_len = it->byte_length;
            buffer.data_ptrs.push_back(m_uri_resolvers.back().GetBuffer());

            LIBDAS_ASSERT(buffer.data_len == buffer.data_ptrs.back().second);
            buffers.push_back(buffer);
        }

        _StrideBuffers(_root, buffers);

        // append images
        for(auto it = _root.images.begin(); it != _root.images.end(); it++) {
            // there are two possibilities:
            // 1. the image is defined with its uri
            // 2. the image is defined in some buffer view
            if(it->uri != "") {
                m_uri_resolvers.push_back(URIResolver(it->uri, m_root_path));
                DasBuffer buffer;
                buffer.type |= m_uri_resolvers.back().GetParsedDataType();
                buffer.data_len = m_uri_resolvers.back().GetBuffer().second;
                buffer.data_ptrs.push_back(m_uri_resolvers.back().GetBuffer());

                buffers.push_back(buffer);
            }
        }

        AppendTextures(buffers, _embedded_textures);
        _FlagBuffersAccordingToMeshes(_root, buffers);
        _FlagBuffersAccordingToAnimations(_root, buffers);

        return buffers;
    }


    std::vector<DasMorphTarget> GLTFCompiler::_CreateMorphTargets(const GLTFRoot &_root) {
        std::vector<DasMorphTarget> morph_targets;

        for(auto it = _root.meshes.begin(); it != _root.meshes.end(); it++) {
            for(auto primitive_it = it->primitives.begin(); primitive_it != it->primitives.end(); primitive_it++) {
                for(auto target_it = primitive_it->targets.begin(); target_it != primitive_it->targets.end(); target_it++) {

                    DasMorphTarget morph_target;
                    bool is_attr = false;

                    for(size_t i = 0; i < target_it->size(); i++) {
                        std::string no_nr = Algorithm::RemoveNumbers(target_it->at(i).first);

                        if(m_attribute_type_map.find(no_nr) == m_attribute_type_map.end()) {
                            std::cerr << "GLTF error: No valid morph target attribute '" << no_nr << "' available for current implementation" << std::endl;
                            EXIT_ON_ERROR(LIBDAS_ERROR_INVALID_DATA);
                        }

                        BufferAccessorData accessor_data = _FindAccessorData(_root, target_it->at(i).second);

                        switch(m_attribute_type_map.find(no_nr)->second) {
                            case LIBDAS_BUFFER_TYPE_VERTEX:
                                morph_target.vertex_buffer_id = accessor_data.buffer_id;
                                morph_target.vertex_buffer_offset = accessor_data.buffer_offset;
                                is_attr = true;
                                break;

                            case LIBDAS_BUFFER_TYPE_TEXTURE_MAP:
                                morph_target.texture_map_buffer_id = accessor_data.buffer_id;
                                morph_target.texture_map_buffer_offset = accessor_data.buffer_offset;
                                break;

                            case LIBDAS_BUFFER_TYPE_VERTEX_NORMAL:
                                morph_target.vertex_normal_buffer_id = accessor_data.buffer_id;
                                morph_target.vertex_normal_buffer_offset = accessor_data.buffer_offset;
                                is_attr = true;
                                break;

                            case LIBDAS_BUFFER_TYPE_VERTEX_TANGENT:
                                morph_target.vertex_tangent_buffer_id = accessor_data.buffer_id;
                                morph_target.vertex_tangent_buffer_offset = accessor_data.buffer_offset;
                                break;

                            default:
                                LIBDAS_ASSERT(false);
                                break;
                        }
                    }

                    // ignore empty targets
                    if(is_attr) morph_targets.push_back(morph_target);
                }
            }
        }

        return morph_targets;
    }


    std::vector<DasMeshPrimitive> GLTFCompiler::_CreateMeshPrimitives(const GLTFRoot &_root) {
        std::vector<DasMeshPrimitive> primitives;
        primitives.reserve(_FindPrimitiveCount(_root));
        uint32_t used_targets = 0, used_weights = 0;

        // for each mesh
        for(auto mesh_it = _root.meshes.begin(); mesh_it != _root.meshes.end(); mesh_it++) {
            // for each mesh primitive
            for(auto prim_it = mesh_it->primitives.begin(); prim_it != mesh_it->primitives.end(); prim_it++) {
                DasMeshPrimitive prim;

                // check if the primitive mode is correct
                if(prim_it->mode != KHRONOS_TRIANGLES) {
                    std::cerr << "Non-triangle geometry is not supported" << std::endl;
                    EXIT_ON_ERROR(LIBDAS_ERROR_INVALID_DATA);
                }

                // write index buffer data
                BufferAccessorData accessor_data = _FindAccessorData(_root, prim_it->indices);
                prim.index_buffer_id = accessor_data.buffer_id;
                prim.index_buffer_offset = accessor_data.buffer_offset;
                prim.indices_count = _root.accessors[prim_it->indices].count;
                prim.indexing_mode = LIBDAS_COMPACT_INDICES;

                // for each attribute write its data into mesh primitive structure
                for(auto attr_it = prim_it->attributes.begin(); attr_it != prim_it->attributes.end(); attr_it++) {
                    // no attribute found, display an error
                    if(m_attribute_type_map.find(attr_it->first) == m_attribute_type_map.end()) {
                        std::cerr << "Invalid attribute " << attr_it->first << std::endl;
                        EXIT_ON_ERROR(LIBDAS_ERROR_INVALID_DATA);
                    }

                    bool is_joints = false, is_weights = false;
                    accessor_data = _FindAccessorData(_root, attr_it->second);
                    switch(m_attribute_type_map.find(attr_it->first)->second) {
                        case LIBDAS_BUFFER_TYPE_VERTEX:
                            prim.vertex_buffer_id = accessor_data.buffer_id;
                            prim.vertex_buffer_offset = accessor_data.buffer_offset;
                            break;

                        case LIBDAS_BUFFER_TYPE_TEXTURE_MAP:
                            prim.texture_map_buffer_id = accessor_data.buffer_id;
                            prim.texture_map_buffer_offset = accessor_data.buffer_offset;
                            break;

                        case LIBDAS_BUFFER_TYPE_VERTEX_NORMAL:
                            prim.vertex_normal_buffer_id = accessor_data.buffer_id;
                            prim.vertex_normal_buffer_offset = accessor_data.buffer_offset;
                            break;

                        case LIBDAS_BUFFER_TYPE_VERTEX_TANGENT:
                            prim.vertex_tangent_buffer_id = accessor_data.buffer_id;
                            prim.vertex_tangent_buffer_offset = accessor_data.buffer_offset;
                            break;

                        case LIBDAS_BUFFER_TYPE_JOINTS:
                            if(is_joints) {
                                std::cerr << "Using more than 4 joints per node is not allowed" << std::endl;;
                                std::exit(LIBDAS_ERROR_INVALID_DATA);
                            }

                            is_joints = true;
                            prim.joint_index_buffer_id = accessor_data.buffer_id;
                            prim.joint_index_buffer_offset = accessor_data.buffer_offset;
                            break;

                        case LIBDAS_BUFFER_TYPE_WEIGHTS:
                            if(is_weights) {
                                std::cerr << "Using more than 4 joints per node is not allowed" << std::endl;
                                std::exit(LIBDAS_ERROR_INVALID_DATA);
                            }

                            is_weights = true;
                            prim.weight_buffer_id = accessor_data.buffer_id;
                            prim.weight_buffer_offset = accessor_data.buffer_offset;
                            break;

                        default:
                            break;
                    }

                    // set morph targets with their correct counts
                    prim.morph_target_count = static_cast<uint32_t>(prim_it->targets.size());
                    prim.morph_targets = new uint32_t[prim.morph_target_count];
                    for(uint32_t i = 0; i < prim.morph_target_count; i++, used_targets++) 
                        prim.morph_targets[i] = used_targets;

                    // weights are contained inside the mesh structure, where morph targets are contained in mesh.primitives structure
                    // this means that there can be multiple mesh.primitive objects with multiple morph targets in all of them
                    if(static_cast<uint32_t>(mesh_it->weights.size()) - used_weights == prim.morph_target_count) {
                        prim.morph_weights = new float[prim.morph_target_count];
                        std::memcpy(prim.morph_weights, mesh_it->weights.data(), static_cast<size_t>(prim.morph_target_count) * sizeof(float));
                    } else {
                        const std::vector<float> &&node_weights = _FindMorphWeightsFromNodes(_root, mesh_it - _root.meshes.begin());
                        if(static_cast<uint32_t>(node_weights.size()) == prim.morph_target_count) {
                            prim.morph_weights = new float[prim.morph_target_count];
                            std::memcpy(prim.morph_weights, mesh_it->weights.data(), static_cast<size_t>(prim.morph_target_count) * sizeof(float));
                        }
                    }
                }

                primitives.emplace_back(std::move(prim));
            }
        }

        return primitives;
    }


    std::vector<DasMesh> GLTFCompiler::_CreateMeshes(const GLTFRoot &_root) {
        std::vector<DasMesh> meshes;
        meshes.reserve(_root.meshes.size());
        uint32_t used_prims = 0;

        for(auto it = _root.meshes.begin(); it != _root.meshes.end(); it++) {
            DasMesh mesh;
            mesh.primitive_count = static_cast<uint32_t>(it->primitives.size());
            mesh.primitives = new uint32_t[mesh.primitive_count];

            for(uint32_t i = 0; i < mesh.primitive_count; i++, used_prims++)
                mesh.primitives[i] = used_prims;

            meshes.emplace_back(std::move(mesh));
        }

        return meshes;
    }


    std::vector<DasNode> GLTFCompiler::_CreateNodes(const GLTFRoot &_root) {
        std::vector<DasNode> nodes;
        nodes.reserve(_root.nodes.size());

        for(size_t i = 0; i < _root.nodes.size(); i++) {
            if(m_scene_node_id_table[i] == UINT32_MAX) continue;

            DasNode node;
            if(_root.nodes[i].name != "")
                node.name = _root.nodes[i].name;

            // write children objects
            node.children_count = static_cast<uint32_t>(_root.nodes[i].children.size());
            node.children = new uint32_t[node.children_count];

            uint32_t delta_c = 0;
            for(uint32_t j = 0; j < node.children_count; j++) {
                if(m_scene_node_id_table[_root.nodes[i].children[j]] == UINT32_MAX) {
                    delta_c++;
                    continue;
                }

                node.children[j - delta_c] = m_scene_node_id_table[_root.nodes[i].children[j]];
            }
            node.children_count -= delta_c;

            if(_root.nodes[i].mesh != INT32_MAX)
                node.mesh = _root.nodes[i].mesh;
            if(_root.nodes[i].skin != INT32_MAX)
                node.skeleton = _root.nodes[i].skin;

            const Matrix4<float> def_mat;
            if(_root.nodes[i].matrix != def_mat) {
                node.transform = _root.nodes[i].matrix;
            } else {
                node.transform = Matrix4<float> {
                    { _root.nodes[i].scale.x, 0.0f, 0.0f, _root.nodes[i].translation.x },
                    { 0.0f, _root.nodes[i].scale.y, 0.0f, _root.nodes[i].translation.y },
                    { 0.0f, 0.0f, _root.nodes[i].scale.z, _root.nodes[i].translation.z },
                    { 0.0f, 0.0f, 0.0f, 1.0f }
                };
                node.transform *= _root.nodes[i].rotation.ExpandToMatrix4();
            }

            nodes.emplace_back(std::move(node));
        }

        nodes.shrink_to_fit();
        return nodes;
    }


    std::vector<DasScene> GLTFCompiler::_CreateScenes(const GLTFRoot &_root) {
        std::vector<DasScene> scenes;
        scenes.reserve(_root.scenes.size());

        for(auto it = _root.scenes.begin(); it != _root.scenes.end(); it++) {
            DasScene scene;
            scene.name = it->name;
            scene.node_count = static_cast<uint32_t>(it->nodes.size());
            scene.nodes = new uint32_t[scene.node_count];

            uint32_t delta_c = 0;
            for(uint32_t i = 0; i < scene.node_count; i++) {
                if(m_scene_node_id_table[it->nodes[i]] == UINT32_MAX) {
                    delta_c++;
                    continue;
                }

                scene.nodes[i - delta_c] = m_scene_node_id_table[it->nodes[i]];
            }

            scene.node_count -= delta_c;

            scenes.emplace_back(std::move(scene));
        }

        return scenes;
    }


    std::vector<DasSkeleton> GLTFCompiler::_CreateSkeletons(const GLTFRoot &_root) {
        std::vector<DasSkeleton> skeletons;
        skeletons.reserve(_root.skins.size());

        for(size_t i = 0; i < _root.skins.size(); i++) {
            DasSkeleton skeleton;
            skeleton.name = _root.skins[i].name;

            if(_root.skins[i].skeleton != INT32_MAX)
                skeleton.parent = static_cast<uint32_t>(_root.skins[i].skeleton);
            else {
                if((skeleton.parent = _FindCommonRootJoint(_root, _root.skins[i])) == UINT32_MAX) {
                    std::cerr << "Could not find parent skeletal joint for skin " << i << std::endl;
                    EXIT_ON_ERROR(LIBDAS_ERROR_INVALID_DATA);
                }
            }

            skeleton.joint_count = static_cast<uint32_t>(_root.skins[i].joints.size());
            skeleton.joints = new uint32_t[skeleton.joint_count];
            for(uint32_t j = 0; j < skeleton.joint_count; j++)
                skeleton.joints[j] = m_skeleton_joint_id_table[_root.skins[i].joints[j]];

            skeletons.emplace_back(std::move(skeleton));
        }
        return skeletons;
    }


    std::vector<DasSkeletonJoint> GLTFCompiler::_CreateSkeletonJoints(const GLTFRoot &_root, const std::vector<DasBuffer> &_buffers) {
        std::vector<DasSkeletonJoint> joints;
        joints.reserve(_root.nodes.size());
        
        for(auto skin_it = _root.skins.begin(); skin_it != _root.skins.end(); skin_it++) {
            BufferAccessorData accessor_data = _FindAccessorData(_root, skin_it->inverse_bind_matrices);

            size_t ptr_offset = accessor_data.buffer_offset;
            auto it = _FindDataPtrFromOffset(_buffers[accessor_data.buffer_id].data_ptrs, ptr_offset);
            for(size_t i = 0; i < skin_it->joints.size(); i++) {
                if(m_skeleton_joint_id_table[skin_it->joints[i]] == UINT32_MAX) continue;

                DasSkeletonJoint joint;
                joint.inverse_bind_pos = reinterpret_cast<const Matrix4<float>*>(it->first + ptr_offset)[i];
                joint.rotation = _root.nodes[skin_it->joints[i]].rotation;

                // for each child in joint
                joint.children_count = static_cast<uint32_t>(_root.nodes[skin_it->joints[i]].children.size());
                joint.children = new uint32_t[joint.children_count];
                for(uint32_t j = 0; j < joint.children_count; j++)
                    joint.children[j] = m_skeleton_joint_id_table[_root.nodes[skin_it->joints[i]].children[j]];

                // non-uniform scaling is not supported, so just take the vector magnitude of scaling properties
                const float x = _root.nodes[skin_it->joints[i]].scale.x , y = _root.nodes[skin_it->joints[i]].scale.y, z = _root.nodes[skin_it->joints[i]].scale.z;
                joint.scale = sqrtf(x * x + y * y + z * z);
                joint.translation = _root.nodes[skin_it->joints[i]].translation;

                joints.emplace_back(std::move(joint));
            }
        }

        joints.shrink_to_fit();
        return joints;
    }

    std::vector<DasAnimationChannel> GLTFCompiler::_CreateAnimationChannels(const GLTFRoot &_root) {
        std::vector<DasAnimationChannel> channels;

        // assuming that there are 2 channels for each animation object
        channels.reserve(_root.animations.size() * 2);

        for(auto ani_it = _root.animations.begin(); ani_it != _root.animations.end(); ani_it++) {
            // for each channel in animation
            for(auto ch_it = ani_it->channels.begin(); ch_it != ani_it->channels.end(); ch_it++) {
                DasAnimationChannel channel;
                channel.node_id = static_cast<uint32_t>(ch_it->target.node);

                // check path value
                if(ch_it->target.path == "translation")
                    channel.target = LIBDAS_ANIMATION_TARGET_TRANSLATION;
                else if(ch_it->target.path == "rotation")
                    channel.target = LIBDAS_ANIMATION_TARGET_ROTATION;
                else if(ch_it->target.path == "scale")
                    channel.target = LIBDAS_ANIMATION_TARGET_SCALE;
                else if(ch_it->target.path == "weights")
                    channel.target = LIBDAS_ANIMATION_TARGET_WEIGHTS;

                // check interpolation value
                const GLTFAnimationSampler &sampler = ani_it->samplers[ch_it->sampler];
                if(sampler.interpolation == "LINEAR")
                    channel.interpolation = LIBDAS_INTERPOLATION_VALUE_LINEAR;
                else if(sampler.interpolation == "STEP")
                    channel.interpolation = LIBDAS_INTERPOLATION_VALUE_STEP;
                else if(sampler.interpolation == "CUBICSPLINE")
                    channel.interpolation = LIBDAS_INTERPOLATION_VALUE_CUBICSPLINE;

                // keyframe data
                channel.keyframe_count = static_cast<uint32_t>(_root.accessors[sampler.input].count);
                BufferAccessorData keyframe = _FindAccessorData(_root, sampler.input);
                channel.keyframe_buffer_id = keyframe.buffer_id;
                channel.keyframe_buffer_offset = keyframe.buffer_offset;
                
                // target data
                BufferAccessorData target = _FindAccessorData(_root, sampler.output);
                channel.target_value_buffer_id = target.buffer_id;
                channel.target_value_buffer_offset = target.buffer_offset;

                channels.emplace_back(std::move(channel));
            }
        }

        return channels;
    }


    std::vector<DasAnimation> GLTFCompiler::_CreateAnimations(const GLTFRoot &_root) {
        std::vector<DasAnimation> animations;
        animations.reserve(_root.animations.size());
        uint32_t channel_count = 0;

        for(auto ani_it = _root.animations.begin(); ani_it != _root.animations.end(); ani_it++) {
            DasAnimation ani;
            ani.name = ani_it->name;
            ani.channel_count = ani_it->channels.size();
            ani.channels = new uint32_t[ani.channel_count];
            for(uint32_t i = 0; i < ani.channel_count; i++, channel_count++)
                ani.channels[i] = channel_count;

            animations.emplace_back(std::move(ani));
        }
        return animations;
    }


    void GLTFCompiler::Compile(GLTFRoot &_root, const DasProperties &_props, const std::vector<std::string> &_embedded_textures, const std::string &_out_file) {
        // check if new file should be opened
        if(_out_file != "")
            NewFile(_out_file);

        _CheckAndSupplementProperties(const_cast<GLTFRoot&>(_root), const_cast<DasProperties&>(_props));
        InitialiseFile(_props);

        // write buffers to file
        std::vector<DasBuffer> buffers(std::move(_CreateBuffers(_root, _embedded_textures)));
        for(auto it = buffers.begin(); it != buffers.end(); it++)
            WriteBuffer(*it);

        // write mesh primitives to the file
        std::vector<DasMeshPrimitive> primitives(std::move(_CreateMeshPrimitives(_root)));
        for(auto it = primitives.begin(); it != primitives.end(); it++)
            WriteMeshPrimitive(*it);

        // write morph targets to the file
        std::vector<DasMorphTarget> morph_targets(std::move(_CreateMorphTargets(_root)));
        for(auto it = morph_targets.begin(); it != morph_targets.end(); it++)
            WriteMorphTarget(*it);

        // write meshes to the file
        std::vector<DasMesh> meshes(std::move(_CreateMeshes(_root)));
        for(auto it = meshes.begin(); it != meshes.end(); it++)
            WriteMesh(*it);

        // flag all skeleton joint nodes
        _FlagJointNodes(_root);

        // write scene nodes to the file
        std::vector<DasNode> nodes(std::move(_CreateNodes(_root))); 
        for(auto it = nodes.begin(); it != nodes.end(); it++)
            WriteNode(*it);

        // write scenes to the file
        std::vector<DasScene> scenes(std::move(_CreateScenes(_root)));
        for(auto it = scenes.begin(); it != scenes.end(); it++)
            WriteScene(*it);

        // write skeleton joints to the file
        std::vector<DasSkeletonJoint> joints(std::move(_CreateSkeletonJoints(_root, buffers)));
        for(auto it = joints.begin(); it != joints.end(); it++)
            WriteSkeletonJoint(*it);

        // write skeletons to the file
        std::vector<DasSkeleton> skeletons(std::move(_CreateSkeletons(_root)));
        for(auto it = skeletons.begin(); it != skeletons.end(); it++)
            WriteSkeleton(*it);

        // write animation channels to file
        std::vector<DasAnimationChannel> channels(std::move(_CreateAnimationChannels(_root)));
        for(auto it = channels.begin(); it != channels.end(); it++)
            WriteAnimationChannel(*it);

        // write animations to file
        std::vector<DasAnimation> animations(std::move(_CreateAnimations(_root)));
        for(auto it = animations.begin(); it != animations.end(); it++)
            WriteAnimation(*it);
    }
}
