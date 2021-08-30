/// DENG: dynamic engine - small but powerful 3D game engine
/// licence: Apache, see LICENCE file
/// file: tex_loader.c - das image loader source file
/// author: Karl-Mihkel Ott

#define __TEX_LOADER_C
#include <tex_loader.h>

static FILE *__file;

/// Image headers initialisation 
void __das_InitBMPImageHeaders (
    __BMPFileHeader *p_file_header, 
    __BMPInfoHeader *p_info_header, 
    __BMPColorHeader *p_color_header
) {
    *p_file_header = (__BMPFileHeader) { 
        0x4D42, 
        0, 0, 0, 0
    };

    *p_info_header = (__BMPInfoHeader) {
        0, 0, 0, 1, 
        0, 0, 0, 0, 
        0, 0, 0
    };

    *p_color_header = (__BMPColorHeader) {
        0x00ff0000, 
        0x0000ff00, 
        0x000000ff, 
        0xff000000, 
        0x73524742, 
        {0}
    };
}


 /// etect image format and return ImageFormat instance of it
 das_ImageFormat detectImageFormat(const char *file_name) {
    char *ext = cm_ExtractFileExtName((char*) file_name);

    if(!strcmp(ext, "tga")) return DAS_IMAGE_FORMAT_TGA;
    else if(!strcmp(ext, "bmp")) return DAS_IMAGE_FORMAT_BMP;
    else if(!strcmp(ext, "png")) return DAS_IMAGE_FORMAT_PNG;
    else if(!strcmp(ext, "jpg")) return DAS_IMAGE_FORMAT_JPG; 

    free(ext);
    return DAS_IMAGE_FORMAT_UNKNOWN; 
}


/// Realign raw pixel data to B8G8R8A8 format in order to make it compatible with 
/// specified texture format
void das_RealignPixelData(das_Texture *tex, das_PixelFormat pix_format) {
    // Check if the pixel format is correct already
    if(pix_format == DAS_PIXEL_FORMAT_B8G8R8A8) return;
    
    // Variable for storing temporary pixel data
    deng_ui32_t tmp_pix = 0;

    // Save the pointer to old pixel data
    deng_ui8_t *old_pix = tex->pixel_data.pixel_data;

    // Allocate memory for new pixel array
    deng_ui8_t *new_pix = (deng_ui8_t*) calloc(tex->pixel_data.height * tex->pixel_data.width * 4, sizeof(deng_ui8_t));

    // For each pixel in pixel array
    for(size_t i = 0; i < tex->pixel_data.width * tex->pixel_data.height; i++) {
        switch(pix_format) {
        case DAS_PIXEL_FORMAT_B8G8R8:
            new_pix[i * 4] = tex->pixel_data.pixel_data[i * 3];
            new_pix[i * 4 + 1] = tex->pixel_data.pixel_data[i * 3 + 1];
            new_pix[i * 4 + 2] = tex->pixel_data.pixel_data[i * 3 + 2];
            new_pix[i * 4 + 3] = UINT8_MAX;
            break;

        case DAS_PIXEL_FORMAT_R8G8B8:
            new_pix[i * 4] = tex->pixel_data.pixel_data[i * 3 + 2];
            new_pix[i * 4 + 1] = tex->pixel_data.pixel_data[i * 3 + 1];
            new_pix[i * 4 + 2] = tex->pixel_data.pixel_data[i * 3];
            new_pix[i * 4] = UINT8_MAX;
            break;

        case DAS_PIXEL_FORMAT_A8B8G8R8:
            new_pix[i * 4] = tex->pixel_data.pixel_data[i * 4 + 1];
            new_pix[i * 4 + 1] = tex->pixel_data.pixel_data[i * 4 + 2];
            new_pix[i * 4 + 2] = tex->pixel_data.pixel_data[i * 4 + 3];
            new_pix[i * 4 + 3] = tex->pixel_data.pixel_data[i * 4];
            break;

        case DAS_PIXEL_FORMAT_A8R8G8B8:
            new_pix[i * 4] = tex->pixel_data.pixel_data[i * 4 + 3];
            new_pix[i * 4 + 1] = tex->pixel_data.pixel_data[i * 4 + 2];
            new_pix[i * 4 + 2] = tex->pixel_data.pixel_data[i * 4 + 1];
            new_pix[i * 4 + 3] = tex->pixel_data.pixel_data[i * 4];
            break;

        case DAS_PIXEL_FORMAT_R8G8B8A8:
            new_pix[i * 4] = tex->pixel_data.pixel_data[i * 4 + 2];
            new_pix[i * 4 + 1] = tex->pixel_data.pixel_data[i * 4 + 1];
            new_pix[i * 4 + 2] = tex->pixel_data.pixel_data[i * 4];
            new_pix[i * 4 + 3] = tex->pixel_data.pixel_data[i * 4 + 3];
            break;

        default:
            break;
        }
    }

    // Set the new pixel location for the texture
    tex->pixel_data.pixel_data = new_pix;
    
    // Check if old memory area should be freed
    if(!tex->no_reg_cleanup) {
        free(old_pix);
        tex->no_reg_cleanup = !tex->no_reg_cleanup;
    }
}


