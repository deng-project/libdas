/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: WavefrontObjCompiler.cpp - Wavefront OBJ to DAS compiler class implementation
/// author: Karl-Mihkel Ott

#define WAVEFRONT_OBJ_COMPILER_CPP
#include <WavefrontObjCompiler.h>


namespace Libdas {

    WavefrontObjCompiler::WavefrontObjCompiler(const std::string &_out_file) : DasWriterCore(_out_file) {}
    

    WavefrontObjCompiler::WavefrontObjCompiler(std::vector<WavefrontObjGroup> &_groups, const std::string &_out_file) :
        DasWriterCore(_out_file)
    {
        Compile(_groups, _out_file);
    }


    std::vector<DasBuffer> WavefrontObjCompiler::_CreateBuffers(std::vector<WavefrontObjGroup> &_groups) {
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
            const char *data;
            size_t size;
            for(const DasBuffer &buffer : buffers) {
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
        }

        return buffers;
    }


    std::vector<DasModel> WavefrontObjCompiler::


    void WavefrontObjCompiler::Compile(std::vector<WavefrontObjGroup> &_groups, const std::string &_out_file) {
        // open a new file if specified
        if(_out_file != "")
            NewFile(_out_file);

        // write all buffers to the output file
        std::vector<DasBuffer> buffers = _CreateBuffers(_groups);
        for(const DasBuffer &buffer : buffers)
            WriteBuffer(buffer);

        // write all given models to the output file
        std::vector<DasModel> models = _CreateModels(_groups);
    }
}
