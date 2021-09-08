/// DENG: dynamic engine - small but powerful 3D game engine
/// licence: Apache, see LICENCE file
/// file: tex_loader.h - das image loader header file
/// author: Karl-Mihkel Ott


#ifndef __TEX_LOADER_H
#define __TEX_LOADER_H



#ifdef __TEX_LOADER_CPP
    #include <string>
    #include <cstring>

    #include <uuid.h>
    #include <assets.h>

    // Use stb image to load pixel data
    #define STB_IMAGE_IMPLEMENTATION
    #include <stb_image.h>

    #define MAX_WIDTH       UINT16_MAX
    #define MAX_HEIGHT      UINT16_MAX


    //das_ImageFormat findImageFormat(const char *file_name);
    //void verifyImageDimentions(int x, int y);
#endif


namespace libdas {

    enum ImageFormat {
        DAS_IMAGE_FORMAT_BMP,
        DAS_IMAGE_FORMAT_TGA,
        DAS_IMAGE_FORMAT_JPEG,
        DAS_IMAGE_FORMAT_PNG,
        DAS_IMAGE_FORMAT_GIF,
        DAS_IMAGE_FORMAT_HDR,
        DAS_IMAGE_FORMAT_PNM,
        DAS_IMAGE_FORMAT_PPM,
        DAS_IMAGE_FORMAT_PGM,
        DAS_IMAGE_FORMAT_PBM,
        DAS_IMAGE_FORMAT_PSD,
        DAS_IMAGE_FORMAT_UNKNOWN
    };


    struct TextureInfo {
        ImageFormat format;   
        uint16_t width;
        uint16_t height;
        uint64_t fsize;
        uint16_t comp_c;
    };


    ///// Load texture bitmap data into das_Texture instance
    //void das_LoadTexture(das_Texture *tex, const char *file_name);

    ///// Get information about the current texture
    //void das_GetTextureInfo(das_TextureInfo *inf, const char *file_name);


    class TextureLoader {
        private:
            bool m_no_cleanup;
            Texture m_texture;
            ImageFormat m_img_format = DAS_IMAGE_FORMAT_UNKNOWN;

        private:
            ImageFormat findImageFormat(const std::string &file_name);
            void verifyImageDimentions(int x, int y);

        public:
            TextureLoader();
            TextureLoader(bool no_cleanup);
            TextureLoader(const std::string &file_name);
            TextureLoader(const std::string &file_name, bool no_cleanup);
            ~TextureLoader();

            void loadTexture(const std::string &file_name);
            void cleanTexture();

            Texture &getTexture();
            TextureInfo getTextureInfo();
    };

}
#endif
