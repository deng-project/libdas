// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: URIResolver.cpp - URI resolving and its data retrieving class implementation
// author: Karl-Mihkel Ott

#define URI_RESOLVER_CPP
#include <URIResolver.h>


namespace Libdas {

    URIResolver::URIResolver(const std::string &_uri, const std::string &_root_path, UnresolvedUriSeverity _severity) :
        m_uri(_uri), m_root_path(_root_path), m_unresolved_severity(_severity)
    {
        if(m_uri != "") Resolve();
    }


    void URIResolver::_ResolveFileURI() {
        // check if file:// scheme is specified
        if(m_uri.find("file://") != std::string::npos)
            m_stream.open(m_root_path + "/" + m_uri.substr(7), std::ios_base::binary | std::ios_base::in);
        else {
            std::string fpath = m_root_path + "/" + m_uri;
            m_stream.open(fpath, std::ios_base::binary | std::ios_base::in);
        }

        _FindUriBufferTypeFromExtension();

        // check for failbit and throw an error if needed
        if(m_stream.fail()) {
            std::cerr << "Cannot resolve uri '" << m_uri  << "' in root path '" << m_root_path << "'" << std::endl;
            if(m_unresolved_severity == UNRESOLVED_SEVERITY_ERROR) 
                EXIT_ON_ERROR(LIBDAS_ERROR_INVALID_FILE);
        }

        // check the file size
        m_stream.seekg(0, std::ios_base::end);
        size_t file_size = static_cast<size_t>(m_stream.tellg());
        m_stream.seekg(0, std::ios_base::beg);

        // allocate memory for buffer
        m_buffer.resize(file_size);

        // read data and close the stream
        m_stream.read(m_buffer.data(), file_size);
        m_stream.close();
    }


    void URIResolver::_FindUriBufferTypeFromExtension() {
        std::string ext = Algorithm::ExtractFileExtension(m_uri);
        if(ext == "jpeg" || ext == "jpg")
            m_uri_buffer_type |= LIBDAS_BUFFER_TYPE_TEXTURE_JPEG;
        else if(ext == "png")
            m_uri_buffer_type |= LIBDAS_BUFFER_TYPE_TEXTURE_PNG;
        else if(ext == "tga")
            m_uri_buffer_type |= LIBDAS_BUFFER_TYPE_TEXTURE_TGA;
        else if(ext == "ppm")
            m_uri_buffer_type |= LIBDAS_BUFFER_TYPE_TEXTURE_PPM;
    }


    void URIResolver::Resolve(const std::string &_uri, const std::string &_root_path) {
        // set variables if needed
        if(_uri != "") m_uri = _uri;
        if(_root_path != "") m_root_path = _root_path;

        const std::pair<std::string, BufferType> base64_data_uris[] = { 
            { "data:application/octet-stream;base64,", LIBDAS_BUFFER_TYPE_UNKNOWN }, 
            { "data:application/gltf-buffer;base64,", LIBDAS_BUFFER_TYPE_UNKNOWN },
            { "data:image/jpeg;base64,", LIBDAS_BUFFER_TYPE_TEXTURE_JPEG },
            { "data:image/png;base64,", LIBDAS_BUFFER_TYPE_TEXTURE_PNG },
            { "data:image/x-targa;base64,", LIBDAS_BUFFER_TYPE_TEXTURE_TGA },
            { "data:image/x-tga;base64,", LIBDAS_BUFFER_TYPE_TEXTURE_TGA },
            { "data:image/bmp;base64,", LIBDAS_BUFFER_TYPE_TEXTURE_BMP },
            { "data:image/x-portable-pixmap,", LIBDAS_BUFFER_TYPE_TEXTURE_PPM },
            { "data:image/x-portable-bitmap,", LIBDAS_BUFFER_TYPE_TEXTURE_PPM },
        };

        // check if base64 encoding is enabled
        bool found_mime_type = false;
        for(size_t i = 0; i < sizeof(base64_data_uris) / sizeof(std::pair<std::string, BufferType>); i++) {
            if(m_uri.find(base64_data_uris[i].first) == 0) {
                m_buffer = Base64Decoder::Decode(m_uri.substr(base64_data_uris[i].first.size()));
                found_mime_type = true;
                m_uri_buffer_type |= base64_data_uris[i].second;
                break;
            }
        }

        // assume file stream otherwise
        if(!found_mime_type) _ResolveFileURI();
    }
}
