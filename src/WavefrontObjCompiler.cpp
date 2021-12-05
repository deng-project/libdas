/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: WavefrontObjCompiler.cpp - Wavefront OBJ to DAS compiler class implementation
/// author: Karl-Mihkel Ott

#define WAVEFRONT_OBJ_COMPILER_CPP
#include <WavefrontObjCompiler.h>


namespace Libdas {

    WavefrontObjCompiler::WavefrontObjCompiler(const std::string &_out_file, bool use_huffman) : 
        DasWriterCore(_out_file), m_use_huffman(use_huffman) {}
    

    WavefrontObjCompiler::WavefrontObjCompiler(const std::vector<WavefrontObjGroup> &_groups, const DasProperties &_props, const std::string &_out_file, bool use_huffman) :
        DasWriterCore(_out_file), m_use_huffman(use_huffman)
    {
        Compile(_groups, _props, _out_file);
    }


    std::vector<DasBuffer> WavefrontObjCompiler::_CreateBuffers(const std::vector<WavefrontObjGroup> &_groups) {
        // by default initialise following buffer types
        //  * Vertex buffer
        //  * Texture vertices buffer
        //  * Vertex normal buffer
        //  * Vertex indicies buffer
        std::vector<DasBuffer> buffers(4);
        buffers[0].type = LIBDAS_BUFFER_TYPE_VERTEX;
        buffers[1].type = LIBDAS_BUFFER_TYPE_TEXTURE_MAP;
        buffers[2].type = LIBDAS_BUFFER_TYPE_VERTEX_NORMAL;
        buffers[3].type = LIBDAS_BUFFER_TYPE_INDICIES;

        for(const WavefrontObjGroup &group : _groups) {
            const char *data = nullptr;
            size_t size = 0;

            // vertices are given, append to the buffer
            if(group.vertices.position.size()) {
                data = reinterpret_cast<const char*>(group.vertices.position.data());
                size = group.vertices.position.size() * sizeof(Point4D<float>);
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
                size = group.indices.faces.size() * sizeof(DasFace);
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

        return buffers;
    }


    std::vector<DasModel> WavefrontObjCompiler::_CreateModels(const std::vector<WavefrontObjGroup> &_groups) {
        std::vector<DasModel> models(_groups.size());

        // buffer id constants declarations
        const uint32_t vertex_id = 0;
        const uint32_t texture_map_id = 1;
        const uint32_t vertex_normal_id = 2;
        const uint32_t indices_id = 3;

        // offset values
        uint32_t indices_offset = 0;

        for(size_t i = 0; i < _groups.size(); i++) {
            models[i].name = String::ConcatenateNameArgs(_groups[i].names);
            models[i].index_buffer_id = indices_id;
            models[i].index_buffer_offset = indices_offset;
            models[i].indices_count = _groups[i].indices.faces.size();
            models[i].vertex_buffer_id = vertex_id;
            models[i].texture_map_buffer_id = texture_map_id;
            models[i].vertex_normal_buffer_id = vertex_normal_id;
        }

        return models;
    }


    void WavefrontObjCompiler::_HuffmanEncode() {
        std::string tmp_file = m_file_name + ".huf";
        // close the output stream
        CloseStream();
        
        // create new streams and huffman encoder class instance
        std::ifstream in(m_file_name, std::ios_base::binary);
        std::ofstream out(tmp_file, std::ios_base::binary);

        //DasReaderCore rd(in, out);
    }


    void WavefrontObjCompiler::Compile(const std::vector<WavefrontObjGroup> &_groups, const DasProperties &_props, const std::string &_out_file) {
        // open a new file if specified
        if(_out_file != "")
            NewFile(_out_file);

        InitialiseFile(_props);

        // write all buffers to the output file
        std::vector<DasBuffer> buffers = _CreateBuffers(_groups);
        for(const DasBuffer &buffer : buffers)
            WriteBuffer(buffer);

        // write all given models to the output file
        std::vector<DasModel> models = _CreateModels(_groups);
        for(const DasModel &model : models)
            WriteModel(model);

        // check if huffman encoding is requested
        if(m_use_huffman)
            _HuffmanEncode();
    }
}
