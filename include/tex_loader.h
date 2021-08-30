/// DENG: dynamic engine - small but powerful 3D game engine
/// licence: Apache, see LICENCE file
/// file: tex_loader.h - das image loader header file
/// author: Karl-Mihkel Ott


#ifndef __TEX_LOADER_H
#define __TEX_LOADER_H

#ifdef __cplusplus
extern "C" {
#endif

#define DEFAULT_ASSET_COLOR (das_ColorData) {0.7f, 0.7f, 0.7f, 1.0f}


/// This enum specifies the image file format 
typedef enum das_ImageFormat {
    DAS_IMAGE_FORMAT_BMP = 0,
    DAS_IMAGE_FORMAT_TGA = 1,
    DAS_IMAGE_FORMAT_PNG = 2,
    DAS_IMAGE_FORMAT_JPG = 3,
    DAS_IMAGE_FORMAT_UNKNOWN = 4
} das_ImageFormat;


#ifdef __TEX_LOADER_C
    #include <stdlib.h>
    #include <stdio.h>
    #include <stdint.h>
    #include <stdbool.h>
    #include <string.h>

    #include <uuid.h>
    #include <hashmap.h>
    #include <assets.h>

    
    das_ImageFormat detectImageFormat(const char *file_name);
    void readBitmap(FILE *file, char *file_name, uint8_t bit_c, bool vert_re, das_Texture* tex);
    void loadJPGImage(das_Texture *tex, const char *file_name);
    void loadBMPImage(das_Texture *tex, const char *file_name);
    void loadTGAImage(das_Texture *tex, const char *file_name);


	#ifdef __DEBUG
		/// Write a log file for texture instance in following format:
		/// <WIDTH>x<HEIGHT>\n
		/// <B><G><R><A>\n 
		/// ...
		void _db_RawTexture(das_Texture *tex, const char *file_name);
	#endif


    /*******************************************/
    /********** BMP Image Info Headers *********/
    /*******************************************/
    #define __BMPFileHeader_Size 14
    typedef struct __BMPFileHeader {
        uint16_t file_type;
        uint32_t file_size;
        uint16_t reserved1;
        uint16_t reserved2;
        uint32_t offset_data;
    } __BMPFileHeader;


    #define __BMPInfoHeader_Size    40
    typedef struct __BMPInfoHeader {
        uint32_t size;
        int32_t width;
        int32_t height;

        uint16_t planes;
        uint16_t bit_count;
        uint32_t compression;
        uint32_t size_image;
        int32_t x_pixels_per_meter;
        int32_t y_pixels_per_meter; 
        uint32_t colors_used;
        uint32_t colors_important;
    } __BMPInfoHeader;


    #define __BMPColorHeader_Size   24
    typedef struct __BMPColorHeader {
        uint32_t red_mask;
        uint32_t green_mask;
        uint32_t blue_mask;
        uint32_t alpha_mask;
        uint32_t color_space_type;
        uint32_t unused[16];
    } __BMPColorHeader;


    /*******************************************/
    /********** BMP Image Info Headers *********/
    /*******************************************/
    #define __TGATypeHeader_Size    3
    typedef struct __TGATypeHeader {
        uint8_t id_length;
        uint8_t colormap_type;
        uint8_t image_type;
    } __TGATypeHeader;


    #define __TGAColorMapHeader_Size    5
    typedef struct __TGAColorMapHeader {
        uint16_t first_colormap_index;
        uint16_t colormap_length;
        uint8_t entity_size;
    } __TGAColorMapHeader;


    #define __TGAInfoHeader_Size    10
    typedef struct __TGAInfoHeader {
        uint16_t x_origin;
        uint16_t y_origin;
        uint16_t width;
        uint16_t height;
        uint8_t bit_count;
        uint8_t image_descriptor;
    } __TGAInfoHeader;
    #endif


#ifdef __DEBUG
	void _das_RewriteTextureLogs();
	#define das_RewriteTextureLogs()   _das_RewriteTextureLogs()
#else 
	#define das_RewriteTextureLogs()
#endif

/// Pixel alignment specifying enumeral
typedef enum das_PixelFormat {
    DAS_PIXEL_FORMAT_B8G8R8A8   = 0x01,
    DAS_PIXEL_FORMAT_A8B8G8R8   = 0x02,
    DAS_PIXEL_FORMAT_R8G8B8A8   = 0x04,
    DAS_PIXEL_FORMAT_A8R8G8B8   = 0x08,
    DAS_PIXEL_FORMAT_R8G8B8     = 0x10,
    DAS_PIXEL_FORMAT_B8G8R8     = 0x12
} das_PixelFormat;


/// Realign raw pixel data to B8G8R8A8 format in order to make it compatible with 
/// specified texture format
void das_RealignPixelData(das_Texture *tex, das_PixelFormat format);


/// Load texture bitmap data into das_Texture instance
void das_LoadTexture (
    das_Texture *tex, 
    const char *file_name
);


#ifdef __cplusplus
}
#endif

#endif
