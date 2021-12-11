/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: AsciiStreamReader.cpp - File stream reader implementation file that reads chunk by chunk 
/// author: Karl-Mihkel Ott

#define STREAM_READER_CPP
#include <AsciiStreamReader.h>


namespace Libdas {

    AsciiStreamReader::AsciiStreamReader(const std::string &_file_name, size_t _chunk_size, const std::string &_end) : 
        m_end(_end), m_buffer_size(_chunk_size) 
    {
        LIBDAS_ASSERT(_chunk_size > 0);
        m_buffer = (char*) std::calloc(m_buffer_size, sizeof(char));
        std::memset(m_buffer, 0, m_buffer_size);

        if(_file_name != "") {
            NewFile(_file_name);
            _ReadNewChunk();
        }
    }


    AsciiStreamReader::~AsciiStreamReader() {
        std::free(m_buffer);
        if(m_stream.is_open())
            m_stream.close();
    }


    bool AsciiStreamReader::_ReadNewChunk() {
        // verify that stream is open
        LIBDAS_ASSERT(m_stream.is_open());
            
        size_t read_bytes = m_stream.tellg();
        m_last_read = std::min(m_buffer_size, m_stream_size - static_cast<size_t>(read_bytes));

        if(m_last_read == 0) return false;

        // clear the buffer and refill it with new data from stream
        std::memset(m_buffer, 0, m_buffer_size);
        m_stream.read(m_buffer, m_last_read);

        read_bytes = m_stream.tellg();

        // using backwards KMP substring search algorithm
        std::vector<size_t> instances = String::FindSubstringInstances(const_cast<const char*>(m_buffer), m_buffer_size, m_end);

        if(instances.size()) {
            int64_t back = static_cast<int64_t>(instances.back()) + static_cast<int64_t>(m_end.size()) - 
                           static_cast<int64_t>(m_last_read);

            m_last_read = instances.back() + m_end.size();
            m_stream.seekg(back, std::ios_base::cur);
        }
        else m_last_read = m_buffer_size;
        return true;
    }


    void AsciiStreamReader::NewFile(const std::string &_file_name) {
        m_stream.open(_file_name.c_str(), std::ios::binary);

        if(m_stream.bad()) {
            std::cerr << "Failed to open file " << _file_name << std::endl;
            std::exit(LIBDAS_ERROR_INVALID_FILE);
        }

        // determine the file size
        m_stream_size = m_stream.tellg();
        m_stream.seekg(0, std::ios::end);
        m_stream_size = static_cast<size_t>(m_stream.tellg()) - m_stream_size;
        m_stream.seekg(0, std::ios::beg);
    }


    void AsciiStreamReader::CloseFile() {
        if(m_stream.is_open())
            m_stream.close();
    }



    char *AsciiStreamReader::GetBufferAddress() {
        return m_buffer;
    }
}
