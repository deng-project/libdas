/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: huf.h - Huffman coding algorithm header
/// author: Karl-Mihkel Ott


#ifndef __HUF_H
#define __HUF_H

#ifdef __HUF_CPP
    #include <fstream>
    #include <cstring>
    #include <climits>
    #include <iostream>
    #include <queue>

    #include <msg_assert.h>
#endif

#define ALPHABET_SIZE   256
#define MAX_KEY_BYTES   32  

namespace libdas {

    namespace huf_data {

        /// Structure for each as ascii node 
        struct Node {
            uint16_t ch; 
            Node *left;
            Node *right;
            int freq;

            Node() : ch(0), left(nullptr), right(nullptr), freq(0) {}
            Node(uint16_t c, Node *l, Node *r, int f) : 
                ch(c), left(l), right(r), freq(f) {}

            bool isLeaf() {
                return left == nullptr && right == nullptr;
            }
        };


        /// Binary key structure
        struct binkey {
            unsigned char bytes[MAX_KEY_BYTES] = {};
            unsigned int used_bits = 0;
        };


        void shiftBytesLeft(std::pair<char*, size_t> bytes, int used, int shift);
        void shiftBytesRight(std::pair<char*, size_t> bytes, int used, int shift);
        void shiftToMSB(unsigned char *bytes, size_t len, int shift);
        void shiftToLSB(unsigned char *bytes, size_t len, int shift);


        /// Log the encoding table to stdout
        void logEntable(binkey *entable);
    }



    class huf_tree {
        private:
            // comparison class for std::priority_queue
            class less { 
                public: 
                    bool operator()(huf_data::Node *p1, huf_data::Node *p2) {
                        return p1->freq > p2->freq;
                    }
            };

        protected:
            bool m_is_used = false;
            uint32_t m_file_size = 0;
            std::priority_queue<huf_data::Node*, std::vector<huf_data::Node*>, less> m_forest;
            uint32_t m_freq[ALPHABET_SIZE] = {};
            huf_data::Node *m_root = nullptr;
            

        public:
            void mkTree();
            void logTree(huf_data::Node *root, std::string bin);
            void clearTree(huf_data::Node *root);
    };



    class huf_encoder : public huf_tree {
        private:
            huf_data::binkey m_entable[ALPHABET_SIZE] = {};

        private:
            void fillFrequencyTable(std::ifstream &ifile);
            void buildHuffmanTables(huf_data::Node *root, huf_data::binkey bkey);
            void resetEntable();
            uint16_t findFreqTableSize();
            void encode(std::ifstream &ifile, std::ofstream &ofile);

        public:
            huf_encoder();
            huf_encoder(std::ifstream &ifile, std::ofstream &ofile);
            ~huf_encoder();
            void encodeFile(std::ifstream &ifile, std::ofstream &ofile);
    };



    class huf_decoder : public huf_tree {
        private:
            void loadDetable(std::ifstream &ifile);

        public:
            huf_decoder();
            huf_decoder(std::ifstream &ifile, std::ofstream &ofile);
            ~huf_decoder();
            void decodeFile(std::ifstream &ifile, std::ofstream &ofile);
    };
}

#endif
