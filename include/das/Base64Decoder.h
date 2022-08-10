// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: Base64Decoder.h - Base64 encoded data decoder class header
// author: Karl-Mihkel Ott

#ifndef BASE64_DECODER_H
#define BASE64_DECODER_H


#ifdef BASE64_DECODER_CPP
    #include <cstdint>
    #include <string>
    #include <vector>
    #include <utility>
    #include <algorithm>

    #include "das/Api.h"
    #define LIBDAS_DEFS_ONLY
    #include "das/HuffmanCompression.h"
    #undef LIBDAS_DEFS_ONLY
#endif

namespace Libdas {

    class LIBDAS_API Base64Decoder {
        private:
            std::string m_base64_str;
            uint8_t m_base64_table[LIBDAS_ASCII_ALPHABET_SIZE] = {0};

        public:
            Base64Decoder(const std::string &_base64_str = "");
            /**
             * Decode base64 encoded string into char array
             * @param _base64_str optionally specifies the base64 string to decode
             * @return std::vector instance containing decoded data
             */
            std::vector<char> Decode(const std::string &_base64_str = "");
    };
}

#endif
