#ifndef ASCII_LINE_READER_H
#define ASCII_LINE_READER_H


#ifdef ASCII_LINE_READER_CPP
    #include <cstring>
    #include <string>
    #include <vector>
    #include <fstream>
    #include <utility>

    #include <AsciiStreamReader.h>
#endif


namespace Libdas {

    class AsciiLineReader : protected AsciiStreamReader {
        private:
            const char m_end;

        protected:
            char *m_line_beg = nullptr;
            char *m_line_end = nullptr;
            char *m_rd_ptr = nullptr;

        protected:
            /** 
             * Read new line from buffer
             * @return True if newline was found, false otherwise
             */
            bool _NextLine();
            /**
             * Skip all skippable characters (' ', 0x00, '\t')
             * @param _end is the last byte in the buffer 
             */
            void _SkipSkippableCharacters(char *_end);
            /**
             * Extract word statement, assuming that the starting position charcter is not skippable
             * @return End position of the word
             */
            char *_ExtractWord();
            /**
             * Read all general statement arguments until the end of line
             */
            std::vector<std::string> _ReadStatementArgs();


        public:
            AsciiLineReader(size_t _chunk_size, const char _end = '\n', const std::string &_file_name = "");
            /**
             * @return std::pair object where the first element specifies beginning of the line and second element
             * specifies the end of line
             */
            std::pair<char*, char*> GetLineBounds();
    };
}

#endif
