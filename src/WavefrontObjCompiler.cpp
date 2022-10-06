/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: WavefrontObjCompiler.cpp - Wavefront OBJ to DAS compiler class implementation
/// author: Karl-Mihkel Ott

#define WAVEFRONT_OBJ_COMPILER_CPP
#include "das/WavefrontObjCompiler.h"


namespace Libdas {

    WavefrontObjCompiler::WavefrontObjCompiler(const std::string &_out_file) : 
        DasWriterCore(_out_file) {}
    

    WavefrontObjCompiler::WavefrontObjCompiler(WavefrontObjData &_data, const DasProperties &_props, const std::string &_out_file,
                                               const std::vector<std::string> &_embedded_textures) :
        DasWriterCore(_out_file)
    {
        Compile(_data, _props, _out_file, _embedded_textures);
    }


    std::vector<DasBuffer> WavefrontObjCompiler::_CreateBuffers(const std::vector<std::string> &_embedded_textures) {
        // by default initialise buffer with following data alignment
        //  - Vertex data
        //  - Texture vertices data
        //  - Vertex normal data
        //  - Vertex indices data
        std::vector<DasBuffer> buffers(1);
        buffers.reserve(1 + _embedded_textures.size());

        buffers.back().type = LIBDAS_BUFFER_TYPE_VERTEX | LIBDAS_BUFFER_TYPE_INDICES;
        buffers.back().data_len = static_cast<uint32_t>(m_unique_pos.size() * sizeof(TRS::Point3D<float>));
        buffers.back().data_ptrs.push_back(std::make_pair(reinterpret_cast<char*>(m_unique_pos.data()), m_unique_pos.size() * sizeof(TRS::Point3D<float>)));

        if(m_unique_uv.size()) {
            buffers.back().data_len += static_cast<uint32_t>(m_unique_uv.size() * sizeof(TRS::Point2D<float>));
            buffers.back().data_ptrs.push_back(std::make_pair(reinterpret_cast<char*>(m_unique_uv.data()), m_unique_uv.size() * sizeof(TRS::Point2D<float>)));
            buffers.back().type |= LIBDAS_BUFFER_TYPE_TEXTURE_MAP;
        }

        if(m_unique_normals.size()) {
            buffers.back().data_len += static_cast<uint32_t>(m_unique_normals.size() * sizeof(TRS::Point3D<float>));
            buffers.back().type |= LIBDAS_BUFFER_TYPE_VERTEX_NORMAL;
            buffers.back().data_ptrs.push_back(std::make_pair(reinterpret_cast<char*>(m_unique_normals.data()), m_unique_normals.size() * sizeof(TRS::Point3D<float>)));
        }

        buffers.back().data_len += static_cast<uint32_t>(m_indices.size() * sizeof(uint32_t));
        buffers.back().data_ptrs.push_back(std::make_pair(reinterpret_cast<char*>(m_indices.data()), m_indices.size() * sizeof(uint32_t)));

        // append all textures to buffers vector
        for(auto it = _embedded_textures.begin(); it != _embedded_textures.end(); it++) {
            buffers.emplace_back();
            TextureReader reader(*it, false);
            size_t size;
            char *data = reader.GetBuffer(size);
            buffers.back().type = reader.GetImageBufferType();
            buffers.back().data_len = static_cast<uint32_t>(size);
            buffers.back().data_ptrs.push_back(std::make_pair(data, size));
        }

        return buffers;
    }

