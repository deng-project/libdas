// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: Debug.h - Header for debugging methods
// author: Karl-Mihkel Ott

#ifndef DEBUG_H
#define DEBUG_H

#ifdef DEBUG_CPP
    #include <string>
    #include <vector>
    #include <cstdint>
    #include <iostream>
    #include <cstring>
    #include <cmath>

    #include <Vector.h>
    #include <Matrix.h>
    #include <Points.h>
    #include <Quaternion.h>
    #include <DasStructures.h>
#endif

namespace Libdas {

    namespace DEBUG {
        size_t CaclulateBufferSize(DasBuffer &_buffer);
    }
}

#endif
