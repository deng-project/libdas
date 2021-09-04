#include <cstdint>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <queue>
#include <vector>

#define ASCII_LEN   256


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


struct binkey {
    char val = 0;
    int shift = 0;
};


// comparison class for std::priority_queue
class less { 
    public: 
        bool operator()(Node *p1, Node *p2) {
            return p1->freq > p2->freq;
        }
};


void print_bits(char ch) {
    for(int i = 7; i >= 0; i--) {
        char m = ch >> i;
        printf("%d", m & 1);
    }

    printf("\n");
}


Node *mk_tree(std::priority_queue<Node*, std::vector<Node*>, less> &nqueue) {
    while(nqueue.size() > 1) {
        // pop two smallest frequency nodes
        Node *l = nqueue.top();
        nqueue.pop();
        Node *r = nqueue.top();
        nqueue.pop();
        nqueue.push(new Node(0, l, r, l->freq + r->freq));
    }

    return nqueue.top();
}


void build_huffman_tables(Node *root, char bin, char *detable, binkey *entable) {
    if(root->isLeaf()) {
        binkey key;
        key.val = bin;
        for(int i = 7; i >= 0; i--) {
            key.shift = 7 - i;
            if((bin >> i & 1)) break;
        }

        printf("char %c, offset %d\n", root->ch, key.shift);
        print_bits(key.val);
        entable[(int) root->ch] = key;
        detable[(int) key.val] = root->ch;
        return;
    }

    build_huffman_tables(root->left, bin << 1, detable, entable);
    build_huffman_tables(root->right, bin << 1 | 1, detable, entable);
}


char *encode_to_stdout(const std::string &str, int *cbyte, binkey *entable) {
    char *bytes = (char*) malloc(str.size());
    memset(bytes, 0, str.size());
    (*cbyte) = 0;
    int rem_bits = 8;

    for(size_t i = 0; i < str.size(); i++) {
        binkey key = entable[(int) str[i]];

        // attempt to mask key bits to current byte assuming that the byte can store all bits
        if(rem_bits >= 8 - key.shift) {
            bytes[*cbyte] |= (key.val << key.shift) >> (8 - rem_bits);
            rem_bits -= (8 - key.shift);
            
            if(!rem_bits) {
                (*cbyte)++;
                rem_bits = 8;
            }
        } 

        // if the byte cannot store all bits needed, attempt to augment encoded byte into two bytes 
        else {
            // NOTE: potentially buggy and causes wrong encoding
            char aug_bits[2] = {0}; 
            int aug_shift = key.shift + 8 - rem_bits;
            aug_bits[0] = key.val >> (8 - rem_bits);
            aug_bits[1] = key.val << aug_shift;
            rem_bits = 8 - aug_shift;
            
            bytes[*cbyte] |= aug_bits[0];
            (*cbyte)++;
            bytes[*cbyte] |= aug_bits[1];
        }
    }

    // Write all bytes to stdout
    fwrite(bytes, (*cbyte) + 1, 1, stdout);
    std::cout << std::endl;
    return bytes;
}


void decode_to_stdout(char *bytes, int bc, Node *root, char *detable) {
    std::string str = "";

    int abit = 0, rbit = 0;
    
    Node *x = root;
    while(abit < bc * 8) {
        if(bytes[abit / 8] >> (7 - rbit) & 1) 
            x = x->right; 
        else x = x->left;

        if(x->isLeaf()) {
            printf("%c\n", x->ch);
            str += x->ch;
            x = root;
        }

        abit++;
        rbit = abit % 8;
    }

    std::cout << str << std::endl;
}


int main() {
    std::string etext;
    std::getline(std::cin, etext);

    // pairs with ascii characters and their frequencies
    int freq[256] = { 0 };
    std::priority_queue<Node*, std::vector<Node*>, less> nqueue;

    // find all frequencies of characters
    for(size_t i = 0; i < etext.size(); i++)
        freq[(int) etext[i]]++;

    for(int i = 0; i < 256; i++) {
        if(freq[i] > 0)
            nqueue.push(new Node(i, nullptr, nullptr, freq[i]));
    }

    Node *root = mk_tree(nqueue);

    binkey entable[256] = {};
    char detable[256] = {};
    build_huffman_tables(root, 0, detable, entable);

    int cbyte = 0;
    char *bytes = encode_to_stdout(etext, &cbyte, entable);
    decode_to_stdout(bytes, cbyte + 1, root, detable);
    return 0;
}
