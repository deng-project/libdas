/// libdas: deng asset handling management library
/// licence: apache, see licence file
/// file: texturereader.h - generic texture image reader class header
/// author: karl-mihkel ott

#define TEXTURE_READER_CPP
#include <TextureReader.h>

namespace Libdas {

    TextureReader::TextureReader(const std::string &_file_name) : m_file_name(_file_name) {
        m_stream = fopen(_file_name.c_str(), "rb");

        // check if error occured, while opening the stream
        if(!m_stream) {
            std::cerr << "Failed to read texture file " << _file_name << std::endl;
            std::exit(LIBDAS_ERROR_INVALID_FILE);
        }
    }


    TextureReader::TextureReader(const std::string &_file_name, std::ifstream &_stream, size_t _img_size) :
        m_buffer_size(_img_size), m_file_name(_file_name)
    {
        m_buffer = reinterpret_cast<char*>(malloc(m_buffer_size));
        _stream.read(m_buffer, m_buffer_size);
    }


    TextureReader::~TextureReader() {
        if(m_buffer) free(m_buffer);
        if(m_raw_buffer) free(m_raw_buffer);
        if(m_stream) fclose(m_stream);
    }


    const char *TextureReader::GetBuffer() {
        return const_cast<const char*>(m_buffer);
    }


    size_t TextureReader::GetBufferSize() {
        return m_buffer_size;
    }


    const char *TextureReader::GetRawBuffer(int *_x, int *_y) {
        LIBDAS_ASSERT(m_buffer || m_stream);

        // check if cleaning the raw data memory is needed
        if(m_raw_buffer) {
            free(m_raw_buffer);
            m_raw_buffer = nullptr;
            m_raw_buffer_size = 0;
        }

        // no stream present, read from the buffer
        if(!m_stream) {
            int n;
            m_raw_buffer = reinterpret_cast<char*>(stbi_load_from_memory(reinterpret_cast<unsigned char* const>(m_buffer), 
                                                                         static_cast<int>(m_buffer_size), _x, _y, &n, 4));
            m_raw_buffer_size = (*_x) * (*_y) * 4;
        }

        // stream is present read from it
        else {
            int n;
            m_raw_buffer = reinterpret_cast<char*>(stbi_load_from_file(m_stream, _x, _y, &n, 4));
            m_raw_buffer_size = (*_x) * (*_y) * 4;
        }

        return m_raw_buffer;
    }


    size_t TextureReader::GetRawBufferSize() {
        return m_raw_buffer_size;
    }


    BufferType TextureReader::GetImageBufferType() {
        std::string ext = "";
        // find the first '.' instance from the end of file name
        for(int i = static_cast<int>(m_file_name.size() - 1); i >= 0; i--) {
            if(m_file_name[i] == '.')
                ext = m_file_name.substr(i + 1);
        }

        if(ext == "jpeg" || ext == "jpg" || ext == "JPEG" || ext == "JPG")
            return LIBDAS_BUFFER_TYPE_TEXTURE_JPEG;
        else if(ext == "png" || ext == "PNG")
            return LIBDAS_BUFFER_TYPE_TEXTURE_PNG;
        else if(ext == "tga" || ext == "TGA")
            return LIBDAS_BUFFER_TYPE_TEXTURE_TGA;
        else if(ext == "bmp" || ext == "BMP")
            return LIBDAS_BUFFER_TYPE_TEXTURE_BMP;
        else if(ext == "ppm" || ext == "PPM")
            return LIBDAS_BUFFER_TYPE_TEXTURE_PPM;
        else return LIBDAS_BUFFER_TYPE_TEXTURE_RAW;
    }
}
