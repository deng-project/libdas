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

    #include "trs/Vector.h"
    #include "trs/Matrix.h"
    #include "trs/Points.h"
    #include "trs/Quaternion.h"

    
    #include "das/Api.h"
    #include "das/LibdasAssert.h"
    #include "das/ErrorHandlers.h"
    #include "das/DasStructures.h"

    // third party includes
    #define STB_IMAGE_IMPLEMENTATION
    #include "das/stb_image.h"
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
            std::string m_file_name = "";
            char *m_buffer = nullptr;
            size_t m_buffer_size = 0;
            char *m_raw_buffer = nullptr;
            size_t m_raw_buffer_size = 0;
            bool m_is_attached_buffer;
            bool m_is_attached_raw_buffer;

            // raw buffer data
            int m_x = 0, m_y = 0;

        public:
            TextureReader(const std::string &_file_name, bool _use_raw = true);
            TextureReader(std::pair<char*, size_t> _raw_data, int _x, int _y, bool _use_raw = true);
            TextureReader(TextureReader &&_mov);
            ~TextureReader();
            /**
             * Read all data from image file into buffer and return the pointer.
             * Allocated heap memory is automatically freed
             * @return const char* pointer to the image file buffer read into memory
             */
            char* GetBuffer(size_t &_len) {
                _len = m_buffer_size;
                return m_buffer;
            }
            /**
             * Read the image stream and return a pointer to raw image data
             * Allocated heap memory is automatically freed
             * @return const char* pointer to the raw image data buffer
             */
            char *GetRawBuffer(int &_x, int &_y, size_t &_len) {
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
