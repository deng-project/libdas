/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: huf.cpp - Huffman coding algorithm implementation
/// author: Karl-Mihkel Ott


/// Huffman encoded format: 
///    signature: null terminated string "HUF"
///    file size: 4 byte unsigned integer to determine original file size
///    frequency table size: 4 byte unsigned integer n to determine frequency table size
///    freqency table: n sets of 1 byte value and 4 byte unsigned integer to determine frequency
///    encoded data


#define __HUF_CPP
#include <huf.h>


namespace libdas {

    namespace huf_data {

        /// Shift bytes in byte array to left
        /// NOTE: in bytes left byte means more significant byte and right means less significant byte,
        ///       the memory area must be large enough to store in location bytes - shift 
        void shiftBytesLeft(std::pair<char*, size_t> bytes, int used, int shift) {
            msgassert(static_cast<int>(bytes.second) > shift, "Could not shift by more bytes than there are in the array");
            memmove(bytes.first, bytes.first + shift, used - shift);
            memset(bytes.first + (used - shift), 0, shift);
        }


        /// Shift bytes in byte array to right
        /// NOTE: in bytes left byte means less significant byte and right means less significant byte,
        ///       the memory area must be large enough to store in location bytes + shift 
        void shiftBytesRight(std::pair<char*, size_t> bytes, int used, int shift) {
            msgassert(static_cast<int>(bytes.second) > shift, "Could not shift by more bytes than there are in the array");
            memmove(bytes.first + shift, bytes.first, used - shift);
            memset(bytes.first, 0, shift);
        }


        /// Binary shift left accross multiple bytes
        /// <arguments>
        //      bytes - the byte array that is going to be bitshifted
        //      len - length of bytes
        //      shift - the amount of bits to shift across the memory area (must be less than 8)
        /// </arguments>
        void shiftToMSB(unsigned char *bytes, size_t len, int shift) {
            msgassert(shift <= 8, "Cannot move bits left by more than 8 bits");
            if(shift >= 8) {
                shiftBytesRight(std::make_pair(reinterpret_cast<char*>(bytes), len), len, shift);
                shift %= 8;
            }

            // most significant remaining bits are stored to the least significant bits of reserv variables
            unsigned char r1 = 0, r2 = 0;
            //unsigned char b = 0;

            // NOTE: What happens, if the byte overflows by shifting?
            // SOLUTION: move the bytes to next more significant byte

            // For each byte in the array, reserve its most significant bits and perform bitshift to left
            for(int i = static_cast<int>(len) - 1; i >= 0; i--) {
                if(i < static_cast<int>(len) - 1) {
                    r2 = (bytes[i] >> (8 - shift));
                    bytes[i] <<= shift;
                    bytes[i] |= r1;
                    r1 = r2;
                }

                else {
                    r1 = (bytes[i] >> (8 - shift));
                    bytes[i] <<= shift;
                }
            }

            bytes[0] |= r1;
        }


        /// Binary shift right accross multiple bytes
        /// <arguments>
        //      bytes - the byte array that is going to be bitshifted
        //      len - length of bytes
        //      shift - the amount of bits to shift accross the memory area (must be less than 8)
        /// </arguments>
        void shiftToLSB(unsigned char *bytes, size_t len, int shift) {
            if(shift >= 8) {
                shiftBytesLeft(std::make_pair(reinterpret_cast<char*>(bytes), len), len, shift / 8);
                shift %= 8;
            }

            // least significant remaining bits are stored to the most significant bits of reserv variables
            unsigned char r1 = 0, r2 = 0;
            for(int i = 0; i < static_cast<int>(len); i++) {
                if(i) {
                    r2 = (bytes[i] << (8 - shift));
                    bytes[i] >>= shift;
                    bytes[i] |= r1;
                    r1 = r2;
                }

                else {
                    r1 = (bytes[i] << (8 - shift));
                    bytes[i] >>= shift;
                }
            }
        }


        void logEntable(binkey *entable) {
            size_t size = 0;
            for(int i = 0; i < ALPHABET_SIZE; i++) {
                if(entable[i].used_bits != UINT_MAX) {
                    std::cout << "used_bits for " << static_cast<char>(i) << " is " << entable[i].used_bits << " ";

                    size_t bsize = entable[i].used_bits / 8 + (entable[i].used_bits % 8 ? 1 : 0);
                    for(size_t j = MAX_KEY_BYTES - bsize; j < MAX_KEY_BYTES; j++) {
                        int shift = 7;
                        if(!j && entable[i].used_bits % 8) shift = entable[i].used_bits % 8;

                        for(int k = shift; k >= 0; k--)
                            std::cout << static_cast<int>((entable[i].bytes[j] >> k) & 1);
                    }

                    std::cout << std::endl;
                    size++;
                }
            }

            std::cout << "entable size: " << size << std::endl;
            return;
        }
    }


