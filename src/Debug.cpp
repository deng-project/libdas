// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: Debug.cpp - Implementation for debugging methods
// author: Karl-Mihkel Ott

#define DEBUG_CPP
#include <Debug.h>

namespace Libdas {

    namespace DEBUG {

        size_t CalculateBufferSize(DasBuffer &_buffer) {
            size_t s = 0;
            for(auto it = _buffer.data_ptrs.begin(); it != _buffer.data_ptrs.end(); it++)
                s += it->second;

            return s;
        }
    }
}
