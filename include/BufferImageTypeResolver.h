// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: BufferImageTypeResolver.h - Resolve buffer image type from specified URI class header
// author: Karl-Mihkel Ott

#ifndef BUFFER_IMAGE_TYPE_RESOLVER_H
#define BUFFER_IMAGE_TYPE_RESOLVER_H

#ifdef BUFFER_IMAGE_TYPE_RESOLVER_CPP
    #include <any>
    #include <string>
    #include <optional>
    #include <cstdint>
    #include <cfloat>
    #include <cstring>
    #include <string>
    #include <vector>
    #include <unordered_map>

    #include <Api.h>
    #include <Points.h>
    #include <Vector.h>
    #include <Matrix.h>
    #include <GLTFStructures.h>
    #include <FileNameString.h>

#define LIBDAS_DEFS_ONLY
    #include <DasStructures.h>
#undef LIBDAS_DEFS_ONLY
#endif

namespace Libdas {

    class LIBDAS_API BufferImageTypeResolver {
        private:
            BufferType m_resolved_type = LIBDAS_BUFFER_TYPE_UNKNOWN;

        private:
            bool _CheckJPEGSignature(const std::vector<char> &_resource);
            bool _CheckPNGSignature(const std::vector<char> &_resource);
            bool _CheckTGASignature(const std::string &_uri);
            bool _CheckBMPSignature(const std::vector<char> &_resource);
            bool _CheckPPMSignature(const std::vector<char> &_resource);


        public:
            /**
             * Find an appropriate image buffer type from given resource
             * @param _resource specifies a resource array
             * @param _uri is a string that specifies the uri used for retrieving the data
             */
            void FindImageBufferTypeFromResource(const std::vector<char> &_resource, const std::string &_uri);
            /**
             * Resolve image type from given GLTF bufferView
             * @param _mime_type specifies image mime type
             */
            void ResolveFromBufferView(const std::string &_mime_type);
            /**
             * Retrieve the resolved image type
             * @return BufferType instance specifying the image buffer type
             */
            inline BufferType GetResolvedType() {
                return m_resolved_type;
            }
    };
}

#endif