    /// Create a forest of nodes and make a tree out of them
    void huf_tree::mkTree() {
        // start by creating a forest out of nodes
        for(uint16_t i = 0; i < ALPHABET_SIZE; i++) {
            if(m_freq[i] > 0) {
                huf_data::Node *nnode = new huf_data::Node(i, nullptr, nullptr, m_freq[i]);
                m_forest.push(nnode);
            }
        }

        // construct a single tree from the forest of nodes 
        while(m_forest.size() > 1) {
            huf_data::Node *l = m_forest.top();
            m_forest.pop();
            huf_data::Node *r = m_forest.top();
            m_forest.pop();

            huf_data::Node *nn = new huf_data::Node(UINT16_MAX, l, r, l->freq + r->freq);
            m_forest.push(nn);
        }

        m_root = m_forest.top();
    }


    void huf_tree::logTree(huf_data::Node *root, std::string bin) {
        if(root->isLeaf()) {
            std::cout << "char " << static_cast<int>(root->ch) << " " << bin << std::endl;
            return;
        }
        
        logTree(root->left, bin + '0');
        logTree(root->right, bin + '1');
    }


    /// Clear all created nodes from the tree
    void huf_tree::clearTree(huf_data::Node *root) {
        if(root->isLeaf()) {
            delete root;
            return;
        }

        clearTree(root->left);
        root->left = nullptr;

        clearTree(root->right);
        root->right = nullptr;
    }


    // encoder class
    huf_encoder::huf_encoder() {
        resetEntable();
    }


    huf_encoder::huf_encoder(std::ifstream &ifile, std::ofstream &ofile) {
        resetEntable();
        encodeFile(ifile, ofile);
    }


    huf_encoder::~huf_encoder() {
        if(m_is_used)
            clearTree(m_root);
    }


    /// Fill the frequency table with all used bytes
    void huf_encoder::fillFrequencyTable(std::ifstream &ifile) {
        unsigned char ch;
        while(!ifile.eof()) {
            ch = static_cast<unsigned char>(ifile.get());
            m_freq[ch]++;
            m_file_size++;
        }

        ifile.clear();
    }


    void huf_encoder::buildHuffmanTables(huf_data::Node *root, huf_data::binkey bkey) {
        if(root->isLeaf()) {
            m_entable[(int) root->ch] = bkey;
            return;
        }
        
        
        //unsigned char size = bkey.used_bits / 8 + (bkey.used_bits % 8 ? 1 : 0);
        //huf_data::shiftBytesLeft(std::make_pair(reinterpret_cast<char*>(bkey.bytes), MAX_KEY_BYTES), static_cast<int>(size), 1);
        huf_data::shiftToMSB(bkey.bytes, MAX_KEY_BYTES, 1);
        bkey.used_bits++;
        buildHuffmanTables(root->left, bkey);

        bkey.bytes[MAX_KEY_BYTES - 1] |= 1;
        buildHuffmanTables(root->right, bkey);
    }


    void huf_encoder::resetEntable() {
        for(int i = 0; i < ALPHABET_SIZE; i++)
            m_entable[i].used_bits = UINT_MAX;
    }


    /// Find the size of the frequency table for each non-zero frequency byte
    uint16_t huf_encoder::findFreqTableSize() {
        uint16_t size = 0;
        for(int i = 0; i < ALPHABET_SIZE; i++)
            if(m_freq[i]) size++;

        return size;
    }


