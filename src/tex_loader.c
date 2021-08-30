/// DENG: dynamic engine - small but powerful 3D game engine
/// licence: Apache, see LICENCE file
/// file: tex_loader.c - das image loader wrapper source file
/// author: Karl-Mihkel Ott

#define __TEX_LOADER_C
#include <tex_loader.h>

/// Load texture bitmap data into das_Texture instance
void das_LoadTexture(das_Texture *texture, const char *file_name) {
    strcpy(texture->src, file_name);
    texture->uuid = uuid_Generate();

    int x, y, n;
    texture->pixel_data.pixel_data = stbi_load(file_name, &x, &y, &n, 4);

    verifyImageDimentions(x, y);
    texture->pixel_data.width = (uint16_t) x;
    texture->pixel_data.height = (uint16_t) x;
    texture->pixel_data.size = texture->pixel_data.width * texture->pixel_data.height * 4;
}


/// Get information about the current texture
void das_GetTextureInfo(das_TextureInfo *inf, const char *file_name) {
    int x, y, n;
    stbi_info(file_name, &x, &y, &n);
    verifyImageDimentions(x, y);

    // Find the file size
    FILE *file = fopen(file_name, "rb");
    if(!file) {
        fprintf(stderr, "Could not open file %s\n", file_name);
        exit(1);
    }
    fseek(file, 0, SEEK_END);
    inf->fsize = (uint64_t) ftell(file);
    fclose(file);

    inf->format = findImageFormat(file_name);
    inf->width = (uint16_t) x;
    inf->height = (uint16_t) y;
    inf->comp_c = (uint16_t) n;
}


/// Find the file format of the current image
das_ImageFormat findImageFormat(const char *file_name) {
    // Start iterating file_name chars backwards
    const size_t fnlen = strlen(file_name);
    const char *ext = NULL;
    for(int32_t i = (int32_t) fnlen - 2; i >= 0; i--) {
        if(file_name[i] == '.') {
            ext = file_name + i;
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
void verifyImageDimentions(int x, int y) {
    if(x > (int) MAX_WIDTH) {
        fprintf(stderr, "Image width of %d is greater than maximum allowed width %d\n", x, MAX_WIDTH);
        exit(1);
    } else if(y > (int) MAX_HEIGHT) {
        fprintf(stderr, "Image height of %d is greater than maximum allowed height %d\n", y, MAX_HEIGHT);
        exit(1);
    }
}
