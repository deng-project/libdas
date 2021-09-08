/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: huf.h - Huffman coding algorithm header
/// author: Karl-Mihkel Ott


#ifndef __HUF_H
#define __HUF_H

#ifdef __HUF_CPP
    #include <cstdint>
    #include <climits>
    #include <cstdio>
    #include <cstdlib>
    #include <cstring>
    #include <cassert>
    #include <iostream>
    #include <queue>
    #include <vector>

    #define ASCII_LEN   256
#endif


/// Structure for each as ascii node 
struct Node {
    unsigned char ch; 
    Node *left;
    Node *right;
    int freq;

    Node() : ch(0), left(nullptr), right(nullptr), freq(0) {}
    Node(unsigned char c, Node *l, Node *r, int f) : 
        ch(c), left(l), right(r), freq(f) {}

    bool isLeaf() {
        return left == nullptr && right == nullptr;
    }
};


/// Binary key structure
struct binkey {
    unsigned char val = 0;
    int shift = 0;
};


// comparison class for std::priority_queue
class less { 
    public: 
        bool operator()(Node *p1, Node *p2) {
            return p1->freq > p2->freq;
        }
};


Node *mkTree(std::priority_queue<Node*, std::vector<Node*>, less> &nqueue);
void buildHuffmanTables(Node *root, unsigned char bin, int shift, char *detable, binkey *entable);
char *encodeToStdout(FILE *f, size_t flen, int *cbyte, binkey *entable);
void decodeToStdout(char *bytes, int bc, size_t ex_len, Node *root, char *detable);

#endif
