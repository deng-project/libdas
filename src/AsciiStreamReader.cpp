/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: AsciiStreamReader.cpp - File stream reader implementation file that reads chunk by chunk 
/// author: Karl-Mihkel Ott

#define STREAM_READER_CPP
#include <AsciiStreamReader.h>


namespace Libdas {

    namespace String {

        std::string ReadFileToString(const std::string &_file_name) {
            std::string abs_path = Algorithm::RelativePathToAbsolute(_file_name);
            std::ifstream file(abs_path, std::ios_base::binary);
            
            // get the file size
            file.seekg(0, std::ios_base::end);
            size_t len = file.tellg();
            file.seekg(0, std::ios_base::beg);

            std::vector<char> data(len);
            file.read(data.data(), data.size());

            if(file.fail()) {
                std::cerr << "Failed to read file " << abs_path << std::endl; 
                EXIT_ON_ERROR(1);
            }

            return std::string(data.data(), data.size());
        }
    }

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
        std::vector<size_t> instances = Algorithm::FindSubstringInstances(const_cast<const char*>(m_buffer), m_buffer_size, m_end);

        if(instances.size()) {
            int64_t back = static_cast<int64_t>(instances.back()) + static_cast<int64_t>(m_end.size()) - 
                           static_cast<int64_t>(m_last_read);

            m_last_read = instances.back() + m_end.size();
            m_stream.seekg(back, std::ios_base::cur);
        }
        else {
            // check if any chars of "\nENDSCOPE" are present at the end of buffer
            std::string endscope_str = "\nENDSCOPE";
            char e = endscope_str.back();

            size_t offset = m_buffer_size - 1;
            bool is_partial_match = true;
            while(endscope_str.size()) {
                if(m_buffer[offset] == e) {
                    e = endscope_str.back();
                    offset--;
                } else if(offset != m_buffer_size - 1) {
                    is_partial_match = false;
                    break;
                } else {
                    e = endscope_str.back();
                }
                endscope_str.pop_back();
            }

            if(!is_partial_match)
                m_last_read = m_buffer_size;
            else {
                int64_t back = m_buffer_size - offset - 1;
                m_last_read = m_buffer_size - back;
                m_stream.seekg(-back, std::ios_base::cur);
            }
        }
        return true;
    }


    void AsciiStreamReader::NewFile(const std::string &_file_name) {
        // check if stream was previously opened, and if it was, close it
        if(m_stream.is_open())
            m_stream.close();

        std::string abs_file = Algorithm::RelativePathToAbsolute(_file_name);
        m_stream.open(abs_file.c_str(), std::ios::binary);

        if(!m_stream.is_open()) {
            std::cerr << "Failed to open file " << abs_file << std::endl;
            EXIT_ON_ERROR(LIBDAS_ERROR_INVALID_FILE);
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
