/// DENG: dynamic engine - small but powerful 3D game engine
/// licence: Apache, see LICENCE file
/// file: huf.c - Huffman compression implementation test
/// author: Karl-Mihkel Ott

#include <stdio.h>
#include <stdlib.h>
#include <hashmap.h>

int main(int argv, char *argc[]) {
    if(argv < 3) {
        fprintf(stderr, "Please provide a das binary file name as first argument and destination file as a second argument\n");
        exit(1);
    }

    // Attempt to open files for reading and writing
    FILE *in = fopen(argc[1], "rb");
    FILE *out = fopen(argc[2], "wb");

    Hashmap bmap;

    return 0;
}
