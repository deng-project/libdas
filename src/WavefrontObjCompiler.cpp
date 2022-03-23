/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: WavefrontObjCompiler.cpp - Wavefront OBJ to DAS compiler class implementation
/// author: Karl-Mihkel Ott

#define WAVEFRONT_OBJ_COMPILER_CPP
#include <WavefrontObjCompiler.h>


namespace Libdas {

    WavefrontObjCompiler::WavefrontObjCompiler(const std::string &_out_file) : 
        DasWriterCore(_out_file) {}
    

    WavefrontObjCompiler::WavefrontObjCompiler(const std::vector<WavefrontObjGroup> &_groups, const DasProperties &_props, const std::string &_out_file,
                                               const std::vector<std::string> &_embedded_textures) :
        DasWriterCore(_out_file)
    {
        Compile(_groups, _props, _out_file, _embedded_textures);
    }


    std::vector<DasBuffer> WavefrontObjCompiler::_CreateBuffers(const std::vector<WavefrontObjGroup> &_groups, const std::vector<std::string> &_embedded_textures) {
        // by default initialise following buffer types
        //  * Vertex buffer
        //  * Texture vertices buffer
        //  * Vertex normal buffer
        //  * Vertex indicies buffer
        std::vector<DasBuffer> buffers(4);
        buffers[0].type = LIBDAS_BUFFER_TYPE_VERTEX;
        buffers[1].type = LIBDAS_BUFFER_TYPE_TEXTURE_MAP;
        buffers[2].type = LIBDAS_BUFFER_TYPE_VERTEX_NORMAL;
        buffers[3].type = LIBDAS_BUFFER_TYPE_INDICES;

        AppendTextures(buffers, _embedded_textures);

        for(const WavefrontObjGroup &group : _groups) {
            const char *data = nullptr;
            size_t size = 0;

            // vertices are given, append to the buffer
            if(group.vertices.position.size()) {
                data = reinterpret_cast<const char*>(group.vertices.position.data());
                size = group.vertices.position.size() * sizeof(Point3D<float>);
                buffers[0].data_ptrs.push_back(std::make_pair(data, size));
                buffers[0].data_len += size;
            }

            // texture vertices are given, append to the buffer
            if(group.vertices.texture.size()) {
                data = reinterpret_cast<const char*>(group.vertices.texture.data());
                size = group.vertices.texture.size() * sizeof(Point3D<float>);
                buffers[1].data_ptrs.push_back(std::make_pair(data, size));
                buffers[1].data_len += size;
            }

            // vertex normals are given, append to the buffer
            if(group.vertices.normals.size()) {
                data = reinterpret_cast<const char*>(group.vertices.normals.data());
                size = group.vertices.normals.size() * sizeof(Point3D<float>);
                buffers[2].data_ptrs.push_back(std::make_pair(data, size));
                buffers[2].data_len += size;
            }

            // vertex indicies are given append to the buffer
            if(group.indices.faces.size()) {
                // NOTE: WavefrontObjFace structure is same as DasFace thus no data realignment is necessary
                data = reinterpret_cast<const char*>(group.indices.faces.data());
                size = group.indices.faces.size() * sizeof(WavefrontObjIndex);
                buffers[3].data_ptrs.push_back(std::make_pair(data, size));
                buffers[3].data_len += size;
            }
        }


        // check if any buffers are empty and if they are remove them
        for(size_t i = 0; i < buffers.size(); i++) {
            if(!buffers[i].data_len) {
                buffers.erase(buffers.begin() + i);
                if(i) i--;
            } 
        }

        bool is_tex = false;

        // assign correct buffer ids
        for(uint32_t i = 0; i < static_cast<uint32_t>(buffers.size()); i++) {
            switch(buffers[i].type) {
                case LIBDAS_BUFFER_TYPE_VERTEX:
                    m_vertex_buffer_id = i;
                    break;

                case LIBDAS_BUFFER_TYPE_TEXTURE_MAP:
                    m_texture_map_buffer_id = i;
                    break;

                case LIBDAS_BUFFER_TYPE_VERTEX_NORMAL:
                    m_vertex_normal_buffer_id = i;
                    break;

                case LIBDAS_BUFFER_TYPE_INDICES:
                    m_indices_buffer_id = i;
                    break;

                case LIBDAS_BUFFER_TYPE_TEXTURE_RAW:
                case LIBDAS_BUFFER_TYPE_TEXTURE_PNG:
                case LIBDAS_BUFFER_TYPE_TEXTURE_JPEG:
                case LIBDAS_BUFFER_TYPE_TEXTURE_TGA:
                case LIBDAS_BUFFER_TYPE_TEXTURE_BMP:
                case LIBDAS_BUFFER_TYPE_TEXTURE_PPM:
                    // basically just select the first texture instance to use, if multiple are present
                    // NOTE: In the future textures will be replaced with correct material specifications
                    if(!is_tex) {
                        m_texture_id = i;
                        is_tex = true;
                    }
                    break;


                default:
                    continue;
            }
        }

        return buffers;
    }


