/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: WavefrontObjStreamReader.cpp - AsciiStreamReader class test with Wavefront Obj files
/// test purpose: Read Wavefront Obj file and output all chunks to output file
/// author: Karl-Mihkel Ott


/**** PASSED ****/

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <AsciiStreamReader.h>

#define CHUNK   4096

int main(int argc, char *argv[]) {
    if(argc != 2) {
        std::cerr << "Please enter a file name as commandline argument" << std::endl;
        std::exit(-1);
    }


    std::string in_file = std::string(argv[1]);
    Libdas::AsciiStreamReader sr(CHUNK, "\n", std::string(argv[1]));
    std::string out_file = in_file.substr(0, in_file.find('.')) + ".out";
    std::ofstream out_stream = std::ofstream(out_file, std::ios::binary);

    int i = 1;
    do {
        size_t last_read = sr.GetLastRead();
        std::string chunk_msg = "\n//***** Chunk " + std::to_string(i) + " *****//\n";
        out_stream.write(chunk_msg.c_str(), chunk_msg.size() * sizeof(char));
        out_stream.write(sr.GetBufferAddress(), last_read);

        i++;
    } while(sr.ReadNewChunk());

    return 0;
}