/// Write a log file for texture instance in following format:
/// <FILE NAME>
/// <WIDTH>x<HEIGHT>\n
/// <B> <G> <R> <A>\n 
/// ...
void _db_RawTexture(das_Texture *tex, const char *file_name) {
    // Buffer for all character data written into a log file
    char buf[512] = { 0 };
    cm_OpenLogger(__TEXTURE_LOG_FILE, false);

    sprintf(buf, "\n\nFILE: %s", file_name);
    cm_LogWrite(buf);
    ZERO_MEM(buf);

    // Write width and height to log file
    sprintf(buf, "%dx%d", tex->pixel_data.width, tex->pixel_data.height);
    cm_LogWrite(buf);

    // For each pixel output its data
    for(deng_ui32_t i = 0; i < tex->pixel_data.width * tex->pixel_data.height / 4; i += 4) {
        ZERO_MEM(buf);
        const deng_ui8_t *pix_data = tex->pixel_data.pixel_data + i * 4;
        sprintf(buf, "%d %d %d %d", pix_data[0], pix_data[1], pix_data[2], pix_data[3]);
        cm_LogWrite(buf);
    }

    cm_CloseLogger();
}


void _das_RewriteTextureLogs() {
    FILE *file = fopen(__TEXTURE_LOG_FILE, "w");
    const char *emsg = "#ENTRY POINT\n";
    fwrite(emsg, strlen(emsg), 1, file);
    DENG_ASSERT(file, "Failed to open texture log stream");
    fclose(file);
}


/// Load texture bitmap data into das_Texture instance
void das_LoadTexture(das_Texture *texture, const char *file_name) {
    strcpy(texture->src, file_name);
    texture->uuid = uuid_Generate();
    das_ImageFormat format = __das_DetectImageFormat(file_name);

    switch (format) {
    case DAS_IMAGE_FORMAT_BMP:
        __das_LoadBMPImage(texture, file_name);
        break;

    case DAS_IMAGE_FORMAT_TGA:
        __das_LoadTGAImage(texture, file_name);
        break;
    
    default:
        break;
    }

	#ifdef __DEBUG
		_db_RawTexture(texture, file_name);
	#endif
}


/// Read raw bitmap data from file
/// This function expects the file to be uncompressed and color depth either 24 bits or 32 bits
void __das_ReadBitmap (
    FILE* file,
    char *file_name,
    deng_ui8_t bit_c,
    deng_bool_t vert_re,
    das_Texture *tex
) {
    deng_ui8_t *tmp_pix;
    size_t offset = 0, offseta;
    deng_i32_t i, k;
    size_t j, res;

    switch (bit_c) {
    case 24:
        offset = 3 * tex->pixel_data.width * sizeof(deng_ui8_t);
        offseta = 4 * tex->pixel_data.width * sizeof(deng_ui8_t);

        tmp_pix = (deng_ui8_t*)calloc(
            tex->pixel_data.height * offset,
            sizeof(deng_ui8_t)
        );

        res = fread (
            tmp_pix,
            offset,
            tex->pixel_data.height,
            file
        );

        if(!res) FILE_ERR(file_name);
        
        // Check if pixel reaarangement should be done (bottom - left to top - left)
        if (vert_re) {
            for (i = (deng_i32_t) tex->pixel_data.height - 1, k = 0; i >= 0; i--, k++) {
                for (j = 0; j < tex->pixel_data.width; j++) {
                    *(tex->pixel_data.pixel_data + k * offseta + j * 4) = *(tmp_pix + i * offset + j * 3);
                    *(tex->pixel_data.pixel_data + k * offseta + j * 4 + 1) = *(tmp_pix + i * offset + j * 3 + 1);
                    *(tex->pixel_data.pixel_data + k * offseta + j * 4 + 2) = *(tmp_pix + i * offset + j * 3 + 2);
                    *(tex->pixel_data.pixel_data + k * offseta + j * 4 + 3) = 0xff;
                }
            }
        }

        else {
            for (i = 0; i < (deng_i32_t)tex->pixel_data.height; i++) {
                for (j = 0; j < tex->pixel_data.width; j++) {
                    *(tex->pixel_data.pixel_data + i * offseta + j * 4) = *(tmp_pix + i * offset + j * 3);
                    *(tex->pixel_data.pixel_data + i * offseta + j * 4 + 1) = *(tmp_pix + i * offset + j * 3 + 1);
                    *(tex->pixel_data.pixel_data + i * offseta + j * 4 + 2) = *(tmp_pix + i * offset + j * 3 + 2);
                    *(tex->pixel_data.pixel_data + i * offseta + j * 4 + 3) = 0xff;
                }
            }
        }

        free(tmp_pix);
        break;

    case 32:
        offset = 4 * tex->pixel_data.width * sizeof(deng_ui8_t);
           
        if (!vert_re) {
            res = fread(tex->pixel_data.pixel_data, sizeof(deng_ui8_t), tex->pixel_data.size, file);
            if(!res) FILE_ERR(file_name);
        }

        else {
            tmp_pix = (deng_ui8_t*) calloc(tex->pixel_data.height, offset);
            res = fread(tmp_pix, offset, tex->pixel_data.height, file);

            if(!res) FILE_ERR(file_name);

            for (i = tex->pixel_data.height; i >= 0; i--)
                memcpy(tex->pixel_data.pixel_data, tmp_pix + i * offset, offset);
        }

        break;

    default:
        printf("Invalid bit count\n");
        break;
    }
}


