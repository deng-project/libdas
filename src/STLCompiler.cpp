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
        std::unordered_map<Point3D<float>, uint32_t> pos_map;
        std::unordered_map<Point3D<float>, uint32_t> norm_map;


        uint32_t max_pos = 0;
        uint32_t max_norm = 0;
        for(const STLObject &obj : _objects) {
            m_index_offsets.push_back(m_faces.size() * sizeof(DasFace));
            for(size_t i = 0; i < obj.facets.size(); i++) {
                DasFace face;

                // check if vertex normal key exists in normals hashmap
                if(norm_map.find(obj.facets[i].normal) != norm_map.end())
                    face.normal = norm_map[obj.facets[i].normal];
                else {
                    face.normal = max_pos;
                    max_pos++;
                    m_unique_normals.push_back(obj.facets[i].normal);
                    norm_map[obj.facets[i].normal] = face.normal;
                }

                // check if the position vertices already exist in the position vertex map
                for(const Point3D<float> &pos : obj.facets[i].vertices) {
                    if(pos_map.find(pos) != pos_map.end())
                        face.position = pos_map[pos];
                    else {
                        face.position = max_pos;
                        max_pos++;
                        m_unique_verts.push_back(Point4D<float>(pos));
                        pos_map[pos] = face.position;
                    }
                }

                m_faces.push_back(face);
            }
        }
    }


    std::array<DasBuffer, 3> STLCompiler::_CreateBuffers() {
        // 0 - vertices buffer
        // 1 - normals buffer
        // 2 - indices buffer
        std::array<DasBuffer, 3> buffers;

        // create position vertices buffer
        buffers[0].type = LIBDAS_BUFFER_TYPE_VERTEX;
        buffers[0].data_len = m_unique_verts.size() * sizeof(Point4D<float>);
        buffers[0].data_ptrs.push_back(std::make_pair(reinterpret_cast<const char*>(m_unique_verts.data()), buffers[0].data_len));

        // create vertex normal buffer
        buffers[1].type = LIBDAS_BUFFER_TYPE_VERTEX_NORMAL;
        buffers[1].data_len = m_unique_normals.size() * sizeof(Point3D<float>);
        buffers[1].data_ptrs.push_back(std::make_pair(reinterpret_cast<const char*>(m_unique_normals.data()), buffers[1].data_len));

        // create indices buffer
        buffers[2].type = LIBDAS_BUFFER_TYPE_INDICES;
        buffers[2].data_len = m_faces.size() * sizeof(DasFace);
        buffers[2].data_ptrs.push_back(std::make_pair(reinterpret_cast<const char*>(m_faces.data()), buffers[2].data_len));

        return buffers;
    }


    std::vector<DasModel> STLCompiler::_CreateModels(const std::vector<STLObject> &_objects) {
        std::vector<DasModel> models(_objects.size());
        for(size_t i = 0; i < _objects.size(); i++) {
            models[i].name = _objects[i].name;
            models[i].vertex_buffer_id = VERTICES_ID;
            models[i].vertex_normal_buffer_id = NORMALS_ID;
            models[i].index_buffer_id = INDICES_ID;
            models[i].index_buffer_offset = m_index_offsets[i];
            models[i].indices_count = i == _objects.size() - 1 ? m_faces.size() - m_index_offsets[i] / sizeof(DasFace) : (m_index_offsets[i + 1] - m_index_offsets[i]) / sizeof(DasFace);
        }

        return models;
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

        // write all models (objects to the file)
        std::vector<DasModel> models = _CreateModels(_objects);
        for(DasModel &model : models)
            WriteModel(model);
    }
}
