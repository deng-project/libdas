// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: URIResolver.h - URI resolving and its data retrieving class header
// author: Karl-Mihkel Ott

#ifndef URI_RESOLVER_H
#define URI_RESOLVER_H


#ifdef URI_RESOLVER_CPP
    #include <vector>
    #include <string>
    #include <fstream>
    #include <algorithm>
    #include <iostream>
    #include <sstream>
    #include <map>

#ifdef _WIN32
    #include <Windows.h>
#endif

    #include "das/Api.h"
#define LIBDAS_DEFS_ONLY
    #include "das/DasStructures.h"
    #include "das/ErrorHandlers.h"
    #include "das/HuffmanCompression.h"
#undef LIBDAS_DEFS_ONLY

    #include "das/Algorithm.h"
    #include "das/Base64Decoder.h"
#endif



namespace Libdas {
    /// Type to specify unresolved URI severity
    typedef uint8_t UnresolvedUriSeverity;
    #define UNRESOLVED_SEVERITY_WARNING 0
    #define UNRESOLVED_SEVERITY_ERROR   1

    /**
     * Class interface for reading data from given uri.
     * Supported uri types for now are base64 and file
     */
    class LIBDAS_API URIResolver : private Base64Decoder {
        private:
            std::string m_uri;
            std::string m_root_path;

            // buffer storage
            std::vector<char> m_buffer;

            // file stream
            std::ifstream m_stream;
            BufferType m_uri_buffer_type = 0;

            const UnresolvedUriSeverity m_unresolved_severity;

        private:
            /**
             * Resolve file:// URI
             */
            void _ResolveFileURI();
            void _SolvePercentNotation(const std::string &_uri);
            void _FindUriBufferTypeFromExtension();

        public:
            URIResolver(const std::string &_uri = "", const std::string &_root_path = "", UnresolvedUriSeverity _severity = UNRESOLVED_SEVERITY_ERROR);
            URIResolver(URIResolver &&_ur) noexcept;
            /**
             * Resolve the uri and read data into URI data buffer
             * @param _uri optionally specifies the given uri to read data from
             * @param _root_path optionally specifies 
             */
            void Resolve(const std::string &_uri = "", const std::string &_root_path = "");
            inline BufferType GetParsedDataType() {
                return m_uri_buffer_type;
            }
            /**
             * Get a reference of the buffer containing data found on the resouce
             * @return std::pair moveable object containing buffer pointer and its size
             */
            inline std::pair<char*, size_t> GetBuffer() {
                return std::make_pair(m_buffer.data(), m_buffer.size());
            }
    };
}

#endif
