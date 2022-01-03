#include <fstream>
#include <cstring>
#include <string>
#include <iostream>
#include <vector>

#include <Vector.h>
#include <Matrix.h>
#include <Quaternion.h>
#include <ErrorHandlers.h>
#include <DasStructures.h>
#include <TextureReader.h>

int main(int argc, char *argv[]) {
    if(argc < 2) {
        std::cerr << "Please provide a picture file as an argument" << std::endl;
        std::exit(LIBDAS_ERROR_INVALID_FILE);
    }

    std::string file_name = argv[1];
    // extract the file name to relative position
    for(int i = strlen(argv[1]) - 1; i >= 0; i--) {
        if(argv[1][i] == '/') {
            file_name = std::string(argv[1]).substr(i);
            break;
        }
    }


    Libdas::TextureReader rd = Libdas::TextureReader(argv[1]);
    std::ofstream cpy(file_name + ".cpy", std::ios_base::binary);
    cpy.write(rd.GetBuffer(), rd.GetBufferSize());
    cpy.close();

    std::ofstream raw(file_name + ".raw", std::ios_base::binary);

    int x, y;
    raw.write(rd.GetRawBuffer(&x, &y), rd.GetRawBufferSize());
    raw.close();

    BufferType type = rd.GetImageBufferType();
    switch(type) {
        case LIBDAS_BUFFER_TYPE_TEXTURE_JPEG:
            std::cout << "Image type JPEG" << std::endl;
            break;

        case LIBDAS_BUFFER_TYPE_TEXTURE_PNG:
            std::cout << "Image type PNG" << std::endl;
            break;

        case LIBDAS_BUFFER_TYPE_TEXTURE_TGA:
            std::cout << "Image type TGA" << std::endl;
            break;

        case LIBDAS_BUFFER_TYPE_TEXTURE_BMP:
            std::cout << "Image type BMP" << std::endl;
            break;

        case LIBDAS_BUFFER_TYPE_TEXTURE_PPM:
            std::cout << "Image type PPM" << std::endl;
            break;

        case LIBDAS_BUFFER_TYPE_TEXTURE_RAW:
            std::cout << "Image type raw" << std::endl;
            break;
    }

    std::cout << "Image size " << x << "x" << y << std::endl;

    return 0;
}