/* BMP image */
void __das_LoadBMPImage(das_Texture *texture, const char *file_name) {
    size_t res;
    __BMPFileHeader file_header;
    __BMPInfoHeader info_header;
    __BMPColorHeader color_header;
    __das_InitBMPImageHeaders (
        &file_header, 
        &info_header, 
        &color_header
    );
    __file = fopen(file_name, "rb");

    if(!__file) {
        printf("%s%s%s\n", "ERROR: Failed to load texture image file ", file_name, "!");
        return;
    }

    /* __BMP image verifications */
    res = fread (
        &file_header, 
        sizeof(file_header), 
        1, 
        __file
    );

    if(file_header.file_type != 0x4D42) {
        printf("%s%s%s\n", "ERROR: Unrecognised or corrupt texture file ", file_name, "!");
        return;
    }

    res = fread (
        &info_header, 
        sizeof(info_header), 
        1, 
        __file
    );


    // Verify that color masks are correct
    if(info_header.bit_count == 32 && info_header.size >= (sizeof(__BMPInfoHeader) + sizeof(__BMPColorHeader))) {
        res = fread(&color_header, sizeof(color_header), 1, __file);
        deng_bool_t verify_color_data = color_header.alpha_mask == 0xff000000 &&
                                        color_header.blue_mask == 0x000000ff &&
                                        color_header.green_mask == 0x0000ff00 &&
                                        color_header.red_mask == 0x00ff0000 && 
                                        color_header.color_space_type == 0x73524742;
        
        if(!verify_color_data) {
            printf("%s%s%s\n", "ERROR: Unexpected color mask format or color space type in file ", file_name, "!");
            return;
        }
    }

    else if(info_header.bit_count == 32 && !(info_header.size >= (sizeof(__BMPInfoHeader) + sizeof(__BMPColorHeader)))) {
        printf("%s%s%s\n", "ERROR: Texture file ", file_name, " doesn't contain bit mask information!");
        return;
    }

    fseek(__file, (long) file_header.offset_data, SEEK_SET);

    if(info_header.bit_count == 32) {
        info_header.size = sizeof(__BMPInfoHeader) + sizeof(__BMPColorHeader);
        file_header.offset_data = sizeof(__BMPFileHeader) + sizeof(__BMPInfoHeader) + sizeof(__BMPColorHeader);
    }

    else if(info_header.bit_count == 24) {
        info_header.size = sizeof(__BMPInfoHeader);
        file_header.offset_data = sizeof(__BMPFileHeader) + sizeof(__BMPInfoHeader);
    }

    else {
        printf("%s%s%s\n", "ERROR: Unsupported bit count for texture file ", file_name, "!");
        return;
    }

    file_header.file_size = file_header.offset_data;
    texture->pixel_data.width = (deng_ui16_t) info_header.width;
    texture->pixel_data.height = (deng_ui16_t) info_header.height;

    texture->pixel_data.size = info_header.height * info_header.width * 4;
    texture->pixel_data.pixel_data = (deng_ui8_t*) calloc ( 
        texture->pixel_data.size,
        sizeof(deng_ui8_t)
    );

    /* No padding */
    if(info_header.bit_count == 32) {
        size_t w_index, data_index;
        int h_index;
        deng_ui8_t *tmp_arr; 
        
        // Allocate memory for tmp_arr[y][x]
        size_t height_offset = info_header.width * 4;
        tmp_arr = (deng_ui8_t*) calloc (
            info_header.height,
            sizeof(deng_ui8_t) * height_offset
        );
        
        for(h_index = 0; h_index < (int) info_header.height; h_index++) {
            res = fread (
                tmp_arr + h_index * height_offset,
                4 * info_header.width * sizeof(deng_ui8_t), 
                1, 
                __file
           );
        }

        // Realign pixels to top - bottom format
        for(h_index = (int) info_header.height - 1, data_index = 0; h_index >= 0; h_index--) {
            for(w_index = 0; w_index < 4 * info_header.width; w_index++, data_index++)
                texture->pixel_data.pixel_data[data_index] = *(tmp_arr + h_index * height_offset + w_index);
        }
        
        free(tmp_arr);
    }
    /* With padding */
    else {
        size_t w_index, data_index = 0;
        int h_index;

        deng_ui32_t padding = 4 - ((deng_ui32_t) info_header.width * (deng_ui32_t) info_header.bit_count) / 8 % 4;
        deng_ui8_t *tmp_arr;
        
        // Allocate memory for tmp_arr[y][x]
        size_t height_offset = info_header.width * (3 + padding);
        tmp_arr = (deng_ui8_t*)calloc (
            info_header.height,
            sizeof(deng_ui8_t) * height_offset
        );
        
        for(h_index = 0; h_index < (int) info_header.height; h_index++) {
            res = fread (
                tmp_arr + h_index * height_offset, 
                3 * info_header.width * sizeof(deng_ui8_t), 
                1, 
                __file
            );

            // Skip padding bytes
            fseek(__file, padding, SEEK_CUR);
        }

        for(h_index = (int) info_header.height - 1, data_index = 0; h_index >= 0; h_index--) {
            for(w_index = 0; w_index < info_header.width * 3; w_index += 3, data_index += 4) {
                texture->pixel_data.pixel_data[data_index] = *(tmp_arr + h_index * height_offset + w_index);
                texture->pixel_data.pixel_data[data_index + 1] = *(tmp_arr + h_index * height_offset + w_index + 1);
                texture->pixel_data.pixel_data[data_index + 2] = *(tmp_arr + h_index * height_offset + w_index + 2);
                texture->pixel_data.pixel_data[data_index + 3] = 0xFF;
            }
        }
    }

    fclose(__file);

    if(!res) {
        printf("Failed to read from image file: %s\n", file_name);
        exit(-1);
    }
}


