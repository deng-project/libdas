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
    #include <Points.h>
    #include <Quaternion.h>
    #include <DasStructures.h>

    // third party includes
    #define STB_IMAGE_IMPLEMENTATION
    #include <stb_image.h>
#endif


namespace Libdas {

    struct RawImageDataHeader {
        uint32_t width;
        uint32_t height;
        uint8_t bit_depth;
    };

    // TODO: Change C FILE fopen calls with C++ fstream class implementation

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

            // raw buffer data
            int m_x = 0, m_y = 0;

        public:
            TextureReader(const std::string &_file_name, bool _use_raw = true);
            TextureReader(TextureReader &&_mov);
            ~TextureReader();
            /**
             * Read all data from image file into buffer and return the pointer.
             * Allocated heap memory is automatically freed
             * @return const char* pointer to the image file buffer read into memory
             */
            const char* GetBuffer(size_t &_len) {
                _len = m_buffer_size;
                return m_buffer;
            }
            /**
             * Read the image stream and return a pointer to raw image data
             * Allocated heap memory is automatically freed
             * @return const char* pointer to the raw image data buffer
             */
            const char *GetRawBuffer(int &_x, int &_y, size_t &_len) {
                _x = m_x;
                _y = m_y;
                _len = m_raw_buffer_size;
                return m_raw_buffer;
            }
            /**
             * Check the file name and get the correct buffer type
             * @return BufferType value that specifies the image buffer type
             */
            BufferType GetImageBufferType();
    };
}

#endif
