/// libdas: DENG asset handling management library
/// licence: Apache, see licence file
/// file: TextureReader.h - Generic texture image reader class header
/// author: Karl-Mihkel Ott

#define TEXTURE_READER_CPP
#include <TextureReader.h>

namespace Libdas {

    TextureReader::TextureReader(const std::string &_file_name, bool _use_raw) : 
        m_file_name(_file_name),
        m_is_attached_buffer(false)
    {
        std::ifstream file(_file_name, std::ios_base::binary);

        // check if error occured, while opening the stream
        if(file.fail()) {
            std::cerr << "Failed to read texture file " << _file_name << std::endl;
            EXIT_ON_ERROR(LIBDAS_ERROR_INVALID_FILE);
        }

        // read file data into buffer
        file.seekg(0, std::ios_base::end);
        m_buffer_size = file.tellg();
        file.seekg(0, std::ios_base::beg);

        char *buffer = new char[m_buffer_size];
        file.read(buffer, m_buffer_size);
        m_buffer = buffer;

        if(_use_raw) {
            int n;
            m_raw_buffer = reinterpret_cast<char*>(stbi_load_from_memory(reinterpret_cast<const unsigned char*>(m_buffer),
                                                                         static_cast<int>(m_buffer_size), &m_x, &m_y, &n, 4));
            m_raw_buffer_size = static_cast<size_t>(m_x * m_y * 4);
        }

        file.close();
    }


    TextureReader::TextureReader(std::pair<const char*, size_t> _raw_data, bool _use_raw) : 
        m_buffer(_raw_data.first), 
        m_buffer_size(_raw_data.second),
        m_is_attached_buffer(true)
    {
        if(_use_raw) {
            int n;
            m_raw_buffer = reinterpret_cast<char*>(stbi_load_from_memory(reinterpret_cast<const unsigned char*>(m_buffer),
                                                                         static_cast<int>(m_buffer_size), &m_x, &m_y, &n, 4));
            m_raw_buffer_size = static_cast<size_t>(m_x * m_y * 4);
        }
    }


    TextureReader::TextureReader(TextureReader &&_mov) : 
        m_file_name(_mov.m_file_name), 
        m_buffer(_mov.m_buffer), 
        m_buffer_size(_mov.m_buffer_size),
        m_raw_buffer(_mov.m_raw_buffer), 
        m_raw_buffer_size(_mov.m_raw_buffer_size), 
        m_is_attached_buffer(_mov.m_is_attached_buffer),
        m_x(_mov.m_x), 
        m_y(_mov.m_y)
    {
        _mov.m_buffer = nullptr;
        _mov.m_raw_buffer = nullptr;
    }


    TextureReader::~TextureReader() {
        if(!m_is_attached_buffer && m_buffer) 
            delete [] m_buffer;
        if(m_raw_buffer) 
            std::free(m_raw_buffer);
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
