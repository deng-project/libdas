/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: StreamReader.h - File stream reader header file that reads chunk by chunk 
/// author: Karl-Mihkel Ott

#ifndef STREAM_READER_H
#define STREAM_READER_H


#define DEFAULT_CHUNK   4096

#ifdef STREAM_READER_CPP
    #include <cstring>
    #include <string>
    #include <vector>
    #include <fstream>
    #include <iostream>
    #include <algorithm>

    #include <LibdasAssert.h>
    #include <ParserErrorHandler.h>
#endif


namespace Libdas {

    class StreamReader {
        private:
            const char m_end;
            std::ifstream m_stream;
            size_t m_stream_size = 0;

        protected:
            char *m_buffer;
            size_t m_buffer_size;
            size_t m_last_read = 0;

        public:
            StreamReader(size_t _chunk_size, char _end);
            StreamReader(size_t _chunk_size, const std::string &_file_name, char _end);
            ~StreamReader();

            void OpenStream(const std::string &_file_name);
            bool ReadNewChunk();
            void CloseStream();

            char *GetBufferAddress();
            const size_t GetLastRead();
    };
}

#endif