    std::vector<DasMeshPrimitive> WavefrontObjCompiler::_CreateMeshPrimitives(const WavefrontObjData &_data) {
        std::vector<DasMeshPrimitive> primitives;
        primitives.reserve(_data.groups.size());

        const uint32_t buffer_id = 0;
        const uint32_t base_vertex_pos_offset = 0;
        const uint32_t base_uv_vertex_offset = static_cast<uint32_t>(m_unique_pos.size() * sizeof(TRS::Point3D<float>));
        const uint32_t base_vertex_normal_offset = base_uv_vertex_offset + static_cast<uint32_t>(m_unique_uv.size() * sizeof(TRS::Point2D<float>));
        const uint32_t base_index_offset = base_vertex_normal_offset + static_cast<uint32_t>(m_unique_normals.size() * sizeof(TRS::Point3D<float>));

        for(size_t i = 0; i < _data.groups.size(); i++) {
            if(_data.groups[i].indices.faces.size()) {
                primitives.emplace_back();
                primitives.back().index_buffer_id = buffer_id;
                primitives.back().index_buffer_offset = base_index_offset + m_indices_offsets_per_group[i];
                primitives.back().indices_count = _data.groups[i].indices.indices_count;
                primitives.back().vertex_buffer_id = buffer_id;
                primitives.back().vertex_buffer_offset = base_vertex_pos_offset;

                if(_data.groups[i].indices.use_uv) {
                    primitives.back().texture_count = 1;
                    primitives.back().uv_buffer_ids = new uint32_t[primitives.back().texture_count];
                    primitives.back().uv_buffer_offsets = new uint32_t[primitives.back().texture_count];
                    primitives.back().uv_buffer_ids[0] = buffer_id;
                    primitives.back().uv_buffer_offsets[0] = base_uv_vertex_offset;
                } 

                if(_data.groups[i].indices.use_normals) {
                    primitives.back().vertex_normal_buffer_id = buffer_id;
                    primitives.back().vertex_normal_buffer_offset = base_vertex_normal_offset;
                }

                primitives.back().index_buffer_id = buffer_id;
                primitives.back().index_buffer_offset = base_index_offset + m_indices_offsets_per_group[i];
            }
        }

        return primitives;
    }


    std::vector<DasMesh> WavefrontObjCompiler::_CreateMeshes(const std::vector<DasMeshPrimitive> &_primitives) {
        std::vector<DasMesh> meshes(4);

        // allocate enough memory for each possible mesh
        for(auto it = meshes.begin(); it != meshes.end(); it++)
            it->primitives = new uint32_t[_primitives.size()];

        for(auto it = _primitives.begin(); it != _primitives.end(); it++) {
            uint32_t type_mask = 0;
            if(it->texture_count)
                type_mask |= 1;
            if(it->vertex_normal_buffer_id != UINT32_MAX)
                type_mask |= 2;

            meshes[type_mask].primitives[meshes[type_mask].primitive_count++] = static_cast<uint32_t>(it - _primitives.begin());
        }

        return meshes;
    }


    void WavefrontObjCompiler::_CreateRootScene(const std::vector<DasMesh> &_meshes) {
        // generate a node
        uint32_t offset = 0;
        for(size_t i = 0; i < _meshes.size(); i++) {
            if(_meshes[i].primitive_count) {
                DasNode node;
                node.mesh = static_cast<uint32_t>(i) - offset;
                WriteNode(node);
            } else {
                offset++;
            }
        }


        // generate a scene
        DasScene scene;
        scene.name = "Imported from Wavefront Obj";
        scene.node_count = 1;

        scene.nodes = new uint32_t[_meshes.size()];
        for(size_t i = 0; i < _meshes.size(); i++) {
            scene.nodes[i] = static_cast<uint32_t>(i);
        }

        WriteScene(scene);
    }


