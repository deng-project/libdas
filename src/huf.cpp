/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: huf.cpp - Huffman coding algorithm implementation
/// author: Karl-Mihkel Ott


#define __HUF_CPP
#include <huf.h>

Node *mkTree(std::priority_queue<Node*, std::vector<Node*>, less> &nqueue) {
    while(nqueue.size() > 1) {
        // pop two smallest frequency nodes
        Node *l = nqueue.top();
        nqueue.pop();
        Node *r = nqueue.top();
        nqueue.pop();

        Node *nn = new Node(0, l, r, l->freq + r->freq);
        nqueue.push(nn);
    }

    return nqueue.top();
}


void buildHuffmanTables(Node *root, unsigned char bin, int shift, char *detable, binkey *entable) {
    if(root->isLeaf()) {
        binkey key;
        key.val = bin;
        key.shift = shift;

        entable[(int) root->ch] = key;
        detable[(int) key.val] = root->ch;
        return;
    }

    buildHuffmanTables(root->left, bin << 1, shift - 1, detable, entable);
    buildHuffmanTables(root->right, bin << 1 | 1, shift - 1, detable, entable);
}


char *encodeToStdout(FILE *f, size_t flen, int *cbyte, binkey *entable) {
    char *bytes = (char*) malloc(flen);
    memset(bytes, 0, flen);
    (*cbyte) = 0;
    int rem_bits = 8;

    char ch;
    while(fread(&ch, 1, 1, f)) {
        binkey key = entable[(int) ch];

        // attempt to mask key bits to current byte assuming that the byte can store all bits
        if(rem_bits >= 8 - key.shift) {
            bytes[*cbyte] |= ((key.val << key.shift) >> (8 - rem_bits));
            rem_bits -= (8 - key.shift);
            
            if(!rem_bits) {
                (*cbyte)++;
                rem_bits = 8;
            }
        } 

        // if the byte cannot store all bits needed, attempt to augment encoded bits into two bytes 
        else {
            // NOTE: potentially buggy and causes wrong encoding
            char aug_bits[2] = {0}; 
            aug_bits[0] = bytes[*cbyte];
            unsigned int aug_shift = key.shift + rem_bits;
            aug_bits[0] |= ((key.val << key.shift) >> (8 - rem_bits)); // !
            aug_bits[1] |= key.val << aug_shift;
            rem_bits = aug_shift;
            
            bytes[*cbyte] = aug_bits[0];
            (*cbyte)++;
            bytes[*cbyte] = aug_bits[1];
        }
    }

    // Write all bytes to stdout
    fwrite(bytes, (*cbyte) + 1, 1, stdout);
    std::cout << std::endl;
    return bytes;
}


void decodeToStdout(char *bytes, int bc, size_t ex_len, Node *root, char *detable) {
    std::string str = "";
    size_t str_len = 0;

    int abit = 0, rbit = 0;
    
    Node *x = root;
    while(abit < bc * 8 && str_len < ex_len) {
        if(bytes[abit / 8] >> (7 - rbit) & 1) 
            x = x->right; 
        else 
            x = x->left;

        if(x->isLeaf()) {
            str += x->ch;
            str_len++;
            x = root;
        }

        abit++;
        rbit = abit % 8;
    }

    std::cout << str << std::endl;
}


int main(int argv, char *argc[]) {
    if(argv < 2) {
        std::cout << "Please enter a file name as an argument" << std::endl;
        std::exit(1);
    }

    FILE *f = fopen(argc[1], "rb");
    assert(f);
    fseek(f, 0, SEEK_END);
    size_t flen = (size_t) ftell(f);
    fseek(f, 0, SEEK_SET);

    // pairs with ascii characters and their frequencies
    int freq[256];
    memset(freq, 255, sizeof(int) * 256);
    std::priority_queue<Node*, std::vector<Node*>, less> nqueue;

    // find all frequencies of characters in stream
    unsigned char ch = 0;
    while(fread(&ch, 1, 1, f))
        freq[(int) ch]++;

    fseek(f, 0, SEEK_SET);

    for(int i = 0; i < 256; i++) {
        if(freq[i] > 0) {
            Node *nn = new Node((unsigned char) i, nullptr, nullptr, freq[i]);
            nqueue.push(nn);
        }
    }

    Node *root = mkTree(nqueue);

    binkey entable[256] = {};
    char detable[256] = {};
    buildHuffmanTables(root, 0, 8, detable, entable);

    int cbyte = 0;
    char *bytes = encodeToStdout(f, flen, &cbyte, entable);
    free(bytes);
    //decodeToStdout(bytes, cbyte + 1, flen, root, detable);
    return 0;
}
