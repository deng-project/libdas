/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: WavefrontObjParseTest.h - WavefrontObjParser class testing application
/// author: Karl-Mihkel Ott

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

#define NEWLINE '\n'


namespace Libdas {

    class AsciiLineReader : protected AsciiStreamReader {
        private:
            const std::string &m_end;

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
             * Read all general statement arguments until the end of line
             */
            std::vector<std::string> _ReadStatementArgs();


        public:
            AsciiLineReader(size_t _chunk_size = DEFAULT_CHUNK, const std::string &_end = "\n", const std::string &_file_name = "");
            /**
             * Skip all skippable characters (' ', 0x00, '\t')
             * @param _end is the last byte in the buffer 
             */
            void SkipSkippableCharacters();
            /**
             * Extract word statement, assuming that the starting position charcter is not skippable
             * @return End position of the word
             */
            char *ExtractWord();
            /**
             * @return current reading pointer
             */
            char *GetReadPtr();
            /**
             * @return std::pair object where the first element specifies beginning of the line and second element
             * specifies the end of line
             */
            std::pair<char*, char*> GetLineBounds();
            /**
             * Explicitly set the m_rd_ptr value
             * @param _ptr, new m_rd_ptr value
             */
            void SetReadPtr(char *_ptr);
            /**
             * Explicitly set the line bounds
             * @param _bounds is a std::pair value, where the first element states line beginning and second element states the line ending
             */
            void SetLineBounds(const std::pair<char*, char*> &_bounds);
    };
}

#endif