    void WavefrontObjCompiler::_TriangulateFaces(WavefrontObjData &_data) {
        // for each group indices perform triangulation
        for(auto group_it = _data.groups.begin(); group_it != _data.groups.end(); group_it++) {
            std::vector<std::vector<WavefrontObjIndex>> new_faces;
            new_faces.reserve(group_it->indices.faces.size() * 2);

            // for each face check its indices
            for(size_t i = 0; i < group_it->indices.faces.size(); i++) {
                //LIBDAS_ASSERT(group_it->indices.faces[i].size() <= 4);

                // quad triangulation
                if(group_it->indices.faces[i].size() == 4) {
                    new_faces.emplace_back();
                    new_faces.back().resize(3);
                    new_faces.back()[0] = group_it->indices.faces[i][0];
                    new_faces.back()[1] = group_it->indices.faces[i][1];
                    new_faces.back()[2] = group_it->indices.faces[i][2];

                    new_faces.emplace_back();
                    new_faces.back().resize(3);
                    new_faces.back()[0] = group_it->indices.faces[i][2];
                    new_faces.back()[1] = group_it->indices.faces[i][3];
                    new_faces.back()[2] = group_it->indices.faces[i][0];
                } else if(group_it->indices.faces[i].size() > 3){
                    std::deque<WavefrontObjIndex> indices;
                    std::vector<std::vector<WavefrontObjIndex>> ni;

                    for(auto it = group_it->indices.faces[i].begin(); it != group_it->indices.faces[i].end(); it++)
                        indices.push_back(*it);

                    while(indices.size() > 1) {
                        ni.emplace_back();
                        ni.back().resize(3);

                        ni.back()[0] = indices.front();
                        indices.pop_front();
                        indices.push_back(ni.back()[0]);
                        ni.back()[1] = indices.front();
                        indices.pop_front();
                        ni.back()[2] = indices.front();
                    }
                } else {
                    new_faces.push_back(group_it->indices.faces[i]);
                }
            }

            group_it->indices.faces = new_faces;
        }
    }


    void WavefrontObjCompiler::_ReindexFaces(WavefrontObjData &_data) {
        std::unordered_map<Vertex, uint32_t, Hash<Vertex>> index_map;
        uint32_t max_index = 0;

        // for each group iterate its faces
        for(auto group_it = _data.groups.begin(); group_it != _data.groups.end(); group_it++) {
            m_indices_offsets_per_group.push_back(static_cast<uint32_t>(m_indices.size() * sizeof(uint32_t)));
            for(auto face_it = group_it->indices.faces.begin(); face_it != group_it->indices.faces.end(); face_it++) {
                for(size_t i = 0; i < face_it->size(); i++) {
                    Vertex v;
                    v.pos = _data.vertices.position[face_it->at(i).vert];

                    if(face_it->at(i).texture != UINT32_MAX) {
                        v.uv = _data.vertices.texture[face_it->at(i).texture];
                        group_it->indices.use_uv = true;
                    }

                    if(face_it->at(i).normal != UINT32_MAX) {
                        v.normal = _data.vertices.normals[face_it->at(i).normal];
                        group_it->indices.use_normals = true;
                    }

                    // check if generated vertex does not exist in hash map
                    if(index_map.find(v) == index_map.end()) {
                        index_map[v] = max_index;
                        m_unique_pos.push_back(v.pos);
                        m_unique_uv.push_back(v.uv);
                        m_unique_normals.push_back(v.normal);
                        m_indices.push_back(max_index++);
                    } else m_indices.push_back(index_map[v]);
                }
            }

            group_it->indices.indices_count = (static_cast<uint32_t>(m_indices.size() * sizeof(uint32_t)) - m_indices_offsets_per_group.back()) / sizeof(uint32_t);
        }
    }


    void WavefrontObjCompiler::Compile(WavefrontObjData &_data, const DasProperties &_props, const std::string &_out_file, 
                                       const std::vector<std::string> &_embedded_textures) {
        // open a new file if specified
        if(_out_file != "")
            NewFile(_out_file);

        InitialiseFile(_props);

        // some indexing method call here
        _TriangulateFaces(_data);
        _ReindexFaces(_data);
        // end of some indexing method call

        // write all buffers to the output file
        std::vector<DasBuffer> buffers(_CreateBuffers(_embedded_textures));
        for(const DasBuffer &buffer : buffers)
            WriteBuffer(buffer);

        // write all given models to the output file
        std::vector<DasMeshPrimitive> primitives(_CreateMeshPrimitives(_data));
        for(DasMeshPrimitive &prim : primitives)
            WriteMeshPrimitive(prim);

        std::vector<DasMesh> meshes(_CreateMeshes(primitives));
        for(DasMesh &mesh : meshes) {
            if(mesh.primitive_count)
                WriteMesh(mesh);
        }

        _CreateRootScene(meshes);
    }
}
