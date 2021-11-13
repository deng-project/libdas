/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: AsciiStreamReader.cpp - File stream reader implementation file that reads chunk by chunk 
/// author: Karl-Mihkel Ott

#define STREAM_READER_CPP
#include <AsciiStreamReader.h>


namespace Libdas {


    AsciiStreamReader::AsciiStreamReader(size_t _chunk_size, char _end, const std::string &_file_name) : 
        m_end(_end), m_buffer_size(_chunk_size) 
    {
        LIBDAS_ASSERT(_chunk_size > 0);
        m_buffer= (char*) std::malloc(m_buffer_size);
        std::memset(m_buffer, 0, m_buffer_size);

        if(_file_name != "") {
            OpenStream(_file_name);
            ReadNewChunk();
        }
    }


    AsciiStreamReader::~AsciiStreamReader() {
        std::free(m_buffer);
        if(m_stream.is_open())
            m_stream.close();
    }


    void AsciiStreamReader::OpenStream(const std::string &_file_name) {
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


    bool AsciiStreamReader::ReadNewChunk() {
        // verify that stream is open
        LIBDAS_ASSERT(m_stream.is_open());
            
        const size_t read_bytes = m_stream.tellg();
        m_last_read = std::min(m_buffer_size, m_stream_size - static_cast<size_t>(read_bytes));

        if(m_last_read == 0) return false;

        // find the first instance of end character
        for(int32_t i = static_cast<int32_t>(m_last_read) - 1; i >= 0; i--) {
            m_stream.seekg(i + read_bytes, std::ios::beg);
            char ch;
            m_stream.get(ch);

            if(ch == m_end) {
                m_last_read = static_cast<size_t>(i) + 1;
                break;
            }
        }

        m_stream.seekg(read_bytes, std::ios::beg);

        // clear the buffer and refill it with new data from stream
        std::memset(m_buffer, 0, m_buffer_size);
        m_stream.read(m_buffer, m_last_read);
         
        return true;
    }


    void AsciiStreamReader::CloseStream() {
        m_stream.close();
    }


    char *AsciiStreamReader::GetBufferAddress() {
        return m_buffer;
    }


    const size_t AsciiStreamReader::GetLastRead() {
        return m_last_read;
    }
}
