#define ASCII_LINE_READER_CPP
#include <AsciiLineReader.h>

namespace Libdas {

    AsciiLineReader::AsciiLineReader(const std::string &_file_name, size_t _chunk_size, const std::string &_end) :
        AsciiStreamReader(_file_name, _chunk_size, _end), m_rd_ptr(m_buffer) {}


    bool AsciiLineReader::_NextLine() {
        char *new_end = nullptr;

        // line ending is saved
        if(m_line_end != 0 && m_line_end + 1 < m_buffer + m_last_read) {
            new_end = std::strchr(m_line_end + 1, static_cast<int>(NEWLINE));
            if(!new_end) new_end = m_buffer + m_last_read;
            m_line_beg = m_line_end + 1;
            m_line_end = new_end;
        }

        else if(m_line_end + 1 >= m_buffer + m_last_read)
            return false;

        else {
            // no line ending is saved
            m_line_beg = m_buffer;
            m_line_end = std::strchr(m_line_beg, static_cast<int>(NEWLINE));
            if(!m_line_end) m_line_end = m_buffer + m_last_read;
        }
        return true;
    }


    std::vector<std::string> AsciiLineReader::_ReadStatementArgs() {
        std::vector<std::string> args;
        while(true) {
            char *end = nullptr;
            _SkipSkippableCharacters();
            if(m_rd_ptr >= m_line_end) break;
            
            end = _ExtractWord();
            std::string arg = std::string(m_rd_ptr, end - m_rd_ptr);
            m_rd_ptr = end;
            if(arg == "\\\\") {
                _NextLine();
                m_rd_ptr = m_line_beg;
                continue;
            }

            args.push_back(arg);
        }

        return args;
    }


    void AsciiLineReader::_SkipSkippableCharacters(bool _skip_nl) {
        // skip all whitespaces till keyword is found
        for(; m_rd_ptr < m_line_end; m_rd_ptr++) {
            if(!_skip_nl) {
                if(*m_rd_ptr != ' ' && *m_rd_ptr != 0x00 && *m_rd_ptr != '\t' && *m_rd_ptr != '\r')
                    break;
             } else {
                if(*m_rd_ptr != ' ' && *m_rd_ptr != 0x00 && *m_rd_ptr != '\t' && *m_rd_ptr != '\r' && *m_rd_ptr != '\n')
                    break;
             }
        }
    }


    bool AsciiLineReader::_SkipData(const size_t _skip_val) {
        if(m_buffer + m_buffer_size <= m_rd_ptr + _skip_val) {
            if(_ReadNewChunk()) {
                m_rd_ptr = m_buffer + (m_rd_ptr + _skip_val - m_buffer + m_buffer_size);
                return true;
            }

            return false;
        }

        m_rd_ptr += _skip_val;
        return true;
    }


    char *AsciiLineReader::_ExtractWord() {
        char *end = m_rd_ptr;
        while(end < m_line_end && end < m_buffer + m_last_read) {
            if(*end == ' ' || *end == 0x00 || *end == '\t' || *end == '\n' || *end == '\r')
                break;

            end++;
        }

        return end;
    }

    
    std::string AsciiLineReader::_ExtractString() {
        char *end = m_rd_ptr + 1;
        bool br = false;

        do {
            end = strchr(end, static_cast<int>('\"'));
            
            if(end == m_buffer || *(end - 1) != '\\')
                br = true;
            else end++;
        } while(!br);

        std::string str_val = std::string(m_rd_ptr, end - m_rd_ptr + 1);

        // check if quotation marks can be avoided
        if(str_val.size() > 2)
            str_val = str_val.substr(1, str_val.size() - 2);

        m_rd_ptr = end + 1;
        return str_val;
    }


    char *AsciiLineReader::_ExtractBlob(uint32_t _size, char *_data) {
        // allocate memory if _data is a nullptr
        bool is_alloc = false;
        if(!_data) {
            _data = reinterpret_cast<char*>(std::malloc(static_cast<size_t>(_size)));
            is_alloc = true;
        }

        char *data_ptr = _data;
        size_t rem = static_cast<size_t>(_size);
        while(rem != 0) {
            // remaining size is larger than remaining data in buffer
            if(m_last_read - (m_rd_ptr - m_buffer) <= rem) {
                std::memcpy(data_ptr, m_rd_ptr, m_last_read - (m_rd_ptr - m_buffer));
                data_ptr += m_last_read - (m_rd_ptr - m_buffer);
                rem -= m_last_read - (m_rd_ptr - m_buffer);
                bool is_read = _ReadNewChunk();

                // return from the subroutine if chunk reading failed
                if(!is_read) {
                    if(is_alloc) std::free(_data);
                    return nullptr;
                }

                _SetReadPtr(m_buffer);

            } else {
                std::memcpy(data_ptr, m_rd_ptr, rem);
                m_rd_ptr += rem;
                rem = 0;
            }
        }

        return _data;
    }


    char *AsciiLineReader::_GetReadPtr() {
        return m_rd_ptr;
    }


    std::pair<char*, char*> AsciiLineReader::_GetLineBounds() {
        return std::make_pair(m_line_beg, m_line_end);
    }


    void AsciiLineReader::_SetReadPtr(char *_ptr) {
        m_rd_ptr = _ptr;
    }


    void AsciiLineReader::_SetLineBounds(const std::pair<char*, char*> &_bounds) {
        m_line_beg = _bounds.first;
        m_line_end = _bounds.second;
    }


    std::string AsciiLineReader::GetFileName() {
        return m_file_name;
    }
}
