#define ASCII_LINE_READER_CPP
#include <AsciiLineReader.h>

namespace Libdas {

    AsciiLineReader::AsciiLineReader(const std::string &_file_name, size_t _chunk_size, const std::string &_end) :
        AsciiStreamReader(_file_name, _chunk_size, _end), m_end(_end), m_rd_ptr(m_buffer) {}


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
        }
        return true;
    }


    std::vector<std::string> AsciiLineReader::_ReadStatementArgs() {
        std::vector<std::string> args;
        while(true) {
            char *end = nullptr;
            SkipSkippableCharacters();
            if(m_rd_ptr >= m_line_end) break;
            
            end = ExtractWord();
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


    void AsciiLineReader::SkipSkippableCharacters(bool _skip_nl) {
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


    char *AsciiLineReader::ExtractWord() {
        char *end = m_rd_ptr;
        while(end < m_line_end) {
            if(*end == ' ' || *end == 0x00 || *end == '\t' || *end == '\n' || *end == '\r')
                break;

            end++;
        }

        return end;
    }


    char *AsciiLineReader::GetReadPtr() {
        return m_rd_ptr;
    }


    std::pair<char*, char*> AsciiLineReader::GetLineBounds() {
        return std::make_pair(m_line_beg, m_line_end);
    }


    void AsciiLineReader::SetReadPtr(char *_ptr) {
        m_rd_ptr = _ptr;
    }


    void AsciiLineReader::SetLineBounds(const std::pair<char*, char*> &_bounds) {
        m_line_beg = _bounds.first;
        m_line_end = _bounds.second;
    }
}
