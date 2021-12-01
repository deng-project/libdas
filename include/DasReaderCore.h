/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: DasReaderCore.h - DAS file format reader class header
/// author: Karl-Mihkel Ott

#ifndef DAS_READER_CORE_H
#define DAS_READER_CORE_H


#ifdef DAS_READER_CORE_CPP
    #include <cstring>
    #include <string>
    #include <fstream>
#ifdef _DEBUG
    #include <iostream>
#endif
    #include <vector>
    #include <unordered_map>
    
    #include <Vector.h>
    #include <Matrix.h>
    #include <AsciiStreamReader.h>
    #include <AsciiLineReader.h>
    #include <DasStructures.h>
    #include <LibdasAssert.h>
    #include <ErrorHandlers.h>
#endif


namespace Libdas {

    /**
     * Parent class to read data from DAS file format
     */
    class DasReaderCore : private AsciiLineReader {
        private:
            const std::string &m_file_name;
            std::ifstream m_in_stream;
            BinaryFormatErrorHandler m_error;

        public:
            DasReaderCore(const std::string &_file_name = "");
            /**
             * Open a new file for reading and close previous one if needed
             * @param _file_name is a specified file name to use for file streams
             */
            void NewFile(const std::string &_file_name);
            /**
             * Close previously opened stream if necessary
             */
            void CloseStream();
            /**
             * Read and verify file signature
             */
            void ReadSignature();
            /**
             * Read file properties and return their instance
             * @return DasProperties value defining the properties for given file
             */
           DasProperties ReadProperties();

           // incomplete list ...
    };
}

#endif
