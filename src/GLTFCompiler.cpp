// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: GLTFCompiler.cpp - GLTF format to DAS compiler implementation
// author: Karl-Mihkel Ott

#define GLTF_COMPILER_CPP
#include <GLTFCompiler.h>


namespace Libdas {

    GLTFCompiler::GLTFCompiler(const std::string &_in_path, const std::string &_out_file, bool _use_raw_textures) : 
        DasWriterCore(_out_file), m_use_raw_textures(_use_raw_textures), m_root_path(_in_path) {}

    GLTFCompiler::GLTFCompiler(const std::string &_in_path, const GLTFRoot &_root, const DasProperties &_props, 
                               const std::string &_out_file, const std::vector<std::string> &_embedded_textures, bool _use_raw_textures) : 
        m_use_raw_textures(_use_raw_textures), m_root_path(_in_path)
    {
        Compile(_root, _props, _embedded_textures, _out_file);
    }


    GLTFCompiler::~GLTFCompiler() {
        for(size_t i = 0; i < m_supplemented_buffers.size(); i++) {
            delete [] m_supplemented_buffers[i];
        }
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
        return accessor_data;
    }


    size_t GLTFCompiler::_SupplementIndices(const char *_odata, IndexSupplementationInfo &_suppl_info, DasBuffer &_buffer) {
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
        _buffer.data_len += diff;
        m_supplemented_buffers.push_back(buf);
        return diff;
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


    std::vector<std::vector<GLTFCompiler::IndexSupplementationInfo>> GLTFCompiler::_GetBufferIndexRegions(const GLTFRoot &_root) {
        std::vector<std::vector<IndexSupplementationInfo>> index_regions(_root.buffers.size());

        // find all index regions
        for(auto it = _root.meshes.begin(); it != _root.meshes.end(); it++) {
            for(size_t i = 0; i < it->primitives.size(); i++) {
                // error if non-indexed geometry is used
                if(it->primitives[i].indices == INT32_MAX) {
                    std::cerr << "Non-indexed geometry is not supported by DAS" << std::endl;
                    EXIT_ON_ERROR(LIBDAS_ERROR_INVALID_DATA);
                }

                if(_root.accessors[it->primitives[i].indices].component_type == KHRONOS_BYTE ||
                   _root.accessors[it->primitives[i].indices].component_type == KHRONOS_UNSIGNED_BYTE ||
                   _root.accessors[it->primitives[i].indices].component_type == KHRONOS_SHORT ||
                   _root.accessors[it->primitives[i].indices].component_type == KHRONOS_UNSIGNED_SHORT) 
                {
                    BufferAccessorData accessor_data = _FindAccessorData(_root, it->primitives[i].indices);
                    index_regions[accessor_data.buffer_id].push_back({ 
                        accessor_data.buffer_offset, 
                        accessor_data.used_size,  
                        _root.accessors[it->primitives[i].indices].component_type
                    });
                }
            }
        }

        return index_regions;
    }


    void GLTFCompiler::_CorrectOffsets(std::vector<GLTFAccessor*> &_accessors, size_t _diff, size_t _offsets) {
        for(GLTFAccessor *accessor : _accessors) {
            if(accessor->accumulated_offset < _offsets)
                continue;

            accessor->byte_offset += static_cast<uint32_t>(_diff);
            accessor->accumulated_offset += static_cast<uint32_t>(_diff);
        }
    }


    // TODO: add accessor buffer offset correction to the implementation
    // right now only supplementation is done
    void GLTFCompiler::_StrideIndexBuffers(const GLTFRoot &_root, std::vector<DasBuffer> &_buffers) {
        std::vector<std::vector<GLTFAccessor*>> all_regions = _GetAllBufferAccessorRegions(const_cast<GLTFRoot&>(_root));
        std::vector<std::vector<IndexSupplementationInfo>> index_regions = _GetBufferIndexRegions(_root);
        
        // for each buffer with index regions, supplement its data
        for(size_t i = 0; i < index_regions.size(); i++) {
            // sort by offset size
            std::sort(index_regions[i].begin(), index_regions[i].end(), IndexSupplementationInfo::less());
            Algorithm::RemoveDuplicates(index_regions[i], IndexSupplementationInfo::IsDuplicate);

            if(!index_regions[i].size())
                continue;
            DEBUG_LOG("Striding buffer nr " << i);

            const char *odata = _buffers[i].data_ptrs.back().first;
            _buffers[i].data_ptrs.clear();

            for(size_t j = 0; j < index_regions[i].size(); j++) {
                char *buf = nullptr;
                size_t len = 0;
                size_t offset = 0;

                // copy the area before first element
                if(j == 0 && index_regions[i][j].buffer_offset > 0) {
                    len = index_regions[i][j].buffer_offset;
                    buf = new char[len];
                    std::memcpy(buf, odata, len);
                    _buffers[i].data_ptrs.push_back(std::make_pair(reinterpret_cast<const char*>(buf), len));
                    m_supplemented_buffers.push_back(buf);

                    buf = nullptr;
                    len = 0;
                }
                // copy the area between two index regions
                else if(index_regions[i][j - 1].buffer_offset + index_regions[i][j - 1].used_size < index_regions[i][j].buffer_offset) {
                    offset = index_regions[i][j - 1].buffer_offset + index_regions[i][j - 1].used_size;
                    len = index_regions[i][j].buffer_offset - offset;
                    buf = new char[len];

                    std::memcpy(buf, odata + offset, len);
                    _buffers[i].data_ptrs.push_back(std::make_pair(reinterpret_cast<const char*>(buf), len));
                    m_supplemented_buffers.push_back(buf);

                    buf = nullptr;
                    len = 0;
                    offset = 0;
                }

                // supplement indices
                size_t diff = _SupplementIndices(odata, index_regions[i][j], _buffers[i]);

                // correct changed offsets
                _CorrectOffsets(all_regions[i], diff, offset);

                // copy the area between last element and the end of the buffer
                if(j == index_regions[i].size() - 1 && index_regions[i][j].buffer_offset + index_regions[i][j].used_size < _root.buffers[i].byte_length) {
                    offset = index_regions[i][j].buffer_offset + index_regions[i][j].used_size;
                    len = _root.buffers[i].byte_length - offset;
                    buf = new char[len];

                    std::memcpy(buf, odata + offset, len);
                    _buffers[i].data_ptrs.push_back(std::make_pair(reinterpret_cast<const char*>(buf), len));
                    m_supplemented_buffers.push_back(buf);
                }
            }
        }
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
                int32_t size = static_cast<int32_t>(_buffers.size());

                if(it->primitives[i].indices != INT32_MAX)
                    _buffers[index_buffer_id].type |= LIBDAS_BUFFER_TYPE_INDICES;

                // check into attributes
                for(auto map_it = it->primitives[i].attributes.begin(); map_it != it->primitives[i].attributes.end(); map_it++) {
                    std::string no_nr = Algorithm::RemoveNumbers(map_it->first);
                    // error check
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


    std::vector<DasBuffer> GLTFCompiler::_CreateBuffers(const GLTFRoot &_root, const std::vector<std::string> &_embedded_textures) {
        std::vector<DasBuffer> buffers;

        // append buffers
        for(auto it = _root.buffers.begin(); it != _root.buffers.end(); it++) {
            m_uri_resolvers.push_back(URIResolver(it->uri, m_root_path));
            m_modified_buffers_table.push_back(UINT32_MAX);
            DasBuffer buffer;
            buffer.type = LIBDAS_BUFFER_TYPE_UNKNOWN;
            buffer.data_len = it->byte_length;
            buffer.data_ptrs.push_back(m_uri_resolvers.back().GetBuffer());

            LIBDAS_ASSERT(buffer.data_len == buffer.data_ptrs.back().second);
            buffers.push_back(buffer);
        }

        //m_image_buffer_views.insert(m_image_buffer_views.begin(), _root.buffer_views.size(), INT32_MAX);
        _StrideIndexBuffers(_root, buffers);

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


    std::vector<DasMesh> GLTFCompiler::_CreateMeshes(const GLTFRoot &_root) {
        std::vector<DasMesh> meshes;
        for(auto it = _root.meshes.begin(); it != _root.meshes.end(); it++) {
            DasMesh mesh;
            GLTFCompiler::BufferAccessorData accessor_data;
            for(size_t i = 0; i < it->primitives.size(); i++) {
                // check if non-indexed geometry is used and throw an error if needed
                accessor_data = _FindAccessorData(_root, it->primitives[i].indices);
                mesh.index_buffer_id = accessor_data.buffer_id;
                mesh.index_buffer_offset = accessor_data.buffer_offset;
                mesh.indices_count = _root.accessors[it->primitives[i].indices].count;

                // check if topology is anything other than triangles
                if(it->primitives[i].mode != KHRONOS_TRIANGLES) {
                    std::cerr << "Non-triangle topology is not supported" << std::endl;
                    EXIT_ON_ERROR(LIBDAS_ERROR_INVALID_DATA);
                }

                // for each atribute in array
                for(auto attr_it = it->primitives[i].attributes.begin(); attr_it != it->primitives[i].attributes.end(); attr_it++) {
                    accessor_data = _FindAccessorData(_root, attr_it->second);
                    std::string no_nr = Algorithm::RemoveNumbers(attr_it->first);

                    // check if no number string key not present
                    if(m_attribute_type_map.find(no_nr) == m_attribute_type_map.end()) {
                        std::cerr << "Invalid mesh attribute " << attr_it->first << std::endl;
                        EXIT_ON_ERROR(LIBDAS_ERROR_INVALID_DATA);
                    }

                    switch(m_attribute_type_map.find(no_nr)->second) {
                        case LIBDAS_BUFFER_TYPE_VERTEX:
                            mesh.vertex_buffer_id = accessor_data.buffer_id;
                            mesh.vertex_buffer_offset = accessor_data.buffer_offset;
                            break;

                        case LIBDAS_BUFFER_TYPE_VERTEX_NORMAL:
                            mesh.vertex_normal_buffer_id = accessor_data.buffer_id;
                            mesh.vertex_normal_buffer_offset = accessor_data.buffer_offset;
                            break;

                        case LIBDAS_BUFFER_TYPE_VERTEX_TANGENT:
                            mesh.vertex_tangent_buffer_id = accessor_data.buffer_id;
                            mesh.vertex_tangent_buffer_offset = accessor_data.buffer_offset;
                            break;

                        case LIBDAS_BUFFER_TYPE_TEXTURE_MAP:
                            mesh.texture_map_buffer_id = accessor_data.buffer_id;
                            mesh.texture_map_buffer_offset = accessor_data.buffer_offset;
                            break;

                        // temporary fix :DDD
                        default:
                            LIBDAS_ASSERT(false);
                            break;
                    }
                }

                meshes.push_back(mesh);
            }
        }
        return meshes;
    }


    std::vector<DasNode> GLTFCompiler::_CreateNodes(const GLTFRoot &_root) {
        std::vector<DasNode> nodes;
        return nodes;
    }


    std::vector<DasScene> GLTFCompiler::_CreateScenes(const GLTFRoot &_root) {
        std::vector<DasScene> scenes;

        return scenes;
    }


    std::vector<DasSkeleton> GLTFCompiler::_CreateSkeletons(const GLTFRoot &_root) {
        std::vector<DasSkeleton> skeletons;

        return skeletons;
    }


    std::vector<DasSkeletonJoint> GLTFCompiler::_CreateSkeletonJoints(const GLTFRoot &_root) {
        std::vector<DasSkeletonJoint> joints;

        return joints;
    }


    std::vector<DasAnimation> GLTFCompiler::_CreateAnimations(const GLTFRoot &_root) {
        std::vector<DasAnimation> animations;
        return animations;
    }


    void GLTFCompiler::Compile(const GLTFRoot &_root, const DasProperties &_props, const std::vector<std::string> &_embedded_textures, const std::string &_out_file) {
        // check if new file should be opened
        if(_out_file != "")
            NewFile(_out_file);

        _CheckAndSupplementProperties(const_cast<GLTFRoot&>(_root), const_cast<DasProperties&>(_props));
        InitialiseFile(_props);

        // write buffers to file
        std::vector<DasBuffer> buffers = _CreateBuffers(_root, _embedded_textures);
        for(auto it = buffers.begin(); it != buffers.end(); it++)
            WriteBuffer(*it);

        // write meshes to the file
        std::vector<DasMesh> meshes = _CreateMeshes(_root);
        for(auto it = meshes.begin(); it != meshes.end(); it++)
            WriteMesh(*it);

        // write scene nodes to the file
        //std::vector<DasNode> nodes = _CreateNodes(_root);
        //for(auto it = nodes.begin(); it != nodes.end(); it++)
            //WriteNode(*it);

        //// write scenes to the file
        //std::vector<DasScene> scenes = _CreateScenes(_root);
        //for(auto it = scenes.begin(); it != scenes.end(); it++)
            //WriteScene(*it);

        //// write skeletons to the file
        //std::vector<DasSkeleton> skeletons = _CreateSkeletons(_root);
        //for(auto it = skeletons.begin(); it != skeletons.end(); it++)
            //WriteSkeleton(*it);

        //// write skeleton joints to the file
        //std::vector<DasSkeletonJoint> joints = _CreateSkeletonJoints(_root);
        //for(auto it = joints.begin(); it != joints.end(); it++)
            //WriteSkeletonJoint(*it);

        //// write animations to file
        //std::vector<DasAnimation> animations = _CreateAnimations(_root);
        //for(auto it = animations.begin(); it != animations.end(); it++)
            //WriteAnimation(*it);
    }
}
