// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: GLTFCompiler.cpp - GLTF format to DAS compiler implementation
// author: Karl-Mihkel Ott

#define GLTF_COMPILER_CPP
#include <GLTFCompiler.h>


// There is a fundamental error in buffer striding.
// Following BufferAccessorDatas do not get their offsets updated whenever Supplementation method is called


namespace Libdas {

    GLTFCompiler::GLTFCompiler(const std::string &_in_path, const std::string &_out_file, bool _use_raw_textures) : 
        DasWriterCore(_out_file), m_use_raw_textures(_use_raw_textures), m_root_path(_in_path) {}

    GLTFCompiler::GLTFCompiler(const std::string &_in_path, GLTFRoot &_root, const DasProperties &_props, 
                               const std::string &_out_file, const std::vector<std::string> &_embedded_textures, bool _use_raw_textures) : 
        m_use_raw_textures(_use_raw_textures), 
        m_root_path(_in_path)
    {
        Compile(_root, _props, _embedded_textures, _out_file);
    }


    GLTFCompiler::~GLTFCompiler() {
        _FreeSupplementedBuffers(m_supplemented_buffers);
    }


    uint32_t GLTFCompiler::_FindKhronosComponentSize(int32_t _component_type) {
        uint32_t component = 0;
        switch(_component_type) {
            case KHRONOS_BYTE:
            case KHRONOS_UNSIGNED_BYTE:
                component = static_cast<uint32_t>(sizeof(char));
                break;

            case KHRONOS_SHORT:
            case KHRONOS_UNSIGNED_SHORT:
                component = static_cast<uint32_t>(sizeof(short));
                break;

            case KHRONOS_UNSIGNED_INT:
                component = static_cast<uint32_t>(sizeof(unsigned int));
                break;

            case KHRONOS_FLOAT:
                component = static_cast<uint32_t>(sizeof(float));
                break;

            default:
                break;
        }

        return component;
    }


