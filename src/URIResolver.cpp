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

        // check for failbit and throw an error if needed
        if(m_stream.fail()) {
            std::cerr << "Cannot resolve uri '" << m_uri  << "' in root path '" << m_root_path << "'" << std::endl;
            if(m_unresolved_severity == UNRESOLVED_SEVERITY_ERROR) 
                std::exit(LIBDAS_ERROR_INVALID_FILE);
        }

        // check the file size
        m_stream.seekg(0, std::ios_base::end);
        size_t file_size = static_cast<size_t>(m_stream.tellg());
        m_stream.seekg(0, std::ios_base::beg);

        // allocate memory for buffers
        m_buffer.resize(file_size);

        // read data and close the stream
        m_stream.read(m_buffer.data(), file_size);
        m_stream.close();
    }


    void URIResolver::Resolve(const std::string &_uri, const std::string &_root_path) {
        // set variables if needed
        if(_uri != "") m_uri = _uri;
        if(_root_path != "") m_root_path = _root_path;

        const std::string base64_data_uris[2] = { "data:application/octet-stream;base64,", "data:application/gltf-buffer;base64," };

        // check if base64 encoding is enabled
        if(m_uri.find(base64_data_uris[0]) == 0)
            m_buffer = Base64Decoder::Decode(m_uri.substr(base64_data_uris[0].size()));
        else if(m_uri.find(base64_data_uris[1]) == 0)
            m_buffer = Base64Decoder::Decode(m_uri.substr(base64_data_uris[1].size()));

        // assume file stream otherwise
        else _ResolveFileURI();
    }
}
