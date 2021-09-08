/// DAM: DENG asset manager
/// licence: Apache, see LICENCE file
/// file: ldtest.c - test loading functionality of libdas
/// author: Karl-Mihkel Ott

#include <iostream>
#include <fstream>

#include <uuid.h>
#include <assets.h>
#include <das_file.h>
#include <das_loader.h>

int main(int argv, char *argc[]) {
    if(argv < 2) {
        printf("Please provide a file to use in loading process\n");
        exit(0);
    }

    libdas::das_loader loader = libdas::das_loader(DAS_ASSET_MODE_UNDEFINED, ColorData{1.0f, 1.0f, 1.0f, 1.0f}, NULL, NULL, argc[1]);
    Asset &asset = loader.getAsset();

    std::cout << "Loaded asset from source " << asset.src << std::endl;
    std::cout << "Position vertices count: " << asset.vertices.v3d.mul.pn << std::endl;
    std::cout << "Texture vertices count: " << asset.vertices.v3d.mul.tn << std::endl;
    std::cout << "Vertex normals count: " << asset.vertices.v3d.mul.nn << std::endl;
    std::cout << "Indices count: " << asset.indices.n << std::endl;

    libdas::das_loader::cleanupAsset(asset);
    return 0;
}