    std::vector<DasMeshPrimitive> WavefrontObjCompiler::_CreateMeshPrimitives(const std::vector<WavefrontObjGroup> &_groups) {
        std::vector<DasMeshPrimitive> primitives(_groups.size());

        // offset values
        uint32_t indices_offset = 0;

        for(size_t i = 0; i < _groups.size(); i++) {
            primitives[i].index_buffer_id = m_indices_buffer_id;
            primitives[i].index_buffer_offset = indices_offset;
            primitives[i].indices_count = _groups[i].indices.faces.size();
            primitives[i].vertex_buffer_id = m_vertex_buffer_id;
            if(m_texture_map_buffer_id != UINT32_MAX)
                primitives[i].uv_buffer_id = m_texture_map_buffer_id;
            if(m_vertex_normal_buffer_id != UINT32_MAX)
                primitives[i].vertex_normal_buffer_id = m_vertex_normal_buffer_id;
            if(m_texture_id != UINT32_MAX)
                primitives[i].texture_id = m_texture_id;

            indices_offset += _groups[i].indices.faces.size() * sizeof(uint32_t[m_face_attr_count]);
        }

        return primitives;
    }


    DasMesh WavefrontObjCompiler::_CreateMesh(uint32_t _primitive_size) {
        DasMesh mesh;
        mesh.name = "Mesh";
        mesh.primitive_count = _primitive_size;
        mesh.primitives = new uint32_t[_primitive_size];

        for(uint32_t i = 0; i < _primitive_size; i++)
            mesh.primitives[i] = i;

        return mesh;
    }


    void WavefrontObjCompiler::_CreateRootScene() {
        // generate a node
        DasNode node;
        node.mesh = 0;

        WriteNode(node);

        // generate a scene
        DasScene scene;
        scene.name = "Imported from Wavefront Obj";
        scene.node_count = 1;
        scene.nodes = new uint32_t;
        scene.nodes[0] = 0;

        WriteScene(scene);
    }


    void WavefrontObjCompiler::Compile(const std::vector<WavefrontObjGroup> &_groups, const DasProperties &_props, const std::string &_out_file, 
                                       const std::vector<std::string> &_embedded_textures) {
        // open a new file if specified
        if(_out_file != "")
            NewFile(_out_file);

        InitialiseFile(_props);

        // write all buffers to the output file
        std::vector<DasBuffer> buffers = _CreateBuffers(_groups, _embedded_textures);
        for(const DasBuffer &buffer : buffers)
            WriteBuffer(buffer);

        // write all given models to the output file
        std::vector<DasMeshPrimitive> primitives = _CreateMeshPrimitives(_groups);
        for(DasMeshPrimitive &prim : primitives)
            WriteMeshPrimitive(prim);

        DasMesh mesh = _CreateMesh(static_cast<uint32_t>(primitives.size()));
        WriteMesh(mesh);

        _CreateRootScene();
    }
}
