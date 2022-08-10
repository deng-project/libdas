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
    #include <cfloat>
    #include <unordered_map>
    #include <any>

    #include "trs/Vector.h"
    #include "trs/Matrix.h"
    #include "trs/Points.h"
    #include "trs/Quaternion.h"

    #include "das/Api.h"
    #include "das/ErrorHandlers.h"
    #include "das/DasStructures.h"
    #include "das/GLTFStructures.h"
    #include "das/WavefrontObjStructures.h"
#endif

namespace Libdas {

    namespace DEBUG {
        size_t CaclulateBufferSize(DasBuffer &_buffer);
        void PrintAccumulatedOffsets(std::vector<std::vector<GLTFAccessor*>> &_all_regions);
        void PrintWavefrontObjFacesCount(const WavefrontObjData &_data);
    }
}

#endif