/// Load TGA image into das_Texture
void __das_LoadTGAImage(das_Texture *p_asset, const char *file_name) {
    size_t res;
    __TGATypeHeader type_header = {0};
    __TGAColorMapHeader color_header = {0};
    __TGAInfoHeader info_header = {0};
    __file = fopen(file_name, "rb");
    if(!__file) {
        printf("%s%s%s\n", "Failed to load texture image file ", file_name, "!");
        return;
    }

    res = fread(&type_header, __TGATypeHeader_Size, 1, __file);

    // Check the tga image type and exit if image is not supported
    switch (type_header.image_type) {
    case 0:
        printf("%s\n", "ERROR: No texture image data available!");
        return;

    case 1:
        printf("%s\n", "ERROR: Color-mapped tga image is not supported!");
        return;
    
    case 3:
        printf("%s\n", "ERROR: Grayscale tga image is not supported!");
        return;

    case 9:
        printf("%s\n", "ERROR: RLE color-mapped tga image is not supported!");
        return;
    
    case 10:
        printf("%s\n", "ERROR: RLE true-color tga image is not supported!");
        return;  
    
    case 11:
        printf("%s\n", "ERROR: RLE grayscale tga image is not supported!");
        return;
    
    default:
        break;
    }

    res = fread(&color_header, __TGAColorMapHeader_Size, 1, __file);
    res = fread(&info_header, __TGAInfoHeader_Size, 1, __file);
    
    p_asset->pixel_data.width = info_header.width;
    p_asset->pixel_data.height = info_header.height;
    p_asset->pixel_data.size = (size_t) (info_header.height * info_header.width * 4);

    p_asset->pixel_data.pixel_data = (deng_ui8_t*) calloc(p_asset->pixel_data.size, sizeof(deng_ui8_t));
    
    if (info_header.y_origin != p_asset->pixel_data.height)
        __das_ReadBitmap(__file, (char*) file_name, info_header.bit_count, false, p_asset);
    else __das_ReadBitmap( __file, (char*) file_name, info_header.bit_count, true, p_asset);

    p_asset->pixel_data.width = info_header.width;
    p_asset->pixel_data.height = info_header.height;

    fclose(__file);

    if(!res) {
        printf("Failed to read image file: %s\n", file_name);
        exit(-1);
    }
}
