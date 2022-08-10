// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: BufferImageTypeResolver.h - Resolve buffer image type from specified URI class implementation
// author: Karl-Mihkel Ott

#define BUFFER_IMAGE_TYPE_RESOLVER_CPP
#include "das/BufferImageTypeResolver.h"


namespace Libdas {

    bool BufferImageTypeResolver::_CheckJPEGSignature(const std::vector<char> &_resource) {
        const uint8_t jfif_sig[] = {0xff, 0xd8, 0xff, 0xe0, 0x00, 0x10, 0x4a, 0x46, 0x49, 0x46, 0x00, 0x01};
        const uint8_t raw_sig[] = {0xff, 0xd8, 0xff, 0xee};

        if(_resource.size() > 4 && !memcmp(raw_sig, _resource.data(), 4))
            return true;
        else if(_resource.size() > 12 && !memcmp(jfif_sig, _resource.data(), 12))
            return true;

        return false;
    }


    bool BufferImageTypeResolver::_CheckPNGSignature(const std::vector<char> &_resource) {
        const uint8_t png_sig[] = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};

        if(_resource.size() > 8 && !memcmp(png_sig, _resource.data(), 8))
            return true;

        return false;
    }


    bool BufferImageTypeResolver::_CheckTGASignature(const std::string &_uri) {
        std::string ext = Algorithm::ExtractFileName(_uri);
        // convert extension to lower case
        for(size_t i = 0; i < ext.size(); i++) {
            if(ext[i] >= 'A' && ext[i] <= 'Z')
                ext[i] -= 'a' - 'A';
        }

        if(ext == "tga" || ext == "icb" || ext == "vda" || ext == "vst")
            return true;

        return false;
    }


    bool BufferImageTypeResolver::_CheckBMPSignature(const std::vector<char> &_resource) {
        const uint8_t bmp_sig[] = {0x42, 0x4d};
        if(_resource.size() > 2 && !memcmp(bmp_sig, _resource.data(), 2))
            return true;

        return false;
    }


    bool BufferImageTypeResolver::_CheckPPMSignature(const std::vector<char> &_resource) {
        const uint8_t ppm_sig[] = {0x50, 0x33, 0x0a};
        if(_resource.size() > 3 && !memcmp(ppm_sig, _resource.data(), 3))
            return true;

        return false;
    }


    void BufferImageTypeResolver::FindImageBufferTypeFromResource(const std::vector<char> &_resource, const std::string &_uri) {
        if(_CheckJPEGSignature(_resource)) 
            m_resolved_type |= LIBDAS_BUFFER_TYPE_TEXTURE_JPEG;
        else if(_CheckPNGSignature(_resource)) 
            m_resolved_type |= LIBDAS_BUFFER_TYPE_TEXTURE_PNG;
        else if(_CheckTGASignature(_uri)) 
            m_resolved_type |= LIBDAS_BUFFER_TYPE_TEXTURE_TGA;
        else if(_CheckBMPSignature(_resource))
            m_resolved_type |= LIBDAS_BUFFER_TYPE_TEXTURE_BMP;
        else if(_CheckPPMSignature(_resource))
            m_resolved_type |= LIBDAS_BUFFER_TYPE_TEXTURE_PPM;
    }


    void BufferImageTypeResolver::ResolveFromBufferView(const std::string &_mime_type) {
        std::unordered_map<std::string, BufferType> mime_type_map = {
            std::make_pair("image/jpeg", LIBDAS_BUFFER_TYPE_TEXTURE_JPEG),
            std::make_pair("image/png", LIBDAS_BUFFER_TYPE_TEXTURE_PNG),
            std::make_pair("image/x-targa", LIBDAS_BUFFER_TYPE_TEXTURE_TGA),
            std::make_pair("image/x-tga", LIBDAS_BUFFER_TYPE_TEXTURE_TGA),
            std::make_pair("image/bmp", LIBDAS_BUFFER_TYPE_TEXTURE_BMP),
            std::make_pair("image/x-bmp", LIBDAS_BUFFER_TYPE_TEXTURE_BMP),
            std::make_pair("image/x-portable-pixmap", LIBDAS_BUFFER_TYPE_TEXTURE_PPM)
        };

        if(mime_type_map.find(_mime_type) == mime_type_map.end())
            return;
        else m_resolved_type |= mime_type_map[_mime_type];
    }
}
