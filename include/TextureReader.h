/// libdas: deng asset handling management library
/// licence: apache, see licence file
/// file: texturereader.h - generic texture image reader class header
/// author: karl-mihkel ott

#ifndef TEXTURE_READER_H
#define TEXTURE_READER_H


#ifdef TEXTURE_READER_CPP
    #include <fstream>
    #include <vector>
    #include <iostream>
    #include <cstring>
    #include <cmath>

    #include <LibdasAssert.h>
    #include <ErrorHandlers.h>
    
    #include <Api.h>
    #include <Vector.h>
    #include <Matrix.h>
    #include <Quaternion.h>
    #include <DasStructures.h>

    // third party includes
    #define STB_IMAGE_IMPLEMENTATION
    #include <stb_image.h>
#endif


namespace Libdas {

    /**
     * Interface for reading and handling texture files 
     */
    class LIBDAS_API TextureReader {
        private:
            char *m_buffer = nullptr;
            size_t m_buffer_size = 0;
            char *m_raw_buffer = nullptr;
            size_t m_raw_buffer_size = 0;
            std::string m_file_name = "";
            FILE *m_stream = nullptr;

        public:
            TextureReader(const std::string &_file_name);
            TextureReader(const std::string &_file_name, std::ifstream &_stream, size_t _img_size);
            ~TextureReader();
            /**
             * Read all data from image file into buffer and return the pointer.
             * Allocated heap memory is automatically freed
             * @return const char* pointer to the image file buffer read into memory
             */
            inline const char* GetBuffer(size_t &_len) {
                _len = m_buffer_size;
                return m_buffer;
            }
            /**
             * Read the image stream and return a pointer to raw image data
             * Allocated heap memory is automatically freed
             * @return const char* pointer to the raw image data buffer
             */
            const char *GetRawBuffer(int &_x, int &_y, size_t &_len);
            /**
             * Check the file name and get the correct buffer type
             * @return BufferType value that specifies the image buffer type
             */
            BufferType GetImageBufferType();
    };
}

#endif
