// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: Hash.h - Universal hashing header
// author: Karl-Mihkel Ott

#ifndef HASH_H
#define HASH_H

#ifdef HASH_CPP
    #include <cstddef>
    #include <cstdint>
    #include <Api.h>
    #define PRIME   0x9E3779B1   
#endif

namespace Libdas {

    /**
     * Universal hashing function used for defining hashes for custom data types
     * @param _data specifies any arbitrary data pointer
     * @param _len specifies the data length in bytes
     */
    uint32_t HashFunc(char const* _data, size_t _len);

    template <class T>
    struct LIBDAS_API Hash {
        typedef T argument_type;
        typedef uint32_t result_type;

        inline result_type operator()(argument_type const& _s) const {
            char const *data = reinterpret_cast<char const*>(&_s);
            return HashFunc(data, sizeof(argument_type));
        }
    };
}

#endif
