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
        std::unordered_map<Point3D<float>, uint32_t, Hash<Point3D<float>>> pos_map;
        std::unordered_map<Point3D<float>, uint32_t, Hash<Point3D<float>>> norm_map;

        uint32_t max_pos = 0;

        size_t offset = 0;
        for(const STLObject &obj : _objects) {
            for(size_t i = 0; i < obj.facets.size(); i++) {
                std::array<uint32_t, 2> index;

                // check if vertex normal key exists in normals hashmap
                if(norm_map.find(obj.facets[i].normal) != norm_map.end())
                    index[1] = norm_map[obj.facets[i].normal];
                else {
                    index[1] = max_pos;
                    max_pos++;
                    m_unique_normals.push_back(obj.facets[i].normal);
                    norm_map[obj.facets[i].normal] = index[1];
                }

                // check if the position vertices already exist in the position vertex map
                for(const Point3D<float> &pos : obj.facets[i].vertices) {
                    if(pos_map.find(pos) != pos_map.end())
                        index[0] = pos_map[pos];
                    else {
                        index[0] = max_pos;
                        max_pos++;
                        m_unique_verts.push_back(pos);
                        pos_map[pos] = index[0];
                    }
                }

                m_indices.push_back(index);
            }

            m_offsets.push_back(offset);
            offset += obj.facets.size();
        }
    }


    std::array<DasBuffer, 3> STLCompiler::_CreateBuffers() {
        // 0 - vertices buffer
        // 1 - normals buffer
        // 2 - indices buffer
        std::array<DasBuffer, 3> buffers;

        // create position vertices buffer
        buffers[0].type = LIBDAS_BUFFER_TYPE_VERTEX;
        buffers[0].data_len = static_cast<uint32_t>(m_unique_verts.size() * sizeof(Point3D<float>));
        buffers[0].data_ptrs.push_back(std::make_pair(reinterpret_cast<const char*>(m_unique_verts.data()), buffers[0].data_len));

        // create vertex normal buffer
        buffers[1].type = LIBDAS_BUFFER_TYPE_VERTEX_NORMAL;
        buffers[1].data_len = m_unique_normals.size() * sizeof(Point3D<float>);
        buffers[1].data_ptrs.push_back(std::make_pair(reinterpret_cast<const char*>(m_unique_normals.data()), buffers[1].data_len));

        // create indices buffer
        buffers[2].type = LIBDAS_BUFFER_TYPE_INDICES;
        buffers[2].data_len = m_indices.size() * sizeof(uint32_t[2]);
        buffers[2].data_ptrs.push_back(std::make_pair(reinterpret_cast<const char*>(m_indices.data()), buffers[2].data_len));

        return buffers;
    }


    std::vector<DasMeshPrimitive> STLCompiler::_CreateMeshPrimitives(const std::vector<STLObject> &_objects) {
        std::vector<DasMeshPrimitive> primitives;
        primitives.reserve(_objects.size());

        for(size_t i = 0; i < _objects.size(); i++) {
            DasMeshPrimitive prim;
            prim.index_buffer_id = INDICES_ID;
            prim.index_buffer_offset = static_cast<uint32_t>(m_offsets[i]);
            prim.indices_count = static_cast<uint32_t>(_objects[i].facets.size() * 3);
            prim.indexing_mode = LIBDAS_SEPERATE_INDICES;
            prim.vertex_buffer_id = VERTICES_ID;
            prim.vertex_normal_buffer_id = NORMALS_ID;

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
        _IndexVertices(_objects);
        
        // check if new file should be opened
        if(_out_file != "")
            NewFile(_out_file);

        InitialiseFile(_props);

        // write all buffers to the file
        std::array<DasBuffer, 3> buffers = _CreateBuffers();
        for(DasBuffer &buf : buffers)
            WriteBuffer(buf);

        std::vector<DasMeshPrimitive> mesh_primitives = _CreateMeshPrimitives(_objects);
        for(DasMeshPrimitive &prim : mesh_primitives)
            WriteMeshPrimitive(prim);

        // write mesh object to the file
        WriteMesh(_CreateMesh(static_cast<uint32_t>(mesh_primitives.size())));

        // generate a default scene if possible
        _CreateDefaultScene();
    }
}
