#include <iostream>
#include <fstream>
#include <cstring>
#include <queue>

#include <huf.h>
#include <msg_assert.h>

int main(int argc, char *argv[]) {
    ///// test the multibyte bit shifting
    //unsigned char bytes[MAX_TREE_DEPTH] = {};
    //bytes[MAX_TREE_DEPTH - 1] = 5;
    //bytes[MAX_TREE_DEPTH - 2] = 1;

    //libdas::huf_data::shiftToMSB(bytes, MAX_TREE_DEPTH, 6);
    //std::cout << static_cast<int>(bytes[MAX_TREE_DEPTH - 1]) << std::endl;
    //std::cout << static_cast<int>(bytes[MAX_TREE_DEPTH - 2]) << std::endl;

    if(argc < 2) {
        std::cout << "Please enter the first argument as in file, second argument as encoded file name and the third file as decoded dump file name" << std::endl;
        std::exit(1);
    }

    std::ifstream ifile(argv[1], std::ios::binary);
    msgassert(ifile.is_open(), "Could not open file " + std::string(argv[1]) + " for reading");

    std::ofstream ofile(argv[2], std::ios::binary);
    msgassert(ofile.is_open(), "Could not open file " + std::string(argv[2]) + " for writing");

    libdas::huf_encoder encoder = libdas::huf_encoder(ifile, ofile);

    ifile.close();
    ofile.close();

    ifile.open(argv[2], std::ios::binary);
    msgassert(ifile.is_open(), "Could not open file " + std::string(argv[2]) + " for reading");
    ofile.open(argv[3], std::ios::binary);
    msgassert(ifile.is_open(), "Could not open file " + std::string(argv[3]) + " for writing");
    
    libdas::huf_decoder decoder = libdas::huf_decoder(ifile, ofile);

    ifile.close();
    ofile.close();
    

    std::cout << "Done!" << std::endl;
    return 0;
}