    /// Evil data encoder from ifile to ofile
    void huf_encoder::encode(std::ifstream &ifile, std::ofstream &ofile) {
        unsigned char r1 = 8;
        unsigned char b1 = 0, b2 = 0;

        unsigned char ch;
        uint32_t csize = 0;
        while(true) {
            ch = static_cast<unsigned char>(ifile.get());
            csize++;

            huf_data::binkey key = m_entable[(int) ch];
            
            // for each byte in bits, check its value
            unsigned char bc = key.used_bits / 8 + (key.used_bits % 8 ? 1 : 0);
            for(int i = MAX_KEY_BYTES - bc; i < MAX_KEY_BYTES; i++) {

                // if the first MSB is the current byte, check the key remainer and previous LSB remainer and 
                // augment the written byte accordingly
                if(i == MAX_KEY_BYTES - bc && key.used_bits % 8) {
                    // if the key remainer is larger than previous key LSB remainer
                    if(key.used_bits % 8 == r1) {
                        b1 |= key.bytes[i];
                        ofile.put(b1);
                        b1 = 0;
                        r1 = 8;
                    }

                    else if(key.used_bits % 8 > r1) {
                        b2 = key.bytes[i] >> ((key.used_bits % 8) - r1);
                        b1 |= b2;
                        ofile.put(b1);

                        r1 = 8 - (key.used_bits % 8) + r1;
                        b1 = key.bytes[i] << r1;
                    }

                    else if(key.used_bits % 8 < r1) { 
                        b2 = key.bytes[i] << (r1 - (key.used_bits % 8));
                        b1 |= b2;

                        r1 -= key.used_bits % 8;
                    }
                }

                else {
                    if(r1) {
                        b2 = key.bytes[i] >> (8 - r1);
                        b1 |= b2;
                        ofile.put(b1);

                        b1 = key.bytes[i] << r1;
                    }

                    else ofile.put(key.bytes[i]);
                }
            }

            // Check if the last byte has been reached and exit if needed
            if(csize == m_file_size) {
                ofile.put(b1);
                return;
            }
        }
    }


    /// Encode the data into given file stream
    void huf_encoder::encodeFile(std::ifstream &ifile, std::ofstream &ofile) {
        if(m_is_used) {
            huf_tree::clearTree(m_root);
            m_root = nullptr;
            m_file_size = 0;
        }
        else m_is_used = true;

        // create encoding and decoding tables 
        fillFrequencyTable(ifile);
        ifile.seekg(0, std::ios_base::beg);
        mkTree();

        huf_data::binkey bkey = {};
        buildHuffmanTables(m_root, bkey);

        // write decode table to stdout
        std::string sig = "HUF";
        ofile.write(sig.c_str(), sig.size() * sizeof(char) + 1);
        ofile.write(reinterpret_cast<char*>(&m_file_size), sizeof(uint32_t));
        uint16_t freq = findFreqTableSize(); 
        ofile.write(reinterpret_cast<char*>(&freq), sizeof(uint16_t));

        for(int i = 0; i < ALPHABET_SIZE; i++) {
            if(m_freq[i]) {
                unsigned char ch = static_cast<unsigned char>(i);
                ofile.put(ch);
                ofile.write(reinterpret_cast<char*>(m_freq + i), sizeof(uint32_t));
            }
        }

        encode(ifile, ofile);
    }


    // decoder class
    huf_decoder::huf_decoder() {}


    huf_decoder::huf_decoder(std::ifstream &ifile, std::ofstream &ofile) {
        decodeFile(ifile, ofile);
    }


    huf_decoder::~huf_decoder() {
        if(m_is_used)
            clearTree(m_root);
    }


    /// Load decoding table into memory
    void huf_decoder::loadDetable(std::ifstream &ifile) {
        uint16_t size;
        ifile.read(reinterpret_cast<char*>(&size), sizeof(uint16_t));
        for(int i = 0; i < size; i++) {
            unsigned char k; 
            uint32_t f;
            k = ifile.get();
            ifile.read(reinterpret_cast<char*>(&f), sizeof(uint32_t));
            m_freq[k] = f;
            msgassert(!ifile.eof(), "End of file reached unexpectedly");
        }
    }


    /// Decode the encoded ifile to ofile
    void huf_decoder::decodeFile(std::ifstream &ifile, std::ofstream &ofile) {
        if(m_is_used) {
            huf_tree::clearTree(m_root);
            m_root = nullptr;
        }
        m_is_used = true;

        char sig[4] = {};
        uint32_t fsize, cur_size = 0;
        ifile.read(sig, 4 * sizeof(char));
        ifile.read(reinterpret_cast<char*>(&fsize), sizeof(uint32_t));
        loadDetable(ifile);
        mkTree();

        // Load bytes by byte
        unsigned char b = 0;
        huf_data::Node *root = m_root;
        while(!ifile.eof()) {
            b = ifile.get();
            // For each bit in the byte try to read its output
            for(int i = 7; i >= 0; i--) {
                if(((b >> i) & 1))
                    root = root->right;
                else root = root->left;

                if(root->isLeaf() && root->ch < ALPHABET_SIZE) {
                    cur_size++; 
                    if(cur_size == fsize) return;
                    ofile.put(static_cast<char>(root->ch));
                    root = m_root;
                }
                else if(root->isLeaf() && root->ch >= ALPHABET_SIZE) 
                    msgassert(nullptr, "Corrupt decode table");
            }
        }
    }
}
