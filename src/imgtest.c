/// libdas: DENG asset loader and assembler library
/// licence: Apache, see LICENCE file
/// file: imgtest.c - image loader functionality test
/// author: Karl-Mihkel Ott

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <uuid.h>
#include <assets.h>
#include <tex_loader.h>
#define BUF_LEN     512

int main(int argv, char *argc[]) {
    if(argv < 2) {
        printf("Please specify an image file to use\n");
        exit(0);
    }

    das_Texture tex = { 0 };
    das_LoadTexture(&tex, argc[1]);

    // Log image data into a log file
    FILE *file = fopen("img.log", "wb");
    if(!file) exit(1);

    size_t px_no = 0;
    printf("image size: %lu\n", tex.pixel_data.size);
    printf("image width: %u\n", tex.pixel_data.width);
    printf("image height: %u\n", tex.pixel_data.height);
    
    for(size_t i = 0; i < tex.pixel_data.size  / 4; i += 4) {
        printf("px%lu: %u %u %u %u\n", px_no, tex.pixel_data.pixel_data[i * 4], tex.pixel_data.pixel_data[i * 4 + 1],
                tex.pixel_data.pixel_data[i * 4 + 2], tex.pixel_data.pixel_data[i * 4 + 3]);
        px_no++;
    }

    free(tex.pixel_data.pixel_data);
    return 0;
}
