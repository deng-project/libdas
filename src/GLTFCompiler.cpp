// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: GLTFCompiler.cpp - GLTF format to DAS compiler implementation
// author: Karl-Mihkel Ott

#define GLTF_COMPILER_CPP
#include "das/GLTFCompiler.h"


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
        for(auto it = m_allocated_memory.begin(); it != m_allocated_memory.end(); it++)
            delete [] *it;
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

        accessor_data.buffer_offset = static_cast<uint32_t>(_root.buffer_views[_root.accessors[_accessor_id].buffer_view].byte_offset +
                                                            _root.accessors[_accessor_id].byte_offset);

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
      
        if(_root.buffer_views[_root.accessors[_accessor_id].buffer_view].byte_stride)
            accessor_data.unit_stride = _root.buffer_views[_root.accessors[_accessor_id].buffer_view].byte_stride;
        else accessor_data.unit_stride = accessor_data.unit_size;

        return accessor_data;
    }


    size_t GLTFCompiler::_FindPrimitiveCount(const GLTFRoot &_root) {
        size_t count = 0;
        for(size_t i = 0; i < _root.meshes.size(); i++)
            count += _root.meshes[i].primitives.size();

        return count;
    }


    size_t GLTFCompiler::_FindMorphTargetCount(const GLTFRoot &_root) {
        size_t count = 0;
        for(auto mesh_it = _root.meshes.begin(); mesh_it != _root.meshes.end(); mesh_it++) {
            for(auto prim_it = mesh_it->primitives.begin(); prim_it != mesh_it->primitives.end(); prim_it++) {
                count += prim_it->targets.size();
            }
        }
        return count;
    }


    void GLTFCompiler::_FlagJointNodes(const GLTFRoot &_root) {
        std::vector<bool> skin_joint_table(_root.nodes.size());
        std::fill(skin_joint_table.begin(), skin_joint_table.end(), false);

        m_scene_node_id_table.resize(_root.nodes.size());
        m_skeleton_joint_id_table.resize(_root.nodes.size());

        for(auto skin_it = _root.skins.begin(); skin_it != _root.skins.end(); skin_it++) {
            if(skin_it->skeleton != INT32_MAX)
                skin_joint_table[skin_it->skeleton] = true;

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


    // TODO: add accessor buffer offset correction to the implementation
    // right now only supplementation is done
    DasBuffer GLTFCompiler::_RewriteMeshBuffer(GLTFRoot &_root) {
        DasBuffer buffer;
        m_meshes.reserve(_root.meshes.size());
        m_mesh_primitives.reserve(_FindPrimitiveCount(_root));
        m_morph_targets.reserve(_FindMorphTargetCount(_root));

        // for each mesh
        for(auto mesh_it = _root.meshes.begin(); mesh_it != _root.meshes.end(); mesh_it++) {
            const size_t mesh_id = mesh_it - _root.meshes.begin();
            m_meshes.emplace_back();
            m_meshes[mesh_id].primitives = new uint32_t[mesh_it->primitives.size()];
            m_meshes[mesh_id].primitive_count = static_cast<uint32_t>(mesh_it->primitives.size());

            // for each mesh primitive
            for(auto prim_it = mesh_it->primitives.begin(); prim_it != mesh_it->primitives.end(); prim_it++) {
                auto gen_acc = _GenerateGenericVertexAttributeAccessors(prim_it->attributes);

                m_mesh_primitives.emplace_back();
                _WritePrimitiveData(_root, gen_acc, buffer, m_mesh_primitives.back());

                // check if morph targets were used
                if(prim_it->targets.size()) {
                    m_mesh_primitives.back().morph_target_count = static_cast<uint32_t>(prim_it->targets.size());
                    m_mesh_primitives.back().morph_targets = new uint32_t[prim_it->targets.size()];
                    m_mesh_primitives.back().morph_weights = new float[mesh_it->weights.size()];

                    for(auto morph_it = prim_it->targets.begin(); morph_it != prim_it->targets.end(); morph_it++) {
                        gen_acc = _GenerateGenericVertexAttributeAccessors(*morph_it);
                        m_morph_targets.emplace_back();
                        _WritePrimitiveData(_root, gen_acc, buffer, m_morph_targets.back());

                        // write morph target indicies and weights
                        const size_t id = morph_it - prim_it->targets.begin();
                        m_mesh_primitives.back().morph_targets[id] = static_cast<uint32_t>(m_morph_targets.size() - 1);
                    }

                    for(size_t i = 0; i < mesh_it->weights.size(); i++) {
                        m_mesh_primitives.back().morph_weights[i] = mesh_it->weights[i];
                    }

                }

                const size_t id = prim_it - mesh_it->primitives.begin();
                m_meshes[mesh_id].primitives[id] = static_cast<uint32_t>(m_mesh_primitives.size() - 1);
            }
        }

        return buffer;
    }


    TRS::Vector2<float> GLTFCompiler::_GetUV(BufferAccessorData &_ad, uint32_t index) {
        // access correct buffer entry point
        const char *ptr = m_uri_resolvers[_ad.buffer_id].GetBuffer().first + _ad.buffer_offset + index * _ad.unit_stride;
        TRS::Vector2<float> uv;
        switch(_ad.component_type) {
            case KHRONOS_UNSIGNED_BYTE:
                uv.first = (float) (*reinterpret_cast<const unsigned char*>(ptr));
                uv.second = (float) (*reinterpret_cast<const unsigned char*>(ptr + sizeof(unsigned char)));

                uv /= 255.0f;
                break;

            case KHRONOS_UNSIGNED_SHORT:
                uv.first = (float) (*reinterpret_cast<const unsigned short*>(ptr));
                uv.second = (float) (*reinterpret_cast<const unsigned short*>(ptr + sizeof(unsigned short)));

                uv /= 65535.0f;
                break;

            case KHRONOS_FLOAT:
                uv.first = *reinterpret_cast<const float*>(ptr);
                uv.second = *reinterpret_cast<const float*>(ptr + sizeof(float));
                break;

            default:
                std::cerr << "GLTF error: UV coordinate type must be VEC2 with component type of unsigned char, unsigned short or float" << std::endl;
                std::exit(LIBDAS_ERROR_INVALID_TYPE);
                break;
        }

        return uv;
    }



    TRS::Vector4<float> GLTFCompiler::_GetColorMultiplier(BufferAccessorData &_ad, uint32_t _index) {
        const char *ptr = m_uri_resolvers[_ad.buffer_id].GetBuffer().first + _ad.buffer_offset + _index * _ad.unit_stride;
        TRS::Vector4<float> color;
        switch(_ad.component_type) {
            case KHRONOS_UNSIGNED_BYTE:
                color.first = (float)(*reinterpret_cast<const unsigned char*>(ptr));
                color.second = (float)(*reinterpret_cast<const unsigned char*>(ptr + sizeof(unsigned char)));
                color.third = (float)(*reinterpret_cast<const unsigned char*>(ptr + 2 * sizeof(unsigned char)));

                if(_ad.unit_size / static_cast<uint32_t>(sizeof(unsigned char)) == 4)
                    color.fourth = (float)(*reinterpret_cast<const unsigned char*>(ptr + 3 * sizeof(unsigned char)));
                else color.fourth = 1.0f; // opacity 100%

                color /= 255.0f;
                break;

            case KHRONOS_UNSIGNED_SHORT:
                color.first = (float)(*reinterpret_cast<const unsigned short*>(ptr));
                color.second = (float)(*reinterpret_cast<const unsigned short*>(ptr + sizeof(unsigned short)));
                color.third = (float)(*reinterpret_cast<const unsigned short*>(ptr + 2 * sizeof(unsigned short)));

                if(_ad.unit_size / static_cast<uint32_t>(sizeof(unsigned short)) == 4)
                    color.fourth = (float)(*reinterpret_cast<const unsigned short*>(ptr + 3 * sizeof(unsigned short)));
                else color.fourth = 1.0f; // -"-

                color /= 65535.0f;
                break;

            case KHRONOS_FLOAT:
                color.first = (float)(*reinterpret_cast<const float*>(ptr));
                color.second = (float)(*reinterpret_cast<const float*>(ptr + sizeof(float)));
                color.third = (float)(*reinterpret_cast<const float*>(ptr + 2 * sizeof(float)));

                if(_ad.unit_size / static_cast<uint32_t>(sizeof(float)) == 4)
                    color.fourth = (float)(*reinterpret_cast<const float*>(ptr + 3 * sizeof(float)));
                else color.fourth = 1.0f; // -"-
                break;

            default:
                std::cerr << "GLTF error: Color multiplier type must be VEC4 with component type of unsigned char, unsigned short or float" << std::endl;
                std::exit(LIBDAS_ERROR_INVALID_TYPE);
                break;
        }

        return color;
    }


    TRS::Vector4<uint16_t> GLTFCompiler::_GetJointIndices(BufferAccessorData &_ad, uint32_t _index) {
        const char *ptr = m_uri_resolvers[_ad.buffer_id].GetBuffer().first + _ad.buffer_offset + _index * _ad.unit_stride;
        TRS::Vector4<uint16_t> jindex;
        switch(_ad.component_type) {
            case KHRONOS_UNSIGNED_BYTE:
                jindex.first = (uint16_t)(*reinterpret_cast<const unsigned char*>(ptr));
                jindex.second = (uint16_t)(*reinterpret_cast<const unsigned char*>(ptr + sizeof(unsigned char)));
                jindex.third = (uint16_t)(*reinterpret_cast<const unsigned char*>(ptr + 2 * sizeof(unsigned char)));
                jindex.fourth = (uint16_t)(*reinterpret_cast<const unsigned char*>(ptr + 3 * sizeof(unsigned char)));
                break;

            case KHRONOS_UNSIGNED_SHORT:
                jindex.first = (uint16_t)(*reinterpret_cast<const unsigned short*>(ptr));
                jindex.second = (uint16_t)(*reinterpret_cast<const unsigned short*>(ptr + sizeof(unsigned short)));
                jindex.third = (uint16_t)(*reinterpret_cast<const unsigned short*>(ptr + 2 * sizeof(unsigned short)));
                jindex.fourth = (uint16_t)(*reinterpret_cast<const unsigned short*>(ptr + 3 * sizeof(unsigned short)));
                break;

            default:
                std::cerr << "GLTF error: Joint index type must be VEC4 with component type of unsigned char or unsigned short" << std::endl;
                std::exit(LIBDAS_ERROR_INVALID_TYPE);
                break;
        }

        return jindex;
    }


    TRS::Vector4<float> GLTFCompiler::_GetJointWeights(BufferAccessorData &_ad, uint32_t _index) {
        const char *ptr = m_uri_resolvers[_ad.buffer_id].GetBuffer().first + _ad.buffer_offset + _index * _ad.unit_stride;
        TRS::Vector4<float> weights;
        switch(_ad.component_type) {
            case KHRONOS_UNSIGNED_BYTE:
                weights.first = (float)(*reinterpret_cast<const unsigned char*>(ptr));
                weights.second = (float)(*reinterpret_cast<const unsigned char*>(ptr + sizeof(unsigned char)));
                weights.third = (float)(*reinterpret_cast<const unsigned char*>(ptr + 2 * sizeof(unsigned char)));
                weights.fourth = (float)(*reinterpret_cast<const unsigned char*>(ptr + 3 * sizeof(unsigned char)));

                weights /= 255.0f;
                break;

            case KHRONOS_UNSIGNED_SHORT:
                weights.first = (float)(*reinterpret_cast<const unsigned char*>(ptr));
                weights.second = (float)(*reinterpret_cast<const unsigned char*>(ptr + sizeof(unsigned char)));
                weights.third = (float)(*reinterpret_cast<const unsigned char*>(ptr + 2 * sizeof(unsigned char)));
                weights.fourth = (float)(*reinterpret_cast<const unsigned char*>(ptr + 3 * sizeof(unsigned char)));

                weights /= 65535.0f;
                break;

            case KHRONOS_FLOAT:
                weights.first = (float)(*reinterpret_cast<const float*>(ptr));
                weights.second = (float)(*reinterpret_cast<const float*>(ptr + sizeof(float)));
                weights.third = (float)(*reinterpret_cast<const float*>(ptr + 2 * sizeof(float)));
                weights.fourth = (float)(*reinterpret_cast<const float*>(ptr + 3 * sizeof(float)));
                break;

            default:
                std::cerr << "GLTF error: Joint weight type must be VEC4 with component type of unsigned char, unsigned short or float" << std::endl;
                std::exit(LIBDAS_ERROR_INVALID_TYPE);
                break;
        }

        return weights;
    }


    uint32_t GLTFCompiler::_GetIndex(BufferAccessorData &_ad, uint32_t _index) {
        const char *ptr = m_uri_resolvers[_ad.buffer_id].GetBuffer().first + _ad.buffer_offset + _index * _ad.unit_stride;
        uint32_t attr_id = 0;
        switch(_ad.component_type) {
            case KHRONOS_UNSIGNED_BYTE:
                attr_id = (uint32_t)(*reinterpret_cast<const unsigned char*>(ptr));
                break;

            case KHRONOS_UNSIGNED_SHORT:
                attr_id = (uint32_t)(*reinterpret_cast<const unsigned short*>(ptr));
                break;

            case KHRONOS_UNSIGNED_INT:
                break;

            default:
                std::cerr << "GLTF error: Index type must be SCALAR with component type of unsigned char, unsigned short or unsigned int" << std::endl;
                std::exit(LIBDAS_ERROR_INVALID_TYPE);
                break;
        };

        return attr_id;
    }



    GLTFCompiler::GenericVertexAttributeAccessors GLTFCompiler::_GenerateGenericVertexAttributeAccessors(GLTFMeshPrimitive::AttributesType &_attrs) {
        GenericVertexAttributeAccessors attr_accessors;

        // for each attribute
        for(auto attr_it = _attrs.begin(); attr_it != _attrs.end(); attr_it++) {
            const std::string no_nr = Libdas::Algorithm::RemoveNumbers(attr_it->first);
            if(m_attribute_type_map.find(no_nr) == m_attribute_type_map.end()) {
                std::cerr << "GLTF error: invalid attribute vertex attribute '" << attr_it->first << "'" << std::endl;
                std::exit(LIBDAS_ERROR_INVALID_ARGUMENT);
            }

            switch(m_attribute_type_map.find(no_nr)->second) {
                case LIBDAS_BUFFER_TYPE_VERTEX:
                    attr_accessors.pos_accessor = attr_it->second;
                    break;

                case LIBDAS_BUFFER_TYPE_VERTEX_NORMAL:
                    attr_accessors.normal_accessor = attr_it->second;
                    break;

                case LIBDAS_BUFFER_TYPE_VERTEX_TANGENT:
                    attr_accessors.tangent_accessor = attr_it->second;
                    break;

                case LIBDAS_BUFFER_TYPE_TEXTURE_MAP: 
                    {
                        const uint32_t index = std::stoi(attr_it->first.substr(no_nr.size()));
                        if(index + 1 > static_cast<uint32_t>(attr_accessors.uv_accessors.size()))
                            attr_accessors.uv_accessors.resize(index + 1);
                        attr_accessors.uv_accessors[index] = attr_it->second;
                    }
                    break;

                case LIBDAS_BUFFER_TYPE_COLOR:
                    {
                        const uint32_t index = std::stoi(attr_it->first.substr(no_nr.size()));
                        if(index + 1 > static_cast<uint32_t>(attr_accessors.color_mul_accessors.size()))
                            attr_accessors.color_mul_accessors.resize(index + 1);
                        attr_accessors.color_mul_accessors[index] = attr_it->second;
                    }
                    break;

                case LIBDAS_BUFFER_TYPE_JOINTS:
                    {
                        const uint32_t index = std::stoi(attr_it->first.substr(no_nr.size()));
                        if(index + 1 > static_cast<uint32_t>(attr_accessors.joints_accessors.size()))
                            attr_accessors.joints_accessors.resize(index + 1);
                        attr_accessors.joints_accessors[index] = attr_it->second;
                    }
                    break;

                case LIBDAS_BUFFER_TYPE_WEIGHTS:
                    {
                        const uint32_t index = std::stoi(attr_it->first.substr(no_nr.size()));
                        if(index + 1 > static_cast<uint32_t>(attr_accessors.weights_accessors.size()))
                            attr_accessors.weights_accessors.resize(index + 1);
                        attr_accessors.weights_accessors[index] = attr_it->second;
                    }
                    break;

                default:
                    LIBDAS_ASSERT(false);
                    break;
            }
        }

        return attr_accessors;
    }


    void GLTFCompiler::_CopyVertexAttributeAccessorDataToBuffer(GLTFRoot &_root, uint32_t _accessor, DasBuffer &_buffer, uint32_t &_attr_id, uint32_t &_attr_offset) {
        BufferAccessorData acc = _FindAccessorData(_root, _accessor);

        // unit stride was specified
        if(acc.unit_stride > acc.unit_size) {
            const size_t mod = (acc.used_size * acc.unit_size / acc.unit_stride) % sizeof(float);
            size_t len = acc.used_size * acc.unit_size / acc.unit_stride + sizeof(float) - mod;
            char *buf = new char[len]{};
            
            char *tmpdata = new char[acc.unit_size]{};
            for(size_t i = 0; i < acc.used_size / acc.unit_stride; i++) {
                std::memcpy(tmpdata, m_uri_resolvers[acc.buffer_id].GetBuffer().first + acc.buffer_offset + i * acc.unit_stride, acc.unit_size);
                std::memcpy(buf + i * acc.unit_size, tmpdata, acc.unit_size);
            }

            delete [] tmpdata;
            m_allocated_memory.push_back(buf);
            _buffer.data_ptrs.push_back(std::make_pair(buf, len));
            _attr_id = 0;
            _attr_offset = _buffer.data_len;
            _buffer.data_len += (uint32_t)len;
        } else {
            size_t len = acc.used_size;
            char *buf = new char[len]{};
            std::memcpy(buf, m_uri_resolvers[acc.buffer_id].GetBuffer().first + acc.buffer_offset, acc.used_size);
            m_allocated_memory.push_back(buf);
            _buffer.data_ptrs.push_back(std::make_pair(buf, len));
            _attr_id = 0;
            _attr_offset = _buffer.data_len;
            _buffer.data_len += (uint32_t)len;
        }

    }


    void GLTFCompiler::_CheckAndSupplementProperties(const GLTFRoot &_root, DasProperties &_props) {
        if(_props.author == "")
            _props.author = _root.asset.generator;
        if(_props.copyright == "")
            _props.copyright = _root.asset.copyright;

        // check if default scene exists
        if(_root.load_time_scene != INT32_MAX)
            _props.default_scene = static_cast<uint32_t>(_root.load_time_scene);
        else _props.default_scene = 0; // default scene generation not implemented
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

                    _buffers[0].type |= m_attribute_type_map.find(no_nr)->second;
                }
            }
        }
    }


    std::vector<DasBuffer> GLTFCompiler::_CreateBuffers(GLTFRoot &_root, const std::vector<std::string> &_embedded_textures) {
        std::vector<DasBuffer> buffers;

        // append buffers
        for(auto it = _root.buffers.begin(); it != _root.buffers.end(); it++) {
            m_uri_resolvers.push_back(URIResolver(it->uri, m_root_path));
        }

        buffers.push_back(_RewriteMeshBuffer(_root));

        // append images
        for(auto it = _root.images.begin(); it != _root.images.end(); it++) {
            // there are two possibilities:
            // 1. the image is defined with its uri
            // 2. the image is defined in some buffer view
            if(it->uri != "") {
                m_uri_resolvers.emplace_back(it->uri, m_root_path);
                DasBuffer buffer;
                buffer.type |= m_uri_resolvers.back().GetParsedDataType();
                buffer.data_len = static_cast<uint32_t>(m_uri_resolvers.back().GetBuffer().second);
                buffer.data_ptrs.push_back(m_uri_resolvers.back().GetBuffer());

                buffers.push_back(buffer);
            }
        }

        AppendTextures(buffers, _embedded_textures);
        _FlagBuffersAccordingToMeshes(_root, buffers);

        return buffers;
    }


    std::vector<DasNode> GLTFCompiler::_CreateNodes(const GLTFRoot &_root) {
        std::vector<DasNode> nodes;
        const int32_t max = *std::max_element(reinterpret_cast<int32_t*>(m_scene_node_id_table.data()), reinterpret_cast<int32_t*>(m_scene_node_id_table.data() + m_scene_node_id_table.size()));
        nodes.resize(max + 1);

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

            const TRS::Matrix4<float> def_mat;
            if(_root.nodes[i].matrix != def_mat) {
                node.transform = _root.nodes[i].matrix.Transpose();
            } else {
                const TRS::Matrix4<float> t = {
                    { 1.0f, 0.0f, 0.0f, _root.nodes[i].translation.x },
                    { 0.0f, 1.0f, 0.0f, _root.nodes[i].translation.y },
                    { 0.0f, 0.0f, 1.0f, _root.nodes[i].translation.z },
                    { 0.0f, 0.0f, 0.0f, 1.0f },
                };
                const TRS::Matrix4<float> r = _root.nodes[i].rotation.ExpandToMatrix4();
                const float uni_scale = (_root.nodes[i].scale.x + _root.nodes[i].scale.y + _root.nodes[i].scale.z) / 3;
                const TRS::Matrix4<float> s = {
                    { uni_scale, 0.0f, 0.0f, 0.0f },
                    { 0.0f, uni_scale, 0.0f, 0.0f },
                    { 0.0f, 0.0f, uni_scale, 0.0f },
                    { 0.0f, 0.0f, 0.0f, 1.0f }
                };

                node.transform = t * r * s;
            }

            nodes[m_scene_node_id_table[i]] = std::move(node);
        }

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
                skeleton.parent = static_cast<uint32_t>(m_skeleton_joint_id_table[_root.skins[i].skeleton]);
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


    std::vector<DasSkeletonJoint> GLTFCompiler::_CreateSkeletonJoints(const GLTFRoot &_root) {
        std::vector<DasSkeletonJoint> joints;
        const int32_t max = *std::max_element(reinterpret_cast<int32_t*>(m_skeleton_joint_id_table.data()), reinterpret_cast<int32_t*>(m_skeleton_joint_id_table.data() + m_skeleton_joint_id_table.size()));
        joints.resize(max + 1);
        
        for(auto skin_it = _root.skins.begin(); skin_it != _root.skins.end(); skin_it++) {
            BufferAccessorData accessor_data = _FindAccessorData(_root, skin_it->inverse_bind_matrices);

            const char *buf = m_uri_resolvers[accessor_data.buffer_id].GetBuffer().first + accessor_data.buffer_offset;
            for(size_t i = 0; i < skin_it->joints.size(); i++) {
                if(m_skeleton_joint_id_table[skin_it->joints[i]] == UINT32_MAX) continue;

                DasSkeletonJoint joint;
                joint.inverse_bind_pos = reinterpret_cast<const TRS::Matrix4<float>*>(buf)[i].Transpose();
                joint.rotation = _root.nodes[skin_it->joints[i]].rotation;

                // for each child in joint
                joint.children_count = static_cast<uint32_t>(_root.nodes[skin_it->joints[i]].children.size());
                joint.children = new uint32_t[joint.children_count];
                for(uint32_t j = 0; j < joint.children_count; j++)
                    joint.children[j] = m_skeleton_joint_id_table[_root.nodes[skin_it->joints[i]].children[j]];

                const float x = _root.nodes[skin_it->joints[i]].scale.x , 
                            y = _root.nodes[skin_it->joints[i]].scale.y, 
                            z = _root.nodes[skin_it->joints[i]].scale.z;

                // convert non-uniform scale to uniform scale
                joint.scale = (x + y + z) / 3;
                joint.translation = _root.nodes[skin_it->joints[i]].translation;

                joints[m_skeleton_joint_id_table[skin_it->joints[i]]] = std::move(joint);
            }
        }

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

                if(m_scene_node_id_table[ch_it->target.node] != UINT32_MAX)
                    channel.node_id = m_scene_node_id_table[ch_it->target.node];
                else if(m_skeleton_joint_id_table[ch_it->target.node] != UINT32_MAX)
                    channel.joint_id = m_skeleton_joint_id_table[ch_it->target.node];
                else continue;

                // check path value
                uint32_t type_stride = 0;
                bool scale = false;
                if(ch_it->target.path == "translation") {
                    channel.target = LIBDAS_ANIMATION_TARGET_TRANSLATION;
                    type_stride = static_cast<uint32_t>(sizeof(TRS::Vector3<float>));
                }
                else if(ch_it->target.path == "rotation") {
                    channel.target = LIBDAS_ANIMATION_TARGET_ROTATION;
                    type_stride = static_cast<uint32_t>(sizeof(TRS::Quaternion));
                }
                else if(ch_it->target.path == "scale") {
                    channel.target = LIBDAS_ANIMATION_TARGET_SCALE;
                    type_stride = static_cast<uint32_t>(sizeof(float));
                    scale = true;
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
                const char *buf = m_uri_resolvers[accessor_data.buffer_id].GetBuffer().first + accessor_data.buffer_offset;
                std::memcpy(channel.keyframes, buf, channel.keyframe_count * sizeof(float));

                // allocate and copy keyframe outputs
                accessor_data = _FindAccessorData(_root, sampler.output);
                channel.target_values = new char[type_stride * channel.keyframe_count];

                if(channel.interpolation == LIBDAS_INTERPOLATION_VALUE_CUBICSPLINE) {
                    // tangent values need to be extracted
                    buf = m_uri_resolvers[accessor_data.buffer_id].GetBuffer().first + accessor_data.buffer_offset;
                    channel.tangents = new char[2 * type_stride * channel.keyframe_count];

                    for(size_t i = 0; i < channel.keyframe_count; i++) {
                        std::memcpy(channel.tangents + 2 * i * type_stride, buf + 3 * i * type_stride, type_stride);
                        std::memcpy(channel.target_values + type_stride * i, buf + (3 * i + 1) * type_stride, type_stride);
                        std::memcpy(channel.tangents + (2 * i + 1) * type_stride, buf + (3 * i + 2) * type_stride, type_stride);
                    }
                } else if(scale) {  // convert vec3 scaling into uniform scale
                    buf = m_uri_resolvers[accessor_data.buffer_id].GetBuffer().first + accessor_data.buffer_offset;

                    for(uint32_t i = 0; i < channel.keyframe_count; i++) {
                        const TRS::Vector3<float> &s = reinterpret_cast<const TRS::Vector3<float>*>(buf)[i];
                        float fscale = (s.first + s.second + s.third) / 3;
                        reinterpret_cast<float*>(channel.target_values)[i] = fscale;
                    }
                } else {
                    buf = m_uri_resolvers[accessor_data.buffer_id].GetBuffer().first + accessor_data.buffer_offset;
                    std::memcpy(channel.target_values, buf, channel.keyframe_count * type_stride);
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
            ani.channel_count = static_cast<uint32_t>(ani_it->channels.size());
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
        _FlagJointNodes(_root);

        // write buffers to file
        std::vector<DasBuffer> buffers(_CreateBuffers(_root, _embedded_textures));
        for(auto it = buffers.begin(); it != buffers.end(); it++)
            WriteBuffer(*it);

        // write mesh primitives to the file
        for(auto it = m_mesh_primitives.begin(); it != m_mesh_primitives.end(); it++)
            WriteMeshPrimitive(*it);

        // write morph targets to the file
        for(auto it = m_morph_targets.begin(); it != m_morph_targets.end(); it++)
            WriteMorphTarget(*it);

        // write meshes to the file
        for(auto it = m_meshes.begin(); it != m_meshes.end(); it++)
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
        std::vector<DasSkeletonJoint> joints(_CreateSkeletonJoints(_root));
        for(auto it = joints.begin(); it != joints.end(); it++)
            WriteSkeletonJoint(*it);

        // write skeletons to the file
        std::vector<DasSkeleton> skeletons(_CreateSkeletons(_root));
        for(auto it = skeletons.begin(); it != skeletons.end(); it++)
            WriteSkeleton(*it);

        // write animation channels to file
        std::vector<DasAnimationChannel> channels(_CreateAnimationChannels(_root));
        for(auto it = channels.begin(); it != channels.end(); it++)
            WriteAnimationChannel(*it);

        // write animations to file
        std::vector<DasAnimation> animations(_CreateAnimations(_root));
        for(auto it = animations.begin(); it != animations.end(); it++)
            WriteAnimation(*it);
    }
}
