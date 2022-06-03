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


        void PrintAccumulatedOffsets(std::vector<std::vector<GLTFAccessor*>> &_all_regions) {
            for(auto buf_it = _all_regions.begin(); buf_it != _all_regions.end(); buf_it++) {
                const size_t id = buf_it - _all_regions.begin();
                std::cout << "Buffer id: " << id << std::endl;
                for(auto acc_it = buf_it->begin(); acc_it != buf_it->end(); acc_it++) {
                    std::cout << (*acc_it)->accumulated_offset << std::endl;
                }
            }
        }
    }
}
