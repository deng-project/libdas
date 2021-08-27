/// DENG: dynamic engine - small but powerful 3D game engine
/// licence: Apache, see LICENCE file
/// file: tex_loader.h - das image loader header file
/// author: Karl-Mihkel Ott


#ifndef __TEX_LOADER_H
#define __TEX_LOADER_H

#ifdef __cplusplus
extern "C" {
#endif

#define DEFAULT_ASSET_COLOR (das_ObjColorData) {0.7f, 0.7f, 0.7f, 1.0f}


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
    #include <string.h>

    #include <common/base_types.h>
    #include <common/common.h>
    #include <common/hashmap.h>
    #include <common/uuid.h>
    #include <common/cerr_def.h>

    #include <data/image_headers.h>
    #include <data/assets.h>
	#define __TEXTURE_LOG_FILE  "textures.log"
    
    das_ImageFormat __das_DetectImageFormat(const char *file_name);

    /// Read raw bitmap data from file stream
    /// This function expects the file to be uncompressed
    void __das_ReadBitmap(FILE *file, char *file_name, deng_ui8_t bit_c, deng_bool_t vert_re, das_Texture* tex);


    /// Load JPEG image into das_Texture instance
    void __das_LoadJPGImage(das_Texture *tex, const char *file_name);


    /// Load BMP image data into das_Texture
    void __das_LoadBMPImage(das_Texture *tex, const char *file_name);
    

    /// Load TGA image into das_Texture instance
    void __das_LoadTGAImage(das_Texture *tex, const char *file_name);


	#ifdef __DEBUG
		/// Write a log file for texture instance in following format:
		/// <WIDTH>x<HEIGHT>\n
		/// <B><G><R><A>\n 
		/// ...
		void _db_RawTexture(das_Texture *tex, const char *file_name);
	#endif
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
