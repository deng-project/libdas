#include <queue>
#include <string>
#include <iostream>
#include <cstdint>
#include <fstream>
typedef std::size_t size_t;

#include <HuffmanCompression.h>

int main(int argc, char *argv[]) {
    if(argc < 3) {
        std::cerr << "Usage: HuffmanCompressionTest <file> d|e" << std::endl;
        std::exit(-1);
    }

    std::string file_name = argv[1];
    std::string out_file = "";

    // decompression
    if(std::string(argv[2]) == "d") out_file = file_name + ".raw";
    else out_file = file_name + ".huf";

    std::ifstream in(file_name, std::ios_base::binary);
    std::ofstream out(out_file, std::ios_base::binary);

    if(std::string(argv[2]) == "d")
        Libdas::Huffman::Decoder(in, out);
    else Libdas::Huffman::Encoder(in, out);

    return 0;
}
