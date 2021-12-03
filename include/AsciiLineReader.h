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

    class AsciiLineReader : public AsciiStreamReader {
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
            AsciiLineReader(const std::string &_file_name, size_t _chunk_size = DEFAULT_CHUNK, const std::string &_end = "\n");
            /**
             * Skip all skippable characters (' ', 0x00, '\t', '\d' and or '\n')
             * @param _skip_nl is a boolean flag indicating newline skipping
             */
            void SkipSkippableCharacters(bool _skip_nl = false);
            /**
             * Skip the reading pointer by specified amount
             * @param _skip_val is a size_t value that specifies the read_ptr offset in bytes
             */
            void SkipData(const size_t _skip_val);
            /**
             * Extract word statement, assuming that the starting position character is not skippable
             * @return End position of the word
             */
            char *ExtractWord();
            /**
             * Extract string value from buffer (value must be in between quotation marks)
             * @return std::string value that contains the string value without quotation marks
             */
            std::string ExtractString();
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
