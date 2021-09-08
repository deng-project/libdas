/// DENG: dynamic engine - small but powerful 3D game engine
/// licence: Apache, see LICENCE file
/// file: tex_loader.cpp - das image loader wrapper source file
/// author: Karl-Mihkel Ott

#define __TEX_LOADER_CPP
#include <tex_loader.h>


namespace libdas {

    TextureLoader::TextureLoader() : m_no_cleanup(CLEANUP), m_texture{}, m_img_format(DAS_IMAGE_FORMAT_UNKNOWN) {}


    TextureLoader::TextureLoader(bool no_cleanup) : m_no_cleanup(no_cleanup), m_texture{}, m_img_format(DAS_IMAGE_FORMAT_UNKNOWN) {}


    TextureLoader::TextureLoader(const std::string &file_name) : m_no_cleanup(CLEANUP), m_texture{}, m_img_format(DAS_IMAGE_FORMAT_UNKNOWN) {
        loadTexture(file_name);
    }


    TextureLoader::TextureLoader(const std::string &file_name, bool no_cleanup) : m_no_cleanup(no_cleanup), m_texture{}, m_img_format(DAS_IMAGE_FORMAT_UNKNOWN) {
        loadTexture(file_name);
    }


    TextureLoader::~TextureLoader() {
        if(!m_no_cleanup)
            cleanTexture();
    }


    /// Find the file format of the current image
    ImageFormat TextureLoader::findImageFormat(const std::string &file_name) {
        // Start iterating file_name chars backwards
        const size_t fnlen = file_name.size();
        const char *ext = NULL;
        for(int32_t i = (int32_t) fnlen - 2; i >= 0; i--) {
            if(file_name[i] == '.') {
                ext = file_name.c_str() + i;
                break;
            }
        }

        if(!ext) return DAS_IMAGE_FORMAT_UNKNOWN;
        else if(!strcasecmp(ext, "bmp")) return DAS_IMAGE_FORMAT_BMP;
        else if(!strcasecmp(ext, "tga")) return DAS_IMAGE_FORMAT_TGA;
        else if(!strcasecmp(ext, "jpg") || !strcasecmp(ext, "jpeg"))
            return DAS_IMAGE_FORMAT_JPEG;
        else if(!strcasecmp(ext, "png")) return DAS_IMAGE_FORMAT_PNG;
        else if(!strcasecmp(ext, "gif")) return DAS_IMAGE_FORMAT_GIF;
        else if(!strcasecmp(ext, "hdr")) return DAS_IMAGE_FORMAT_HDR;
        else if(!strcasecmp(ext, "pnm")) return DAS_IMAGE_FORMAT_PNM;
        else if(!strcasecmp(ext, "ppm")) return DAS_IMAGE_FORMAT_PPM;
        else if(!strcasecmp(ext, "pgm")) return DAS_IMAGE_FORMAT_PGM;
        else if(!strcasecmp(ext, "pbm")) return DAS_IMAGE_FORMAT_PBM;
        else return DAS_IMAGE_FORMAT_UNKNOWN;
    }


    /// Verify that image dimentions are correct and do not exceed MAX_WIDTH / MAX_HEIGHT 
    void TextureLoader::verifyImageDimentions(int x, int y) {
        if(x > (int) MAX_WIDTH) {
            fprintf(stderr, "Image width of %d is greater than maximum allowed width %d\n", x, MAX_WIDTH);
            exit(1);
        } else if(y > (int) MAX_HEIGHT) {
            fprintf(stderr, "Image height of %d is greater than maximum allowed height %d\n", y, MAX_HEIGHT);
            exit(1);
        }
    }


    /// Load texture bitmap data into das_Texture instance
    void TextureLoader::loadTexture(const std::string &file_name) {
        strcpy(m_texture.src, file_name.c_str());
        id_generator idg = id_generator();
        m_texture.uuid = idg.getUUID();

        int x, y, n;
        m_texture.pixel_data.pixel_data = stbi_load(file_name.c_str(), &x, &y, &n, 4);

        verifyImageDimentions(x, y);
        m_texture.pixel_data.width = static_cast<uint16_t>(x);
        m_texture.pixel_data.height = static_cast<uint16_t>(x);
        m_texture.pixel_data.size = m_texture.pixel_data.width * m_texture.pixel_data.height * 4;
    }


    /// Clean all texture data in current texture
    void TextureLoader::cleanTexture() {
        free(m_texture.pixel_data.pixel_data);
    }
}
