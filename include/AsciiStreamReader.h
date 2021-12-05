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
    #include <ErrorHandlers.h>
#endif


namespace Libdas {

    /**
     * Parent class for AsciiLineReader and similar ASCII data parsing classes.
     * The interface allows reading data step by step into specific chunk buffer
     */
    class AsciiStreamReader {
        private:
            std::string m_end;
            std::ifstream m_stream;
            size_t m_stream_size = 0;

        protected:
            char *m_buffer;
            size_t m_buffer_size;
            size_t m_last_read = 0;

        private:
            /**
             * Create longest suffix-prefix table from end string for KMP algorithm
             * @return int array containing the LSP table
             */
            size_t *_CreateLSPArray();
            /**
             * Find all end string locations from given buffer using kmp algorithm (O(n))
             * @return std::vector<int> instance containing all substring occurence indices
             */
            std::vector<size_t> _FindEndStringInstances();

        protected:
            AsciiStreamReader(const std::string &_file_name = "", size_t _chunk_size = DEFAULT_CHUNK, const std::string &_end = "\n");
            ~AsciiStreamReader();
            /**
             * Read new buffer chunk from the stream
             * @return true if new chunk value was read
             */
            bool _ReadNewChunk();
            /**
             * Get the current stream reading std::ifstream reference 
             * @return reference to std::ifstream 
             */
            std::ifstream &_GetStream();

        public:
            /**
             * Open new file stream for reading data from
             * @param _file_name is an std::string referece value that represents the used file name
             */
            void NewFile(const std::string &_file_name);
            /**
             * Close previously opened file stream if possible
             */
            void CloseFile();
            /**
             * Read custom binary blob from seemingly binary file
             * @param _cont is a reference to the data structure where binary data is read into
             */
            template<typename T>
            void ReadBlob(T &_cont) {
                m_stream.read(reinterpret_cast<char*>(&_cont), sizeof(T));
            }
            /**
             * Get the chunk buffer starting address
             * @return Starting address of the chunk buffer
             */
            char *GetBufferAddress();
    };
}

#endif