    GLTFCompiler::BufferAccessorData GLTFCompiler::_FindAccessorData(const GLTFRoot &_root, int32_t _accessor_id) {
        GLTFCompiler::BufferAccessorData accessor_data;
        accessor_data.buffer_id = static_cast<uint32_t>(_root.buffer_views[_root.accessors[_accessor_id].buffer_view].buffer);
        accessor_data.unit_stride = _root.buffer_views[_root.accessors[_accessor_id].buffer_view].byte_stride;

        if(!_root.accessors[_accessor_id].accumulated_offset) {
            accessor_data.buffer_offset = static_cast<uint32_t>(_root.buffer_views[_root.accessors[_accessor_id].buffer_view].byte_offset +
                                                                _root.accessors[_accessor_id].byte_offset);
        } else {
            accessor_data.buffer_offset = _root.accessors[_accessor_id].accumulated_offset;
        }

        // component type being 0 means, that the entire used buffer size calculation is comprimised
        uint32_t component_mul = _FindKhronosComponentSize(_root.accessors[_accessor_id].component_type);;
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


    /*****************************************/
    /***** Buffer region finder methods ******/
    /*****************************************/


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


    std::vector<std::vector<GLTFCompiler::BufferAccessorData>> GLTFCompiler::_GetInvalidStridedBufferRegions(GLTFRoot &_root) {
        std::vector<std::vector<BufferAccessorData>> invalid_strided_regions(_root.buffers.size());
        for(uint32_t i = 0; i < _root.accessors.size(); i++) {
            BufferAccessorData accessor_data = _FindAccessorData(_root, i);

            if(accessor_data.unit_stride != 0 && accessor_data.unit_size != accessor_data.unit_stride)
                invalid_strided_regions[accessor_data.buffer_id].push_back(accessor_data);
        }

        return invalid_strided_regions;
    }


    std::vector<std::vector<GLTFCompiler::BufferAccessorData>> GLTFCompiler::_GetBufferIndexRegions(GLTFRoot &_root) {
        std::vector<std::vector<BufferAccessorData>> index_regions(_root.buffers.size());

        // find all index regions
        for(auto it = _root.meshes.begin(); it != _root.meshes.end(); it++) {
            for(size_t i = 0; i < it->primitives.size(); i++) {
                // error if non-indexed geometry is used
                if(it->primitives[i].indices == INT32_MAX)
                    continue;

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


    std::vector<std::vector<GLTFCompiler::BufferAccessorData>> GLTFCompiler::_GetBufferColorStrideRegions(GLTFRoot &_root) {
        std::vector<std::vector<BufferAccessorData>> uv_regions(_root.buffers.size());

        for(auto mesh_it = _root.meshes.begin(); mesh_it != _root.meshes.end(); mesh_it++) {
            for(auto prim_it = mesh_it->primitives.begin(); prim_it != mesh_it->primitives.end(); prim_it++) {
                // for each mesh primitive attribute
                for(auto attr_it = prim_it->attributes.begin(); attr_it != prim_it->attributes.end(); attr_it++) {
                    const std::string no_nr = Algorithm::RemoveNumbers(attr_it->first);
                    if(m_attribute_type_map.find(no_nr) == m_attribute_type_map.end()) {
                        std::cerr << "Invalid vertex attribute '" << attr_it->first << "'" << std::endl;
                        std::exit(LIBDAS_ERROR_INVALID_KEYWORD);
                    }

                    if(m_attribute_type_map.find(no_nr)->second == LIBDAS_BUFFER_TYPE_TEXTURE_MAP ||
                       m_attribute_type_map.find(no_nr)->second == LIBDAS_BUFFER_TYPE_COLOR) {
                        BufferAccessorData accessor_data = _FindAccessorData(_root, attr_it->second);
                        if(accessor_data.component_type == KHRONOS_UNSIGNED_BYTE || accessor_data.component_type == KHRONOS_UNSIGNED_SHORT)
                            uv_regions[accessor_data.buffer_id].push_back(accessor_data);
                    } 
                }

                // for each mesh primitive morph target
                for(auto tar_it = prim_it->targets.begin(); tar_it != prim_it->targets.end(); tar_it++) {
                    for(auto attr_it = tar_it->begin(); attr_it != tar_it->end(); attr_it++) {
                        const std::string no_nr = Algorithm::RemoveNumbers(attr_it->first);
                        if(m_attribute_type_map.find(no_nr) == m_attribute_type_map.end()) {
                            std::cerr << "Invalid vertex attribute '" << attr_it->first << "'" << std::endl;
                            std::exit(LIBDAS_ERROR_INVALID_KEYWORD);
                        }

                        if(m_attribute_type_map.find(no_nr)->second == LIBDAS_BUFFER_TYPE_TEXTURE_MAP ||
                           m_attribute_type_map.find(no_nr)->second == LIBDAS_BUFFER_TYPE_COLOR) {
                            BufferAccessorData accessor_data = _FindAccessorData(_root, attr_it->second);
                            if(accessor_data.component_type == KHRONOS_UNSIGNED_BYTE || accessor_data.component_type == KHRONOS_UNSIGNED_SHORT)
                                uv_regions[accessor_data.buffer_id].push_back(accessor_data);
                        }
                    }
                }
            }
        }

        return uv_regions;
    }


    std::vector<std::vector<GLTFCompiler::BufferAccessorData>> GLTFCompiler::_GetBufferJointRegions(GLTFRoot &_root) {
        std::vector<std::vector<BufferAccessorData>> joint_regions(_root.buffers.size());

        // find all joint regions
        for(auto mesh_it = _root.meshes.begin(); mesh_it != _root.meshes.end(); mesh_it++) {
            for(auto prim_it = mesh_it->primitives.begin(); prim_it != mesh_it->primitives.end(); prim_it++) {
                // find JOINTS_ objects
                for(auto attr_it = prim_it->attributes.begin(); attr_it != prim_it->attributes.end(); attr_it++) {
                    const std::string attr_wo_num = Algorithm::RemoveNumbers(attr_it->first);
                    if(attr_wo_num == "JOINTS_") {
                        BufferAccessorData accessor_data = _FindAccessorData(_root, attr_it->second);
                        if(accessor_data.component_type != KHRONOS_UNSIGNED_SHORT)
                            joint_regions[accessor_data.buffer_id].push_back(accessor_data);
                    }
                }
            }
        }

        return joint_regions;
    }


    std::vector<std::vector<GLTFCompiler::BufferAccessorData>> GLTFCompiler::_GetBufferWeightRegions(GLTFRoot &_root) {
        std::vector<std::vector<BufferAccessorData>> weight_regions(_root.buffers.size());

        // find all weight regions
        for(auto mesh_it = _root.meshes.begin(); mesh_it != _root.meshes.end(); mesh_it++) {
            for(auto prim_it = mesh_it->primitives.begin(); prim_it != mesh_it->primitives.end(); prim_it++) {
                // find WEIGHTS_ objects
                for(auto attr_it = prim_it->attributes.begin(); attr_it != prim_it->attributes.end(); attr_it++) {
                    const std::string attr_wo_num = Algorithm::RemoveNumbers(attr_it->first);
                    if(attr_wo_num == "WEIGHTS_") {
                        BufferAccessorData accessor_data = _FindAccessorData(_root, attr_it->second);
                        if(accessor_data.component_type != KHRONOS_FLOAT)
                            weight_regions[accessor_data.buffer_id].push_back(accessor_data);
                    }
                }
            }
        }

        return weight_regions;
    }


    std::vector<std::vector<GLTFCompiler::BufferAccessorData>> GLTFCompiler::_GetAnimationDataRegions(GLTFRoot &_root) {
        // first inner element: keyframe input
        // second inner element: keyframe output
        std::vector<std::vector<GLTFCompiler::BufferAccessorData>> regions;
        regions.resize(_root.buffers.size());

        // for each animation
        for(auto ani_it = _root.animations.begin(); ani_it != _root.animations.end(); ani_it++) {
            // for each animation channel
            for(auto ch_it = ani_it->channels.begin(); ch_it != ani_it->channels.end(); ch_it++) {
                LIBDAS_ASSERT(ch_it->sampler < static_cast<int32_t>(ani_it->samplers.size()));
                GLTFAnimationSampler &sampler = ani_it->samplers[ch_it->sampler];

                auto input = _FindAccessorData(_root, sampler.input);
                auto output = _FindAccessorData(_root, sampler.output);
                regions[input.buffer_id].push_back(input);
                regions[output.buffer_id].push_back(output);
            }
        }

        return regions;
    }


    /**********************************************/
    /***** End of buffer data regions finders *****/
    /**********************************************/


    void GLTFCompiler::_GetUnindexedMeshPrimitives(GLTFRoot &_root) {
        // find all unindexed mesh primitives
        m_unindexed_primitives.resize(_root.meshes.size());
        for(size_t i = 0; i < _root.meshes.size(); i++) {
            uint32_t j = 0;
            for(auto prim_it = _root.meshes[i].primitives.begin(); prim_it != _root.meshes[i].primitives.end(); prim_it++, j++) {
                if(prim_it->indices == INT32_MAX)
                    m_unindexed_primitives[i].push_back(j);
            }
        }
    }


    uint32_t GLTFCompiler::_CorrectOffsets(GLTFRoot &_root, std::vector<GLTFAccessor*> &_accessors, uint32_t _diff, size_t _offset, DasBuffer &_buffer) {
        bool is_prev_unaligned = false;
        uint32_t accumulated_pad = 0;
        for(GLTFAccessor *accessor : _accessors) {
            if(accessor->accumulated_offset > _offset) {
                if(is_prev_unaligned) {
                    const uint32_t id = static_cast<uint32_t>(accessor - _root.accessors.data());
                    BufferAccessorData accessor_data = _FindAccessorData(_root, id);

                    uint32_t component_size = _FindKhronosComponentSize(accessor_data.component_type);

                    // check if offset alignment is necessary
                    if((accessor_data.buffer_offset + _diff) % component_size) {
                        uint32_t pad_size = component_size - ((accessor_data.buffer_offset + _diff) % component_size);
                        _buffer.data_ptrs.push_back(std::make_pair(m_pad, pad_size));
                        _buffer.data_len += pad_size;
                        _diff += pad_size;
                        accumulated_pad += pad_size;
                    }
                }

                accessor->byte_offset += _diff;
                accessor->accumulated_offset += _diff;
                is_prev_unaligned = false;
            } else {
                is_prev_unaligned = true;
            }
        }

        return accumulated_pad;
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


    uint32_t GLTFCompiler::_FindMultiSpecVertexAttributeIndex(const std::string &_attr_name) {
        const std::string texcoord_str = "TEXCOORD_";
        const std::string color_str = "COLOR_";
        const std::string joints_str = "JOINTS_";
        const std::string weights_str = "WEIGHTS_";

        uint32_t idx = UINT32_MAX;
        if(_attr_name.find(texcoord_str) == 0)
            idx = std::stoi(_attr_name.substr(texcoord_str.size()));
        else if(_attr_name.find(color_str) == 0)
            idx = std::stoi(_attr_name.substr(color_str.size()));
        else if(_attr_name.find(joints_str) == 0)
            idx = std::stoi(_attr_name.substr(joints_str.size()));
        else if(_attr_name.find(weights_str) == 0)
            idx = std::stoi(_attr_name.substr(weights_str.size()));

        LIBDAS_ASSERT(idx != UINT32_MAX);
        return idx;
    }


    uint32_t GLTFCompiler::_EnumerateMultiSpecVertexAttributes(const GLTFMeshPrimitive::AttributesType &_attrs, const std::string &_attr_name_core) {
        uint32_t count = 0, max = 0;
        for(auto it = _attrs.begin(); it != _attrs.end(); it++) {
            if(it->first.find(_attr_name_core) != std::string::npos) {
                count++;
                const uint32_t tmp = _FindMultiSpecVertexAttributeIndex(it->first);
                if(tmp >= max)
                    max = tmp;
            }
        }

        LIBDAS_ASSERT(!count || max == count - 1);
        return count;
    }


    void GLTFCompiler::_FindMultiSpecVertexAttributeAccessors(const GLTFMeshPrimitive::AttributesType &_attrs) {
        const std::string texcoord_str = "TEXCOORD_";
        const std::string color_str = "COLOR_";
        const std::string joints_str = "JOINTS_";
        const std::string weights_str = "WEIGHTS_";

        // TODO: Find TEXCOORD_n, COLOR_n, JOINTS_n and WEIGHTS_n counts and associated accessors
        m_texcoord_accessors.resize(_EnumerateMultiSpecVertexAttributes(_attrs, "TEXCOORD_"));
        m_colormul_accessors.resize(_EnumerateMultiSpecVertexAttributes(_attrs, "COLOR_"));
        m_joints_accessors.resize(_EnumerateMultiSpecVertexAttributes(_attrs, "JOINTS_"));
        m_weights_accessors.resize(_EnumerateMultiSpecVertexAttributes(_attrs, "WEIGHTS_"));

        // error: weights and joints don't match
        if(m_joints_accessors.size() != m_weights_accessors.size()) {
            std::cerr << "Vertex joints count doesn't match vertex joint weights count" << std::endl;
            EXIT_ON_ERROR(LIBDAS_ERROR_INVALID_DATA_LENGTH);
        }

        for(auto it = _attrs.begin(); it != _attrs.end(); it++) {
            if(it->first.find(texcoord_str) == 0) {
                uint32_t index = _FindMultiSpecVertexAttributeIndex(it->first);
                m_texcoord_accessors[index] = it->second;
            } else if(it->first.find(color_str) == 0) {
                uint32_t index = _FindMultiSpecVertexAttributeIndex(it->first);
                m_colormul_accessors[index] = it->second;
            } else if(it->first.find(joints_str) == 0) {
                uint32_t index = _FindMultiSpecVertexAttributeIndex(it->first);
                m_joints_accessors[index] = it->second;
            } else if(it->first.find(weights_str) == 0) {
                uint32_t index = _FindMultiSpecVertexAttributeIndex(it->first);
                m_weights_accessors[index] = it->second;
            }
        }
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
                std::exit(LIBDAS_ERROR_INVALID_DATA_LENGTH);
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


    /*********************************************/
    /***** Universal supplementation methods *****/
    /*********************************************/

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
        m_supplemented_buffers[_suppl_info.buffer_id].push_back(buf);
        return static_cast<uint32_t>(diff);
    }


    uint32_t GLTFCompiler::_SupplementInvalidStridedData(const char *_odata, BufferAccessorData &_suppl_info, DasBuffer &_buffer) {
        char *buf = new char[_suppl_info.used_size];
        size_t len = static_cast<size_t>(_suppl_info.used_size);
        const size_t diff = _suppl_info.used_size - _suppl_info.unit_stride * (_suppl_info.used_size / _suppl_info.unit_size);

        size_t rd_offset = _suppl_info.buffer_offset;
        for(uint32_t i = 0; i < _suppl_info.used_size / _suppl_info.unit_size; i++) {
            const char *const rd_ptr = _odata + rd_offset;
            std::memcpy(buf + i * _suppl_info.unit_size, rd_ptr, _suppl_info.unit_size);
            rd_offset += _suppl_info.unit_stride;
        }

        _buffer.data_ptrs.push_back(std::make_pair(buf, len));
        _buffer.data_len += diff;
        return diff;
    }


    uint32_t GLTFCompiler::_SupplementColorMultipliers(const char *_odata, BufferAccessorData &_suppl_info, DasBuffer &_buffer) {
        char *buf = nullptr;
        size_t len = 0;
        size_t diff = 0;

        switch(_suppl_info.component_type) {
            case KHRONOS_UNSIGNED_BYTE:
                len = (static_cast<size_t>(_suppl_info.used_size) / sizeof(unsigned char));
                buf = new char[len * sizeof(float)];
                diff = len * sizeof(float) - len * sizeof(unsigned char);

                for(size_t i = 0; i < len; i++) {
                    unsigned char val = reinterpret_cast<const unsigned char*>(_odata + _suppl_info.buffer_offset)[i];
                    reinterpret_cast<float*>(buf)[i] = static_cast<float>(val) / 255.0f;
                }
                break;

            case KHRONOS_UNSIGNED_SHORT:
                len = static_cast<size_t>(_suppl_info.used_size) / sizeof(unsigned short);
                buf = new char[len * sizeof(float)];
                diff = len * sizeof(float) - len * sizeof(unsigned short);

                for(size_t i = 0; i < len; i++) {
                    unsigned short val = reinterpret_cast<const unsigned short*>(_odata + _suppl_info.buffer_offset)[i];
                    reinterpret_cast<float*>(buf)[i] = static_cast<float>(val) / UINT16_MAX;
                }
                break;

            default:
                LIBDAS_ASSERT(false);
                break;
        }

        _buffer.data_ptrs.push_back(std::make_pair(buf, len));
        _buffer.data_len += diff;
        m_supplemented_buffers[_suppl_info.buffer_id].push_back(buf);
        return static_cast<uint32_t>(diff);
    }


    uint32_t GLTFCompiler::_SupplementJointIndices(const char *_odata, BufferAccessorData &_suppl_info, DasBuffer &_buffer) {
        char *buf = nullptr;
        size_t len = 0; // units
        size_t diff = 0;

        switch(_suppl_info.component_type) {
            case KHRONOS_UNSIGNED_BYTE:
                len = (static_cast<size_t>(_suppl_info.used_size) / sizeof(unsigned char));
                buf = new char[len * sizeof(uint16_t)];
                diff = len * sizeof(uint16_t) - len * sizeof(unsigned char);

                for(size_t i = 0; i < len; i++)
                    reinterpret_cast<uint16_t*>(buf)[i] = static_cast<uint16_t>(reinterpret_cast<const unsigned char*>(_odata)[i]);
                break;

            default:
                LIBDAS_ASSERT(false);
                break;
        }

        _buffer.data_ptrs.push_back(std::make_pair(reinterpret_cast<const char*>(buf), len * sizeof(uint32_t)));
        _buffer.data_len += static_cast<uint32_t>(diff);
        m_supplemented_buffers[_suppl_info.buffer_id].push_back(buf);
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
                    reinterpret_cast<float*>(buf)[i] = static_cast<float>(reinterpret_cast<const unsigned short*>(_odata)[i]) / UINT16_MAX;
                break;

            default:
                LIBDAS_ASSERT(false);
                break;
        }

        _buffer.data_ptrs.push_back(std::make_pair(reinterpret_cast<const char*>(buf), len * sizeof(float)));
        _buffer.data_len += static_cast<uint32_t>(diff);
        m_supplemented_buffers[_suppl_info.buffer_id].push_back(buf);
        return diff;
    }


    uint32_t GLTFCompiler::_SupplementAnimationKeyframeData(const char *_odata, BufferAccessorData &_suppl_info, DasBuffer &_buffer) {
        const uint32_t diff = -_suppl_info.used_size;
        _buffer.data_len += diff;
        return diff;
    }

    /************************************************/
    /***** End of universal supplementation methods *****/
    /************************************************/


    /************************************************/
    /***** Indexed data supplementation methods *****/
    /************************************************/


    uint32_t GLTFCompiler::_SupplementIndexedPositionVertices(const char *_odata, BufferAccessorData &_suppl_info, DasBuffer &_buffer) {
        LIBDAS_ASSERT(_odata);
        const size_t len = m_indexed_attributes.size() * sizeof(Vector3<float>);
        char *buf = new char[len];
        size_t offset = 0;
        uint32_t diff = static_cast<uint32_t>(len) - _suppl_info.used_size;

        // copy data over
        for(auto it = m_indexed_attributes.begin(); it != m_indexed_attributes.end(); it++) {
            *reinterpret_cast<Vector3<float>*>(buf + offset) = it->pos;
            offset += static_cast<uint32_t>(sizeof(Vector3<float>));
        }

        _buffer.data_ptrs.push_back(std::make_pair(buf, len));
        _buffer.data_len += diff;
        m_supplemented_buffers[_suppl_info.buffer_id].push_back(buf);

        return diff;
    }


    uint32_t GLTFCompiler::_SupplementIndexedVertexNormals(const char *_odata, BufferAccessorData &_suppl_info, DasBuffer &_buffer) {
        LIBDAS_ASSERT(_odata);
        const size_t len = m_indexed_attributes.size() * sizeof(Vector3<float>);
        char *buf = new char[len];
        size_t offset = 0;
        uint32_t diff = static_cast<uint32_t>(len) - _suppl_info.used_size;

        // copy data over
        for(auto it = m_indexed_attributes.begin(); it != m_indexed_attributes.end(); it++) {
            *reinterpret_cast<Vector3<float>*>(buf + offset) = it->normal;
            offset += static_cast<uint32_t>(sizeof(Vector3<float>));
        }

        _buffer.data_ptrs.push_back(std::make_pair(buf, len));
        _buffer.data_len += diff;
        m_supplemented_buffers[_suppl_info.buffer_id].push_back(buf);

        return diff;
    }


    uint32_t GLTFCompiler::_SupplementIndexedVertexTangents(const char *_odata, BufferAccessorData &_suppl_info, DasBuffer &_buffer) {
        LIBDAS_ASSERT(_odata);
        const size_t len = m_indexed_attributes.size() * sizeof(Vector4<float>);
        char *buf = new char[len];
        size_t offset = 0;
        uint32_t diff = static_cast<uint32_t>(len) - _suppl_info.used_size;

        // copy data over
        for(auto it = m_indexed_attributes.begin(); it != m_indexed_attributes.end(); it++) {
            *reinterpret_cast<Vector4<float>*>(buf + offset) = it->tangent;
            offset += static_cast<uint32_t>(sizeof(Vector4<float>));
        }

        _buffer.data_ptrs.push_back(std::make_pair(buf, len));
        _buffer.data_len += diff;
        m_supplemented_buffers[_suppl_info.buffer_id].push_back(buf);

        return diff;
    }


    uint32_t GLTFCompiler::_SupplementIndexedUVVertices(const char *_odata, BufferAccessorData &_suppl_info, DasBuffer &_buffer) {
        LIBDAS_ASSERT(_odata);
        const size_t len = m_indexed_attributes.size() * sizeof(Vector2<float>);
        char *buf = new char[len];
        size_t offset = 0;
        uint32_t diff = static_cast<uint32_t>(len) - _suppl_info.used_size;

        // copy data over
        for(auto it = m_indexed_attributes.begin(); it != m_indexed_attributes.end(); it++) {
            *reinterpret_cast<Vector2<float>*>(buf + offset) = it->uv[m_multi_spec_vertex_attribute_index];
            offset += static_cast<uint32_t>(sizeof(Vector2<float>));
        }

        _buffer.data_ptrs.push_back(std::make_pair(buf, len));
        _buffer.data_len += diff;
        m_supplemented_buffers[_suppl_info.buffer_id].push_back(buf);

        return diff;
    }


    uint32_t GLTFCompiler::_SupplementIndexedColorMultipliers(const char *_odata, BufferAccessorData &_suppl_info, DasBuffer &_buffer) {
        LIBDAS_ASSERT(_odata);
        const size_t len = m_indexed_attributes.size() * sizeof(Vector4<float>);
        char *buf = new char[len];
        size_t offset = 0;
        uint32_t diff = static_cast<uint32_t>(len) - _suppl_info.used_size;

        // copy data over
        for(auto it = m_indexed_attributes.begin(); it != m_indexed_attributes.end(); it++) {
            *reinterpret_cast<Vector4<float>*>(buf + offset) = it->color[m_multi_spec_vertex_attribute_index];
            offset += static_cast<uint32_t>(sizeof(Vector4<float>));
        }

        _buffer.data_ptrs.push_back(std::make_pair(buf, len));
        _buffer.data_len += diff;
        m_supplemented_buffers[_suppl_info.buffer_id].push_back(buf);

        return diff;
    }


    uint32_t GLTFCompiler::_SupplementIndexedJointsIndices(const char *_odata, BufferAccessorData &_suppl_info, DasBuffer &_buffer) {
        LIBDAS_ASSERT(_odata);
        const size_t len = m_indexed_attributes.size() * sizeof(Vector4<uint16_t>);
        char *buf = new char[len];
        size_t offset = 0;
        uint32_t diff = static_cast<uint32_t>(len) - _suppl_info.used_size;

        // copy data over
        for(auto it = m_indexed_attributes.begin(); it != m_indexed_attributes.end(); it++) {
            *reinterpret_cast<Vector4<uint16_t>*>(buf + offset) = it->joints[m_multi_spec_vertex_attribute_index];
            offset += static_cast<uint32_t>(sizeof(Vector4<uint16_t>));
        }

        _buffer.data_ptrs.push_back(std::make_pair(buf, len));
        _buffer.data_len += diff;
        m_supplemented_buffers[_suppl_info.buffer_id].push_back(buf);

        return diff;
    }


    uint32_t GLTFCompiler::_SupplementIndexedJointWeights(const char *_odata, BufferAccessorData &_suppl_info, DasBuffer &_buffer) {
        LIBDAS_ASSERT(_odata);
        const size_t len = m_indexed_attributes.size() * sizeof(Vector4<float>);
        char *buf = new char[len];
        size_t offset = 0;
        uint32_t diff = static_cast<uint32_t>(len) - _suppl_info.used_size;

        // copy data over
        for(auto it = m_indexed_attributes.begin(); it != m_indexed_attributes.end(); it++) {
            *reinterpret_cast<Vector4<float>*>(buf + offset) = it->weights[m_multi_spec_vertex_attribute_index];
            offset += static_cast<uint32_t>(sizeof(Vector4<float>));
        }

        _buffer.data_ptrs.push_back(std::make_pair(buf, len));
        _buffer.data_len += diff;
        m_supplemented_buffers[_suppl_info.buffer_id].push_back(buf);

        return diff;
    }

    /*******************************************************/
    /***** End of indexed data supplementation methods *****/
    /*******************************************************/


    void GLTFCompiler::_AdjustBufferViewStrides(GLTFRoot &_root) {
        for(auto it = _root.buffer_views.begin(); it != _root.buffer_views.end(); it++)
            it->byte_stride = 0;
    }


    void GLTFCompiler::_CopyToBuffer(const std::vector<std::pair<const char*, size_t>> &_optrs, char *_dst, size_t _len, size_t _offset, DasBuffer &_buffer, uint32_t _buffer_id) {
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
        m_supplemented_buffers[_buffer_id].push_back(_dst);
    }


    void GLTFCompiler::_StrideBuffer(GLTFRoot &_root, GLTFCompiler::GLTFAccessors &_accessors, GLTFCompiler::BufferAccessorDatas &_regions, std::vector<DasBuffer> &_buffers, Supplement_PFN _suppl_fn) {
        std::vector<std::vector<char*>> prev_suppl(_buffers.size());
        bool is_aug = false;

        // for each buffer with index regions, supplement its data
        for(size_t i = 0; i < _regions.size(); i++) {
            uint32_t accumulated_diff = 0;
            uint32_t olen = _buffers[i].data_len; 
            // sort by offset size
            std::sort(_regions[i].begin(), _regions[i].end(), BufferAccessorData::less());
            Algorithm::RemoveDuplicates(_regions[i], BufferAccessorData::IsDuplicate);

            if(!_regions[i].size())
                continue;

            prev_suppl[i] = m_supplemented_buffers[i];
            m_supplemented_buffers[i].clear();
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

                    _CopyToBuffer(optrs, buf, len, offset, _buffers[i], i);
                    buf = nullptr;
                    len = 0;
                }

                // copy the area between areas
                else if(_regions[i][j - 1].buffer_offset + _regions[i][j - 1].used_size < _regions[i][j].buffer_offset) {
                    if(_regions[i][j - 1].unit_stride > _regions[i][j - 1].unit_size)
                        offset = _regions[i][j - 1].buffer_offset + _regions[i][j - 1].used_size;
                    else offset = _regions[i][j - 1].buffer_offset + (_regions[i][j - 1].used_size / _regions[i][j - 1].unit_size) * _regions[i][j - 1].unit_stride;
                    len = _regions[i][j].buffer_offset - offset;
                    buf = new char[len];

                    _CopyToBuffer(optrs, buf, len, offset, _buffers[i], i);
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
                diff += _CorrectOffsets(_root, _accessors[_regions[i][j].buffer_id], diff, _regions[i][j].buffer_offset + accumulated_diff, _buffers[i]);
                accumulated_diff += diff;

                // copy the area between last element and the end of the buffer
                if(j == _regions[i].size() - 1 && _regions[i][j].buffer_offset + _regions[i][j].used_size < olen) {
                    if(_regions[i][j].unit_stride > _regions[i][j].unit_size) 
                        offset = _regions[i][j].buffer_offset + (_regions[i][j].used_size / _regions[i][j].unit_size) * _regions[i][j].unit_stride;
                    else offset = _regions[i][j].buffer_offset + _regions[i][j].used_size;
                    len = olen - offset;
                    buf = new char[len];

                    _CopyToBuffer(optrs, buf, len, offset, _buffers[i], i);
                    buf = nullptr;
                    len = 0;
                    offset = 0;
                }
            }
        }

        if(is_aug) _FreeSupplementedBuffers(prev_suppl);
    }


    // TODO: add accessor buffer offset correction to the implementation
    // right now only supplementation is done
    void GLTFCompiler::_StrideBuffers(GLTFRoot &_root, std::vector<DasBuffer> &_buffers) {
        std::vector<std::vector<GLTFAccessor*>> all_regions(_GetAllBufferAccessorRegions(_root));
        m_supplemented_buffers.resize(_buffers.size());

        std::vector<std::vector<BufferAccessorData>> invalid_stride_regions(_GetInvalidStridedBufferRegions(_root));
        _StrideBuffer(_root, all_regions, invalid_stride_regions, _buffers, &GLTFCompiler::_SupplementInvalidStridedData);

        _AdjustBufferViewStrides(_root);

        std::vector<std::vector<BufferAccessorData>> index_regions(_GetBufferIndexRegions(_root));
        _StrideBuffer(_root, all_regions, index_regions, _buffers, &GLTFCompiler::_SupplementIndices);

        std::vector<std::vector<BufferAccessorData>> color_stride_regions(_GetBufferColorStrideRegions(_root));
        _StrideBuffer(_root, all_regions, color_stride_regions, _buffers, &GLTFCompiler::_SupplementColorMultipliers);

        std::vector<std::vector<BufferAccessorData>> joint_regions(_GetBufferJointRegions(_root));
        _StrideBuffer(_root, all_regions, joint_regions, _buffers, &GLTFCompiler::_SupplementJointIndices);

        std::vector<std::vector<BufferAccessorData>> joint_weight_regions(_GetBufferJointRegions(_root));
        _StrideBuffer(_root, all_regions, joint_weight_regions, _buffers, &GLTFCompiler::_SupplementJointWeights);

        std::vector<std::vector<BufferAccessorData>> animation_regions(_GetAnimationDataRegions(_root));
        _StrideBuffer(_root, all_regions, animation_regions, _buffers, &GLTFCompiler::_SupplementAnimationKeyframeData);

        _GetUnindexedMeshPrimitives(_root);
        _IndexGeometry(_root, all_regions, _buffers);
    }


    void GLTFCompiler::_OmitEmptyBuffers(GLTFRoot &_root, std::vector<DasBuffer> &_buffers) {
        std::vector<int32_t> buffer_id_offset_table(_buffers.size());
        std::fill(buffer_id_offset_table.begin(), buffer_id_offset_table.end(), 0);

        // omit unnecessary empty buffers
        for(size_t i = 0; i < _buffers.size(); i++) {
            if(!_buffers[i].data_len) {
                _buffers.erase(_buffers.begin() + i);

                for(size_t j = i; j < _buffers.size(); j++)
                    buffer_id_offset_table[j]--;

                i--;
            }
        }


        // change buffer ids
        for(size_t i = 0; i < _root.buffer_views.size(); i++)
            _root.buffer_views[i].buffer += buffer_id_offset_table[i];
    }


    void GLTFCompiler::_WriteIndexedData(GLTFRoot &_root, std::vector<DasBuffer> &_buffers, GLTFCompiler::GenericVertexAttributeAccessors &_gen_acc) {
        std::vector<std::vector<BufferAccessorData>> indexed_regions(_root.buffers.size());
        std::vector<std::vector<GLTFAccessor*>> all_regions(_GetAllBufferAccessorRegions(_root));
        
        // push position accessor
        indexed_regions[_gen_acc.pos_accessor.buffer_id].push_back(_gen_acc.pos_accessor);
        _StrideBuffer(_root, all_regions, indexed_regions, _buffers, &GLTFCompiler::_SupplementIndexedPositionVertices);
        indexed_regions[_gen_acc.pos_accessor.buffer_id].clear();

        // push vertex normal accessor
        if(_gen_acc.normal_accessor.buffer_id != UINT32_MAX) {
            indexed_regions[_gen_acc.normal_accessor.buffer_id].push_back(_gen_acc.normal_accessor);
            _StrideBuffer(_root, all_regions, indexed_regions, _buffers, &GLTFCompiler::_SupplementIndexedVertexNormals);
            indexed_regions[_gen_acc.normal_accessor.buffer_id].clear();
        }

        // push vertex tangent accessor
        if(_gen_acc.tangent_accessor.buffer_id != UINT32_MAX) {
            indexed_regions[_gen_acc.tangent_accessor.buffer_id].push_back(_gen_acc.tangent_accessor);
            _StrideBuffer(_root, all_regions, indexed_regions, _buffers, &GLTFCompiler::_SupplementIndexedVertexTangents);
            indexed_regions[_gen_acc.tangent_accessor.buffer_id].clear();
        }

        // push uv accessors
        for(auto it = _gen_acc.uv_accessors.begin(); it != _gen_acc.uv_accessors.end(); it++)
            indexed_regions[it->buffer_id].push_back(*it);
        _StrideBuffer(_root, all_regions, indexed_regions, _buffers, &GLTFCompiler::_SupplementIndexedUVVertices);
        indexed_regions.clear();
        indexed_regions.resize(_root.buffers.size());

        // push color accessors
        for(auto it = _gen_acc.color_mul_accessors.begin(); it != _gen_acc.color_mul_accessors.end(); it++)
            indexed_regions[it->buffer_id].push_back(*it);
        _StrideBuffer(_root, all_regions, indexed_regions, _buffers, &GLTFCompiler::_SupplementIndexedColorMultipliers);
        indexed_regions.clear();
        indexed_regions.resize(_root.buffers.size());

        // push joints accessors
        for(auto it = _gen_acc.joints_accessors.begin(); it != _gen_acc.joints_accessors.end(); it++)
            indexed_regions[it->buffer_id].push_back(*it);
        _StrideBuffer(_root, all_regions, indexed_regions, _buffers, &GLTFCompiler::_SupplementIndexedJointsIndices);
        indexed_regions.clear();
        indexed_regions.resize(_root.buffers.size());

        // push weights accessors
        for(auto it = _gen_acc.weights_accessors.begin(); it != _gen_acc.weights_accessors.end(); it++)
            indexed_regions[it->buffer_id].push_back(*it);
        _StrideBuffer(_root, all_regions, indexed_regions, _buffers, &GLTFCompiler::_SupplementIndexedJointWeights);
        indexed_regions.clear();
        indexed_regions.resize(_root.buffers.size());

    }


    GLTFCompiler::GenericVertexAttribute GLTFCompiler::_GenerateGenericVertexAttribute(GLTFCompiler::GenericVertexAttributeAccessors &_gen_acc, std::vector<DasBuffer> &_buffers, uint32_t _index) {
        GenericVertexAttribute v;

        // position vertex
        size_t offset = _gen_acc.pos_accessor.buffer_offset + sizeof(Vector3<float>) * _index;
        auto bit = _FindDataPtrFromOffset(_buffers[_gen_acc.pos_accessor.buffer_id].data_ptrs, offset);
        v.pos = *reinterpret_cast<const Vector3<float>*>(bit->first + offset);
        
        // vertex normal
        if(_gen_acc.normal_accessor.buffer_id != UINT32_MAX) {
            offset = _gen_acc.normal_accessor.buffer_offset + sizeof(Vector3<float>) * _index;
            bit = _FindDataPtrFromOffset(_buffers[_gen_acc.normal_accessor.buffer_id].data_ptrs, offset);
            v.normal = *reinterpret_cast<const Vector3<float>*>(bit->first + offset);
        }

        // vertex tangent
        if(_gen_acc.tangent_accessor.buffer_id != UINT32_MAX) {
            offset = _gen_acc.tangent_accessor.buffer_offset + sizeof(Vector3<float>) * _index;
            bit = _FindDataPtrFromOffset(_buffers[_gen_acc.tangent_accessor.buffer_id].data_ptrs, offset);
            v.normal = *reinterpret_cast<const Vector3<float>*>(bit->first + offset);
        }

        // uv accessors
        v.uv.reserve(_gen_acc.uv_accessors.size());
        for(auto it = _gen_acc.uv_accessors.begin(); it != _gen_acc.uv_accessors.end(); it++) {
            offset = it->buffer_offset + sizeof(Vector2<float>) * _index;
            bit = _FindDataPtrFromOffset(_buffers[it->buffer_id].data_ptrs, offset);
            v.uv.push_back(*reinterpret_cast<const Vector2<float>*>(bit->first + offset));
        }

        // color accessors
        v.color.reserve(_gen_acc.color_mul_accessors.size());
        for(auto it = _gen_acc.color_mul_accessors.begin(); it != _gen_acc.color_mul_accessors.end(); it++) {
            offset = it->buffer_offset + sizeof(Vector4<float>) * _index;
            bit = _FindDataPtrFromOffset(_buffers[it->buffer_id].data_ptrs, offset);
            v.color.push_back(*reinterpret_cast<const Vector4<float>*>(bit->first + offset));
        }

        // joints accessors
        v.joints.reserve(_gen_acc.joints_accessors.size());
        for(auto it = _gen_acc.joints_accessors.begin(); it != _gen_acc.joints_accessors.end(); it++) {
            offset = it->buffer_offset + sizeof(Vector4<uint16_t>) * _index;
            bit = _FindDataPtrFromOffset(_buffers[it->buffer_id].data_ptrs, offset);
            v.joints.push_back(*reinterpret_cast<const Vector4<uint16_t>*>(bit->first + offset));
        }

        // weights accessors
        v.weights.reserve(_gen_acc.weights_accessors.size());
        for(auto it = _gen_acc.weights_accessors.begin(); it != _gen_acc.weights_accessors.end(); it++) {
            offset = it->buffer_offset + sizeof(Vector4<float>) * _index;
            bit = _FindDataPtrFromOffset(_buffers[it->buffer_id].data_ptrs, offset);
            v.weights.push_back(*reinterpret_cast<const Vector4<float>*>(bit->first + offset));
        }

        return v;
    }


    void GLTFCompiler::_IndexMeshPrimitive(GLTFRoot &_root, GLTFMeshPrimitive &_prim, std::vector<DasBuffer> &_buffers, GLTFAccessors &_accessors, GenericVertexAttributeAccessors &_gen_acc) {
        std::array<uint32_t, 6> offsets;
        std::fill(offsets.begin(), offsets.end(), UINT32_MAX);

        std::unordered_map<GenericVertexAttribute, uint32_t, Hash<GenericVertexAttribute>> m;
        m.reserve(static_cast<size_t>(_gen_acc.pos_accessor.used_size / _gen_acc.pos_accessor.unit_size));

        uint32_t max = 0;
        m_indexed_attributes.clear();
        m_supplemented_indices.clear();
        m_indexed_attributes.reserve(static_cast<size_t>(_gen_acc.pos_accessor.used_size / _gen_acc.pos_accessor.unit_size));
        m_supplemented_indices.reserve(static_cast<size_t>(_gen_acc.pos_accessor.used_size / _gen_acc.pos_accessor.unit_size));

        for(uint32_t i = 0; i < _gen_acc.pos_accessor.used_size / _gen_acc.pos_accessor.unit_size; i++) {
            GenericVertexAttribute v = _GenerateGenericVertexAttribute(_gen_acc, _buffers, i);

            // 1. vertex was not found to be in hashmap thus push it there
            // 2. vertex was found add index to supplemented_indices array
            if(m.find(v) == m.end()) {
                m[v] = max;
                m_indexed_attributes.push_back(std::move(v));
                m_supplemented_indices.push_back(max++);
            } else {
                m_supplemented_indices.push_back(m[v]);
            }
        }

        _WriteIndexedData(_root, _buffers, _gen_acc);
        _CreateNewIndexRegion(_root, _accessors[0], _prim, _buffers[0]);
    }


    void GLTFCompiler::_IndexGeometry(GLTFRoot &_root, GLTFAccessors &_accessors, std::vector<DasBuffer> &_buffers) {
        // for each mesh
        for(auto mesh_it = _root.meshes.begin(); mesh_it != _root.meshes.end(); mesh_it++) {
            // for each primitive
            for(auto prim_it = mesh_it->primitives.begin(); prim_it != mesh_it->primitives.end(); prim_it++) {
                if(prim_it->indices == INT32_MAX) {
                    // TODO: Calculate GenericVertexAttributeAccessors object to pass into indexing method
                    GenericVertexAttributeAccessors gen_acc;
                    gen_acc.uv_accessors.resize(_EnumerateMultiSpecVertexAttributes(prim_it->attributes, "TEXCOORD_"));
                    gen_acc.color_mul_accessors.resize(_EnumerateMultiSpecVertexAttributes(prim_it->attributes, "COLOR_"));
                    gen_acc.joints_accessors.resize(_EnumerateMultiSpecVertexAttributes(prim_it->attributes, "JOINTS_"));
                    gen_acc.weights_accessors.resize(_EnumerateMultiSpecVertexAttributes(prim_it->attributes, "WEIGHTS_"));

                    for(auto attr_it = prim_it->attributes.begin(); attr_it != prim_it->attributes.end(); attr_it++) {
                        // check if the attribute exists in attribute type map   
                        const std::string no_nr = Algorithm::RemoveNumbers(attr_it->first);
                        if(m_attribute_type_map.find(no_nr) == m_attribute_type_map.end()) {
                            std::cerr << "Invalid attribute with name '" << attr_it->first << "'" << std::endl;
                            std::exit(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                        }

                        switch(m_attribute_type_map.find(no_nr)->second) {
                            case LIBDAS_BUFFER_TYPE_VERTEX:
                                gen_acc.pos_accessor = _FindAccessorData(_root, attr_it->second);
                                break;

                            case LIBDAS_BUFFER_TYPE_VERTEX_NORMAL:
                                gen_acc.normal_accessor = _FindAccessorData(_root, attr_it->second);
                                break;

                            case LIBDAS_BUFFER_TYPE_VERTEX_TANGENT:
                                gen_acc.tangent_accessor = _FindAccessorData(_root, attr_it->second);
                                break;

                            case LIBDAS_BUFFER_TYPE_TEXTURE_MAP:
                                {
                                    uint32_t index = _FindMultiSpecVertexAttributeIndex(attr_it->first);
                                    gen_acc.uv_accessors[index] = _FindAccessorData(_root, attr_it->second);
                                }
                                break;

                            case LIBDAS_BUFFER_TYPE_COLOR:
                                {
                                    uint32_t index = _FindMultiSpecVertexAttributeIndex(attr_it->first);
                                    gen_acc.color_mul_accessors[index] = _FindAccessorData(_root, attr_it->second);
                                }
                                break;

                            case LIBDAS_BUFFER_TYPE_JOINTS:
                                {
                                    uint32_t index = _FindMultiSpecVertexAttributeIndex(attr_it->first);
                                    gen_acc.joints_accessors[index] = _FindAccessorData(_root, attr_it->second);
                                }
                                break;

                            case LIBDAS_BUFFER_TYPE_WEIGHTS:
                                {
                                    uint32_t index = _FindMultiSpecVertexAttributeIndex(attr_it->first);
                                    gen_acc.weights_accessors[index] = _FindAccessorData(_root, attr_it->second);
                                }
                                break;

                            default:
                                break;
                        }
                    }

                    // index the accessor region
                    _IndexMeshPrimitive(_root, *prim_it, _buffers, _accessors, gen_acc);
                }
            }
        }
    }


    void GLTFCompiler::_CreateNewIndexRegion(GLTFRoot &_root, std::vector<GLTFAccessor*> &_accessors, GLTFMeshPrimitive &_prim, DasBuffer &_buffer) {
        _root.buffer_views.emplace_back();
        _root.buffer_views.back().byte_offset = _buffer.data_len;
        _root.buffer_views.back().byte_length = static_cast<uint32_t>(m_supplemented_indices.size() * sizeof(uint32_t));

        _root.accessors.emplace_back();
        _root.accessors.back().buffer_view = static_cast<int32_t>(_root.buffer_views.size() - 1);
        _root.accessors.back().byte_offset = 0;
        _root.accessors.back().component_type = KHRONOS_UNSIGNED_INT;
        _root.accessors.back().count = static_cast<int32_t>(m_supplemented_indices.size());
        _root.accessors.back().type = "SCALAR";

        _accessors.push_back(&_root.accessors.back());

        _buffer.data_ptrs.push_back(std::make_pair(reinterpret_cast<const char*>(m_supplemented_indices.data()), m_supplemented_indices.size() * sizeof(uint32_t)));
        _buffer.data_len += static_cast<uint32_t>(m_supplemented_indices.size() * sizeof(uint32_t));

        _prim.indices = static_cast<int32_t>(_root.accessors.size() - 1);
    }

    
    void GLTFCompiler::_FreeSupplementedBuffers(std::vector<std::vector<char*>> _mem_areas) {
        for(auto buf_it = _mem_areas.begin(); buf_it != _mem_areas.end(); buf_it++) {
            for(auto mem_it = buf_it->begin(); mem_it != buf_it->end(); mem_it++) {
                delete [] *mem_it;
            }
        }

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
                if(it->primitives[i].indices != INT32_MAX) {
                    int32_t index_buffer_id = _root.buffer_views[_root.accessors[it->primitives[i].indices].buffer_view].buffer;
                    _buffers[index_buffer_id].type |= LIBDAS_BUFFER_TYPE_INDICES;
                }

                // check into attributes
                for(auto map_it = it->primitives[i].attributes.begin(); map_it != it->primitives[i].attributes.end(); map_it++) {
                    const std::string no_nr = Algorithm::RemoveNumbers(map_it->first);
                    if(m_attribute_type_map.find(no_nr) == m_attribute_type_map.end()) {
                        std::cerr << "GLTF error:  No valid attribute '" << map_it->first << "' available for current implementation" << std::endl;
                        EXIT_ON_ERROR(1);
                    }

                    const int32_t buffer_id = _root.buffer_views[_root.accessors[map_it->second].buffer_view].buffer;
                    _buffers[buffer_id].type |= m_attribute_type_map.find(no_nr)->second;
                }
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

        return buffers;
    }


    std::vector<DasMorphTarget> GLTFCompiler::_CreateMorphTargets(const GLTFRoot &_root) {
        std::vector<DasMorphTarget> morph_targets;

        for(auto it = _root.meshes.begin(); it != _root.meshes.end(); it++) {
            for(auto primitive_it = it->primitives.begin(); primitive_it != it->primitives.end(); primitive_it++) {
                for(auto target_it = primitive_it->targets.begin(); target_it != primitive_it->targets.end(); target_it++) {

                    DasMorphTarget morph_target;
                    bool is_attr = false;

                    _FindMultiSpecVertexAttributeAccessors(*target_it);   
                    _CopyUniversalMultiSpecVertexAttributes(_root, morph_target);

                    for(size_t i = 0; i < target_it->size(); i++) {
                        const std::string no_nr = Algorithm::RemoveNumbers(target_it->at(i).first);

                        if(m_attribute_type_map.find(no_nr) == m_attribute_type_map.end()) {
                            std::cerr << "GLTF error: No valid morph target attribute '" << no_nr << "' available for current implementation" << std::endl;
                            EXIT_ON_ERROR(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                        }

                        BufferAccessorData accessor_data = _FindAccessorData(_root, target_it->at(i).second);

                        switch(m_attribute_type_map.find(no_nr)->second) {
                            case LIBDAS_BUFFER_TYPE_VERTEX:
                                morph_target.vertex_buffer_id = accessor_data.buffer_id;
                                morph_target.vertex_buffer_offset = accessor_data.buffer_offset;
                                is_attr = true;
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

        // for TEXCOORD_n, COLOR_n, JOINTS_n and WEIGHTS_n
        std::vector<uint32_t> texcoord_accessors;
        std::vector<uint32_t> colormul_accessors;
        std::vector<uint32_t> joints_accessors;
        std::vector<uint32_t> weights_accessors;

        // for each mesh
        for(auto mesh_it = _root.meshes.begin(); mesh_it != _root.meshes.end(); mesh_it++) {
            // for each mesh primitive
            for(auto prim_it = mesh_it->primitives.begin(); prim_it != mesh_it->primitives.end(); prim_it++) {
                DasMeshPrimitive prim;

                // check if the primitive mode is correct
                if(prim_it->mode != KHRONOS_TRIANGLES) {
                    std::cerr << "Non-triangle geometry is not supported" << std::endl;
                    EXIT_ON_ERROR(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                }

                // write index buffer data
                BufferAccessorData accessor_data = _FindAccessorData(_root, prim_it->indices);
                prim.index_buffer_id = accessor_data.buffer_id;
                prim.index_buffer_offset = accessor_data.buffer_offset;
                prim.indices_count = _root.accessors[prim_it->indices].count;
                _FindMultiSpecVertexAttributeAccessors(prim_it->attributes);
                _CopyUniversalMultiSpecVertexAttributes(_root, prim);
    
                prim.joint_set_count = m_joints_accessors.size();
                if(prim.joint_set_count) {
                    prim.joint_index_buffer_ids = new uint32_t[prim.joint_set_count];
                    prim.joint_index_buffer_offsets = new uint32_t[prim.joint_set_count];
                    prim.joint_weight_buffer_ids = new uint32_t[prim.joint_set_count];
                    prim.joint_weight_buffer_offsets = new uint32_t[prim.joint_set_count];

                    // write joint indices data to das mesh primitive
                    for(uint32_t i = 0; i < prim.joint_set_count; i++) {
                        accessor_data = _FindAccessorData(_root, m_joints_accessors[i]);
                        prim.joint_index_buffer_ids[i] = accessor_data.buffer_id;
                        prim.joint_index_buffer_offsets[i] = accessor_data.buffer_offset;
                    }

                    // write joint weights to das mesh primitive
                    for(uint32_t i = 0; i < prim.joint_set_count; i++) {
                        accessor_data = _FindAccessorData(_root, m_weights_accessors[i]);
                        prim.joint_weight_buffer_ids[i] = accessor_data.buffer_id;
                        prim.joint_weight_buffer_offsets[i] = accessor_data.buffer_offset;
                    }
                }

                // for each attribute write its data into mesh primitive structure
                for(auto attr_it = prim_it->attributes.begin(); attr_it != prim_it->attributes.end(); attr_it++) {
                    // no attribute found, display an error
                    const std::string no_nr = Algorithm::RemoveNumbers(attr_it->first);
                    if(m_attribute_type_map.find(no_nr) == m_attribute_type_map.end()) {
                        std::cerr << "Invalid attribute " << attr_it->first << std::endl;
                        EXIT_ON_ERROR(LIBDAS_ERROR_INVALID_DATA_LENGTH);
                    }

                    accessor_data = _FindAccessorData(_root, attr_it->second);
                    switch(m_attribute_type_map.find(no_nr)->second) {
                        case LIBDAS_BUFFER_TYPE_VERTEX:
                            prim.vertex_buffer_id = accessor_data.buffer_id;
                            prim.vertex_buffer_offset = accessor_data.buffer_offset;
                            break;

                        case LIBDAS_BUFFER_TYPE_VERTEX_NORMAL:
                            prim.vertex_normal_buffer_id = accessor_data.buffer_id;
                            prim.vertex_normal_buffer_offset = accessor_data.buffer_offset;
                            break;

                        case LIBDAS_BUFFER_TYPE_VERTEX_TANGENT:
                            prim.vertex_tangent_buffer_id = accessor_data.buffer_id;
                            prim.vertex_tangent_buffer_offset = accessor_data.buffer_offset;
                            break;

                        default:
                            break;
                    }

                    // set morph targets with their correct counts
                    prim.morph_target_count = static_cast<uint32_t>(prim_it->targets.size());
                    if(prim.morph_target_count) {
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
                    EXIT_ON_ERROR(LIBDAS_ERROR_INVALID_DATA_LENGTH);
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
                joint.scale = (x + y + z) / 3;
                joint.translation = _root.nodes[skin_it->joints[i]].translation;

                joints.emplace_back(std::move(joint));
            }
        }

        joints.shrink_to_fit();
        return joints;
    }


    std::vector<DasAnimationChannel> GLTFCompiler::_CreateAnimationChannels(const GLTFRoot &_root, const std::vector<DasBuffer> &_buffers) {
        std::vector<DasAnimationChannel> channels;

        // assuming that there are 2 channels for each animation object
        channels.reserve(_root.animations.size() * 2);

        for(auto ani_it = _root.animations.begin(); ani_it != _root.animations.end(); ani_it++) {
            // for each channel in animation
            for(auto ch_it = ani_it->channels.begin(); ch_it != ani_it->channels.end(); ch_it++) {
                DasAnimationChannel channel;

                if(m_scene_node_id_table[ch_it->target.node] != UINT32_MAX)
                    channel.node_id = m_scene_node_id_table[ch_it->target.node];
                else if(m_skeleton_joint_id_table[ch_it->target.node] != UINT32_MAX)
                    channel.joint_id = m_skeleton_joint_id_table[ch_it->target.node];
                else continue;

                // check path value
                uint32_t type_stride = 0;
                if(ch_it->target.path == "translation") {
                    channel.target = LIBDAS_ANIMATION_TARGET_TRANSLATION;
                    type_stride = static_cast<uint32_t>(sizeof(Libdas::Vector3<float>));
                }
                else if(ch_it->target.path == "rotation") {
                    channel.target = LIBDAS_ANIMATION_TARGET_ROTATION;
                    type_stride = static_cast<uint32_t>(sizeof(Libdas::Quaternion));
                }
                else if(ch_it->target.path == "scale") {
                    channel.target = LIBDAS_ANIMATION_TARGET_SCALE;
                    type_stride = static_cast<uint32_t>(sizeof(float));
                }
                else if(ch_it->target.path == "weights") {
                    channel.target = LIBDAS_ANIMATION_TARGET_WEIGHTS;
                    LIBDAS_ASSERT(ch_it->target.node != INT32_MAX);
                    const GLTFNode &node = _root.nodes[ch_it->target.node];
                    LIBDAS_ASSERT(node.mesh != INT32_MAX);
                    const GLTFMesh &mesh = _root.meshes[node.mesh];
                    LIBDAS_ASSERT(mesh.weights.size());
                    type_stride = static_cast<uint32_t>(sizeof(float) * mesh.weights.size());
                    channel.weight_count = static_cast<uint32_t>(mesh.weights.size());
                } else {
                    LIBDAS_ASSERT(false);
                }

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
                LIBDAS_ASSERT(channel.keyframe_count);

                // allocate and copy keyframe inputs
                channel.keyframes = new float[channel.keyframe_count];
                auto accessor_data = _FindAccessorData(_root, sampler.input);
                size_t offset = static_cast<size_t>(accessor_data.buffer_offset);
                auto ptr = _FindDataPtrFromOffset(_buffers[accessor_data.buffer_id].data_ptrs, offset);
                std::memcpy(channel.keyframes, ptr->first + offset, channel.keyframe_count * sizeof(float));

                // allocate and copy keyframe outputs
                accessor_data = _FindAccessorData(_root, sampler.output);
                channel.target_values = new char[type_stride * channel.keyframe_count];

                if(channel.interpolation == LIBDAS_INTERPOLATION_VALUE_CUBICSPLINE) {
                    // tangent values need to be extracted
                    offset = static_cast<size_t>(accessor_data.buffer_offset);
                    ptr = _FindDataPtrFromOffset(_buffers[accessor_data.buffer_id].data_ptrs, offset);

                    channel.tangents = new char[2 * type_stride * channel.keyframe_count];
                    for(size_t i = 0; i < channel.keyframe_count; i++) {
                        std::memcpy(channel.tangents + 2 * i * type_stride, ptr->first + 3 * i * type_stride, type_stride);
                        std::memcpy(channel.target_values + type_stride * i, ptr->first + (3 * i + 1) * type_stride, type_stride);
                        std::memcpy(channel.tangents + (2 * i + 1) * type_stride, ptr->first + (3 * i + 2) * type_stride, type_stride);
                    }
                } else {
                    offset = static_cast<size_t>(accessor_data.buffer_offset);
                    ptr = _FindDataPtrFromOffset(_buffers[accessor_data.buffer_id].data_ptrs, offset);
                    std::memcpy(channel.target_values, ptr->first + offset, channel.keyframe_count * type_stride);
                }

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
        std::vector<DasBuffer> buffers(_CreateBuffers(_root, _embedded_textures));
        _FlagJointNodes(_root);
        std::vector<DasAnimationChannel> channels(_CreateAnimationChannels(_root, buffers));
        _StrideBuffers(_root, buffers);
        _OmitEmptyBuffers(_root, buffers);
        for(auto it = buffers.begin(); it != buffers.end(); it++)
            WriteBuffer(*it);

        // write mesh primitives to the file
        std::vector<DasMeshPrimitive> primitives(_CreateMeshPrimitives(_root));
        for(auto it = primitives.begin(); it != primitives.end(); it++)
            WriteMeshPrimitive(*it);

        // write morph targets to the file
        std::vector<DasMorphTarget> morph_targets(_CreateMorphTargets(_root));
        for(auto it = morph_targets.begin(); it != morph_targets.end(); it++)
            WriteMorphTarget(*it);

        // write meshes to the file
        std::vector<DasMesh> meshes(_CreateMeshes(_root));
        for(auto it = meshes.begin(); it != meshes.end(); it++)
            WriteMesh(*it);

        // write scene nodes to the file
        std::vector<DasNode> nodes(_CreateNodes(_root)); 
        for(auto it = nodes.begin(); it != nodes.end(); it++)
            WriteNode(*it);

        // write scenes to the file
        std::vector<DasScene> scenes(_CreateScenes(_root));
        for(auto it = scenes.begin(); it != scenes.end(); it++)
            WriteScene(*it);

        // write skeleton joints to the file
        std::vector<DasSkeletonJoint> joints(_CreateSkeletonJoints(_root, buffers));
        for(auto it = joints.begin(); it != joints.end(); it++)
            WriteSkeletonJoint(*it);

        // write skeletons to the file
        std::vector<DasSkeleton> skeletons(_CreateSkeletons(_root));
        for(auto it = skeletons.begin(); it != skeletons.end(); it++)
            WriteSkeleton(*it);

        // write animation channels to file
        for(auto it = channels.begin(); it != channels.end(); it++)
            WriteAnimationChannel(*it);

        // write animations to file
        std::vector<DasAnimation> animations(_CreateAnimations(_root));
        for(auto it = animations.begin(); it != animations.end(); it++)
            WriteAnimation(*it);
    }
}
