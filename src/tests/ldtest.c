/// DAM: DENG asset manager
/// licence: Apache, see LICENCE file
/// file: ldtest.c - test loading functionality of libdas
/// author: Karl-Mihkel Ott

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <uuid.h>
#include <assets.h>
#include <das_loader.h>

int main(int argv, char *argc[]) {
    if(argv < 2) {
        printf("Please provide a file to use in loading process\n");
        exit(0);
    }

    das_Asset asset = { 0 };
    das_ColorData color = {1.0f, 0.0f, 0.0f, 1.0f};
    das_LoadAsset(&asset, DAS_ASSET_MODE_UNDEFINED, color, false, NULL, argc[1]);

    printf("Loaded asset from source %s\n", asset.src);
    printf("Position vertices count: %lu\n", asset.vertices.v3d.mul.pn);
    printf("Texture vertices count: %lu\n", asset.vertices.v3d.mul.tn);
    printf("Vertex normals count: %lu\n", asset.vertices.v3d.mul.nn);

    return 0;
}
