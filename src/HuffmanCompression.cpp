/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: HuffmanCompression.cpp - Huffman coding compression algorithm implementation
/// author: Karl-Mihkel Ott


/// Huffman encoded format: 
///    signature: null terminated string "HUF"
///    file size: 4 byte unsigned integer to determine original file size
///    frequency table size: 4 byte unsigned integer n to determine frequency table size
///    freqency table: n sets of 1 byte value and 4 byte unsigned integer to determine frequency
///    encoded data


#define HUFFMAN_COMPRESSION_CPP
#include <HuffmanCompression.h>


namespace Libdas {

    namespace Huffman {

        void ShiftBytesLeft(std::pair<char*, size_t> bytes, int used, int shift) {
            LIBDAS_ASSERT(static_cast<int>(bytes.second) > shift);
            memmove(bytes.first, bytes.first + shift, used - shift);
            memset(bytes.first + (used - shift), 0, shift);
        }


        void ShiftBytesRight(std::pair<char*, size_t> bytes, int used, int shift) {
            LIBDAS_ASSERT(static_cast<int>(bytes.second) > shift);
            memmove(bytes.first + shift, bytes.first, used - shift);
            memset(bytes.first, 0, shift);
        }


        void ShiftToMSB(unsigned char *bytes, size_t len, int shift) {
            LIBDAS_ASSERT(shift <= 8);
            if(shift >= 8) {
                ShiftBytesRight(std::make_pair(reinterpret_cast<char*>(bytes), len), len, shift);
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


        void ShiftToLSB(unsigned char *bytes, size_t len, int shift) {
            if(shift >= 8) {
                ShiftBytesLeft(std::make_pair(reinterpret_cast<char*>(bytes), len), len, shift / 8);
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


#ifdef _DEBUG
        void LogEntable(BinaryKey *entable) {
            size_t size = 0;
            for(int i = 0; i < LIBDAS_ASCII_ALPHABET_SIZE; i++) {
                if(entable[i].used_bits != UINT_MAX) {
                    std::cout << "used_bits for " << static_cast<char>(i) << " is " << entable[i].used_bits << " ";

                    size_t bsize = entable[i].used_bits / 8 + (entable[i].used_bits % 8 ? 1 : 0);
                    for(size_t j = LIBDAS_MAX_KEY_BYTES - bsize; j < LIBDAS_MAX_KEY_BYTES; j++) {
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
#endif

        /**********************/
        /***** Tree class *****/
        /**********************/

        void Tree::MakeTree() {
            // start by creating a forest out of nodes
            for(uint16_t i = 0; i < LIBDAS_ASCII_ALPHABET_SIZE; i++) {
                if(m_freq[i] > 0) {
                    Node *nnode = new Node(i, nullptr, nullptr, m_freq[i]);
                    m_forest.push(nnode);
                }
            }

            // construct a single tree from the forest of nodes 
            while(m_forest.size() > 1) {
                Node *l = m_forest.top();
                m_forest.pop();
                Node *r = m_forest.top();
                m_forest.pop();

                Node *nn = new Node(UINT16_MAX, l, r, l->freq + r->freq);
                m_forest.push(nn);
            }

            m_root = m_forest.top();
            m_forest.pop();
        }


        void Tree::ClearTree(Node *root) {
            // recursively call clean tree on next root
            if(root->left) {
                ClearTree(root->left);
                root->left = nullptr;
            }

            if(root->right) {
                ClearTree(root->right);
                root->right = nullptr;
            }

            if(root->IsLeaf()) {
                delete root;
                return;
            }
        }


        /*************************/
        /***** Encoder class *****/
        /*************************/

        Encoder::Encoder() {
            _ResetEntable();
        }


        Encoder::Encoder(std::ifstream &ifile, std::ofstream &ofile) {
            _ResetEntable();
            EncodeFile(ifile, ofile);
        }


        Encoder::~Encoder() {
            if(m_is_used) 
                ClearTree(m_root);
        }


        /// Fill the frequency table with all used bytes
        void Encoder::_FillFrequencyTable(std::ifstream &ifile) {
            unsigned char ch;
            while(!ifile.eof()) {
                ch = static_cast<unsigned char>(ifile.get());
                m_freq[ch]++;
                m_file_size++;
            }

            ifile.clear();
        }


        void Encoder::_BuildHuffmanTables(Node *root, BinaryKey bkey) {
            if(root->IsLeaf()) {
                m_entable[(int) root->ch] = bkey;
                return;
            }
            
            
            //unsigned char size = bkey.used_bits / 8 + (bkey.used_bits % 8 ? 1 : 0);
            //shiftBytesLeft(std::make_pair(reinterpret_cast<char*>(bkey.bytes), MAX_KEY_BYTES), static_cast<int>(size), 1);
            ShiftToMSB(bkey.bytes, LIBDAS_MAX_KEY_BYTES, 1);
            bkey.used_bits++;
            _BuildHuffmanTables(root->left, bkey);

            bkey.bytes[LIBDAS_MAX_KEY_BYTES - 1] |= 1;
            _BuildHuffmanTables(root->right, bkey);
        }


        void Encoder::_ResetEntable() {
            for(int i = 0; i < LIBDAS_ASCII_ALPHABET_SIZE; i++)
                m_entable[i].used_bits = UINT_MAX;
        }


        /// Find the size of the frequency table for each non-zero frequency byte
        uint16_t Encoder::_FindFreqTableSize() {
            uint16_t size = 0;
            for(int i = 0; i < LIBDAS_ASCII_ALPHABET_SIZE; i++)
                if(m_freq[i]) size++;

            return size;
        }


        /// Evil data encoder from ifile to ofile
        void Encoder::_Encode(std::ifstream &_ifile, std::ofstream &_ofile) {
            unsigned char r1 = 8;
            unsigned char b1 = 0, b2 = 0;

            unsigned char ch;
            uint32_t csize = 0;
            while(true) {
                ch = static_cast<unsigned char>(_ifile.get());
                csize++;

                BinaryKey key = m_entable[(int) ch];
                
                // for each byte in bits, check its value
                unsigned char bc = key.used_bits / 8 + (key.used_bits % 8 ? 1 : 0);
                for(int i = LIBDAS_MAX_KEY_BYTES - bc; i < LIBDAS_MAX_KEY_BYTES; i++) {

                    // if the first MSB is the current byte, check the key remainer and previous LSB remainer and 
                    // augment the written byte accordingly
                    if(i == LIBDAS_MAX_KEY_BYTES - bc && key.used_bits % 8) {
                        // if the key remainer is larger than previous key LSB remainer
                        if(key.used_bits % 8 == r1) {
                            b1 |= key.bytes[i];
                            _ofile.put(b1);
                            b1 = 0;
                            r1 = 8;
                        }

                        else if(key.used_bits % 8 > r1) {
                            b2 = key.bytes[i] >> ((key.used_bits % 8) - r1);
                            b1 |= b2;
                            _ofile.put(b1);

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
                            _ofile.put(b1);

                            b1 = key.bytes[i] << r1;
                        }

                        else _ofile.put(key.bytes[i]);
                    }
                }

                // Check if the last byte has been reached and exit if needed
                if(csize == m_file_size) {
                    _ofile.put(b1);
                    return;
                }
            }
        }


        /// Encode the data into given file stream
        void Encoder::EncodeFile(std::ifstream &ifile, std::ofstream &ofile) {
            if(m_is_used) {
                Tree::ClearTree(m_root);
                m_root = nullptr;
                m_file_size = 0;
            }
            else m_is_used = true;

            // create encoding and decoding tables 
            _FillFrequencyTable(ifile);
            ifile.seekg(0, std::ios_base::beg);
            MakeTree();

            BinaryKey bkey = {};
            _BuildHuffmanTables(m_root, bkey);

            // write decode table to stdout
            std::string sig = "HUF";
            ofile.write(sig.c_str(), sig.size() * sizeof(char) + 1);
            ofile.write(reinterpret_cast<char*>(&m_file_size), sizeof(uint32_t));
            uint16_t freq = _FindFreqTableSize(); 
            ofile.write(reinterpret_cast<char*>(&freq), sizeof(uint16_t));

            for(int i = 0; i < LIBDAS_ASCII_ALPHABET_SIZE; i++) {
                if(m_freq[i]) {
                    unsigned char ch = static_cast<unsigned char>(i);
                    ofile.put(ch);
                    ofile.write(reinterpret_cast<char*>(m_freq + i), sizeof(uint32_t));
                }
            }

            _Encode(ifile, ofile);
        }


        // decoder class
        Decoder::Decoder() {}


        Decoder::Decoder(std::ifstream &ifile, std::ofstream &ofile) {
            DecodeFile(ifile, ofile);
        }


        Decoder::~Decoder() {
            if(m_is_used)
                ClearTree(m_root);

            _FreeBuffer();
        }


        /// Load decoding table into memory
        void Decoder::_LoadDetable(std::ifstream &ifile) {
            uint16_t size;
            ifile.read(reinterpret_cast<char*>(&size), sizeof(uint16_t));
            for(int i = 0; i < size; i++) {
                unsigned char k; 
                uint32_t f;
                k = ifile.get();
                ifile.read(reinterpret_cast<char*>(&f), sizeof(uint32_t));
                m_freq[k] = f;

                // end of file reached unexpectedly
                if(ifile.eof()) {
                    std::cerr << "Unexpected EOF aborting" << std::endl;
                    EXIT_ON_ERROR(LIBDAS_ERROR_INVALID_FILE);
                }
            }
        }


        uint32_t Decoder::_LoadHeader(std::ifstream &_ifile) {
            if(m_is_used) {
                ClearTree(m_root);
                m_root = nullptr;
            }
            m_is_used = true;

            char sig[4] = {};
            uint32_t fsize;
            _ifile.read(sig, 4 * sizeof(char));
            _ifile.read(reinterpret_cast<char*>(&fsize), sizeof(uint32_t));
            _LoadDetable(_ifile);
            MakeTree();

            return fsize;
        }


        void Decoder::DecodeFile(std::ifstream &_ifile, std::ofstream &_ofile) {
            // Load bytes by byte
            unsigned char b = 0;
            uint32_t fsize = _LoadHeader(_ifile);
            uint32_t cur_size = 0;
            Node *root = m_root;

            while(!_ifile.eof()) {
                b = _ifile.get();
                // For each bit in the byte try to read its output
                for(int i = 7; i >= 0; i--) {
                    if(((b >> i) & 1))
                        root = root->right;
                    else root = root->left;

                    if(root->IsLeaf() && root->ch < LIBDAS_ASCII_ALPHABET_SIZE) {
                        cur_size++; 
                        if(cur_size == fsize) return;
                        _ofile.put(static_cast<char>(root->ch));
                        root = m_root;
                    }
                }
            }
        }


        void Decoder::_FreeBuffer() {
            if(m_buffer) {
                free(m_buffer);
                m_buffer = nullptr;
            }
        }


        char *Decoder::DecodeFileToMemory(std::ifstream &_ifile, uint32_t *_o_size) {
            _LoadHeader(_ifile);
            unsigned char b = 0;
            Node *root = m_root;
            uint32_t fsize = _LoadHeader(_ifile);
            uint32_t cur_size = 0;

            // free the memory if needed
            _FreeBuffer();

            // allocate memory for decoded buffer
            char *m_buffer = reinterpret_cast<char*>(malloc(fsize + 1));
            *_o_size = 0;

            while(!_ifile.eof()) {
                // for each bit in the byte read its value
                for(int i = 7; i >= 0; i--) {
                    if((b >> i) & 1)
                        root = root->right;
                    else root = root->left;

                    if(root->IsLeaf() && root->ch < LIBDAS_ASCII_ALPHABET_SIZE) {
                        cur_size++;
                        if(cur_size == fsize) return m_buffer;
                        m_buffer[*_o_size] = static_cast<char>(root->ch);
                        root = m_root;
                    }
                }
            }

            return m_buffer;
        }
    }
}
