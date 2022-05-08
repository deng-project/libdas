// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: STLCompiler.cpp - STL file to DAS compiler class implementation
// author: Karl-Mihkel Ott

#define STL_COMPILER_CPP
#include <STLCompiler.h>

namespace Libdas {

    STLCompiler::STLCompiler(const std::string &_out_file) : DasWriterCore(_out_file) {}


    STLCompiler::STLCompiler(std::vector<STLObject> &_objects, DasProperties &_props, std::string &_out_file) {
        Compile(_objects, _props, _out_file);
    }


    void STLCompiler::_IndexVertices(const std::vector<STLObject> &_objects) {
        std::unordered_map<Vertex, uint32_t, Hash<Vertex>> vert_map;
        uint32_t max_index = 0;

        for(const STLObject &obj : _objects) {
            for(size_t i = 0; i < obj.facets.size(); i++) {
                // for each position vertex construct 
                for(const Point3D<float> &pos : obj.facets[i].vertices) {
                    Vertex v = { pos, obj.facets[i].normal };

                    // check if given vertex already exists in map
                    if(vert_map.find(v) != vert_map.end())
                        m_indices.push_back(vert_map[v]);
                    else {
                        vert_map[v] = max_index;
                        m_unique_positions.push_back(v.pos);
                        m_unique_normals.push_back(v.norm);
                        m_indices.push_back(max_index++);
                    }
                }
            }
        }
    }


    DasBuffer STLCompiler::_CreateBuffers() {
        // - position vertices memory area
        // - normal vertices memory area
        // - indices memory area
        DasBuffer buffer;

        buffer.type = LIBDAS_BUFFER_TYPE_VERTEX | LIBDAS_BUFFER_TYPE_VERTEX_NORMAL | LIBDAS_BUFFER_TYPE_INDICES;
        buffer.data_len = static_cast<uint32_t>(sizeof(Point3D<float>) * (m_unique_positions.size() + m_unique_normals.size()) + m_indices.size() * sizeof(uint32_t));

        // push data pointers
        buffer.data_ptrs.push_back(std::make_pair(reinterpret_cast<const char*>(m_unique_positions.data()), m_unique_positions.size() * sizeof(Point3D<float>)));
        buffer.data_ptrs.push_back(std::make_pair(reinterpret_cast<const char*>(m_unique_normals.data()), m_unique_normals.size() * sizeof(Point3D<float>)));
        buffer.data_ptrs.push_back(std::make_pair(reinterpret_cast<const char*>(m_indices.data()), m_indices.size() * sizeof(uint32_t)));

        return buffer;
    }


    std::vector<DasMeshPrimitive> STLCompiler::_CreateMeshPrimitives(const std::vector<STLObject> &_objects) {
        std::vector<DasMeshPrimitive> primitives;
        primitives.reserve(_objects.size());

        for(size_t i = 0; i < _objects.size(); i++) {
            const uint32_t pos_size = static_cast<uint32_t>(m_unique_positions.size() * sizeof(Point3D<float>));
            const uint32_t norm_size = static_cast<uint32_t>(m_unique_normals.size() * sizeof(Point3D<float>));

            DasMeshPrimitive prim;
            // indices
            prim.index_buffer_id = 0;
            prim.index_buffer_offset = pos_size + norm_size;
            prim.indices_count = static_cast<uint32_t>(m_indices.size());

            // vertices
            prim.vertex_buffer_id = 0;
            prim.vertex_buffer_offset = 0;

            // vertex normals
            prim.vertex_normal_buffer_id = 0;
            prim.vertex_normal_buffer_offset = pos_size;

            primitives.emplace_back(prim);
        }

        return primitives;
    }


    DasMesh STLCompiler::_CreateMesh(uint32_t _primitive_count) {
        DasMesh mesh;
        mesh.primitive_count = _primitive_count;
        mesh.primitives = new uint32_t[_primitive_count];

        for(uint32_t i = 0; i < _primitive_count; i++)
            mesh.primitives[i] = i;
        return mesh;
    }


    void STLCompiler::_CreateDefaultScene() {
        // create a scene node
        DasNode node;
        node.mesh = 0;

        WriteNode(node);

        // create a scene
        DasScene scene;
        scene.name = "Imported from STL";
        scene.node_count = 1;
        scene.nodes = new uint32_t;
        scene.nodes[0] = 0;
        
        WriteScene(scene);
    }


    void STLCompiler::Compile(const std::vector<STLObject> &_objects, DasProperties &_props, const std::string &_out_file) {
        // check if new file should be opened
        if(_out_file != "")
            NewFile(_out_file);

        InitialiseFile(_props);
        _IndexVertices(_objects);

        // write all buffers to the file
        DasBuffer buf(_CreateBuffers());
        WriteBuffer(buf);

        std::vector<DasMeshPrimitive> mesh_primitives(_CreateMeshPrimitives(_objects));
        for(DasMeshPrimitive &prim : mesh_primitives)
            WriteMeshPrimitive(prim);

        // write mesh object to the file
        WriteMesh(_CreateMesh(static_cast<uint32_t>(mesh_primitives.size())));

        // generate a default scene if possible
        _CreateDefaultScene();
    }
}
