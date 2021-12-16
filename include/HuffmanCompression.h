/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: HuffmanCompression.h - Huffman coding compression algorithm header
/// author: Karl-Mihkel Ott


#ifndef HUFFMAN_COMPRESSION_H
#define HUFFMAN_COMPRESSION_H

#ifdef HUFFMAN_COMPRESSION_CPP
    #include <fstream>
    #include <cstring>
    #include <climits>
    #include <iostream>
    #include <queue>

    #include <LibdasAssert.h>
    #include <ErrorHandlers.h>
#endif

#define LIBDAS_ASCII_ALPHABET_SIZE      256
#define LIBDAS_MAX_KEY_BYTES            32  

#ifndef LIBDAS_DEFS_ONLY

namespace Libdas {

    namespace Huffman {

        /// Structure for each as ascii node 
        struct Node {
            uint16_t ch; 
            Node *left;
            Node *right;
            int freq;

            Node() : ch(0), left(nullptr), right(nullptr), freq(0) {}
            Node(uint16_t _ch, Node *_left, Node *_right, int _freq) : 
                ch(_ch), left(_left), right(_right), freq(_freq) {}

            bool IsLeaf() {
                return left == nullptr && right == nullptr;
            }
        };


        /// Binary key structure
        struct BinaryKey {
            unsigned char bytes[LIBDAS_MAX_KEY_BYTES] = {};
            unsigned int used_bits = 0;
        };


        /** 
         * Shift bytes in byte array to left
         * NOTE: in bytes left byte means more significant byte and right means less significant byte, 
         * the memory area must be large enough to store in location bytes - shift 
         * @param _bytes is an std::pair object that specifies correspondingly byte array with its length 
         * @param _used specifies the amount of used bits  
         * @param _shift specifies the byte shift amount
         */
        void ShiftBytesLeft(std::pair<char*, size_t> _bytes, int _used, int _shift);
        /** 
         * Shift bytes in byte array to right
         * NOTE: in bytes left byte means more significant byte and right means less significant byte, 
         * the memory area must be large enough to store in location bytes - shift 
         * @param _bytes is an std::pair object that specifies correspondingly byte array with its length 
         * @param _used specifies the amount of used bits  
         * @param _shift specifies the byte shift amount
         */
        void ShiftBytesRight(std::pair<char*, size_t> _bytes, int _used, int _shift);
        /**
         * Perform binary shift towards most significant bit across given memory area
         * @param _bytes is a pointer to given memory area
         * @param _len is the size of the given memory area in bytes
         * @param _shift total amount of bits to shift
         */
        void ShiftToMSB(unsigned char *_bytes, size_t _len, int _shift);
        /**
         * Perform binary shift towards least significant bit accross given memory area
         * @param _bytes is a pointer to given memory area
         * @param _len is the size of the given memory area in bytes
         * @param _shift total amount of bits to shift
         */
        void ShiftToLSB(unsigned char *_bytes, size_t _len, int _shift);


#ifdef _DEBUG
        /// Log the encoding table to stdout
        void LogEntable(BinaryKey *entable);
#else 
    #define LogEntable(table)
#endif


        /**
         * Huffman tree class that contains all nodes
         */
        class Tree {
            private:
                // comparison class for std::priority_queue
                class less { 
                    public: 
                        bool operator()(Node *p1, Node *p2) {
                            return p1->freq > p2->freq;
                        }
                };

            protected:
                bool m_is_used = false;
                uint32_t m_file_size = 0;
                std::priority_queue<Node*, std::vector<Node*>, less> m_forest;
                uint32_t m_freq[LIBDAS_ASCII_ALPHABET_SIZE] = {};
                Node *m_root = nullptr;
                

            public:
                /**
                 * Create a forest of nodes and make a tree structure out of them
                 */
                void MakeTree();
                /**
                 * Clear all created nodes from the tree and free all allocated memory
                 */
                void ClearTree(Node *root);
        };


        /**
         * Raw data to Huffman encoded bytes encoder
         */
        class Encoder : public Tree {
            private:
                BinaryKey m_entable[LIBDAS_ASCII_ALPHABET_SIZE] = {};

            private:
                /**
                 * Fill the frequency table with all used bytes
                 * @param _ifile is a std::ifstream reference that has input file opened for reading
                 */
                void _FillFrequencyTable(std::ifstream &_ifile);
                /**
                 * Build the encoding table
                 * @param _root is a pointer to the root node
                 * @param _bkey is a BinaryKey value that specifies the used key for the current node
                 */
                void _BuildHuffmanTables(Node *_root, BinaryKey _bkey);
                /**
                 * Clear all entries in the encoding table
                 */
                void _ResetEntable();
                /**
                 * Find the total count of char values that have instances
                 * @return uint16_t value specifying registred characters in current frequency array
                 */
                uint16_t _FindFreqTableSize();
                /**
                 * Encode data from _ifile to _ofile
                 * @param _ifile is an input stream where data is read from
                 * @param _ofile is an output stream where data is put to
                 */
                void _Encode(std::ifstream &_ifile, std::ofstream &_ofile);

            public:
                Encoder();
                Encoder(std::ifstream &_ifile, std::ofstream &_ofile);
                ~Encoder();
                /**
                 * Encode the data from _ifile to _ofile
                 * @param _ifile is an input stream to read data from
                 * @param _ofile is an output stream to write data into
                 */
                void EncodeFile(std::ifstream &_ifile, std::ofstream &_ofile);
        };



        /**
         * Encoded data to raw data decoder class
         */
        class Decoder : public Tree {
            private:
                char *m_buffer = nullptr;

            private:
                /**
                 * Load decoding table from _ifile
                 * @param _ifile is an input stream to read the table from
                 */
                void _LoadDetable(std::ifstream &_ifile);
                /**
                 * Load header data, verify signature, read detable and return the original file size
                 * @param _ifile is a input stream to read data from
                 * @return uint32_t value specifying the original file size
                 */
                uint32_t _LoadHeader(std::ifstream &_file);
                /**
                 * Check and free the previously allocated buffer memory area, if needed
                 */
                void _FreeBuffer();

            public:
                Decoder();
                Decoder(std::ifstream &_ifile, std::ofstream &_ofile);
                ~Decoder();
                /**
                 * Decode encoded file from _ifile into _ofile
                 * @param _ifile is an input stream to read data from
                 * @param _ofile is an output stream to write data into
                 */
                void DecodeFile(std::ifstream &_ifile, std::ofstream &_ofile);
                /**
                 * Decode encoded data into memory
                 * @param _ifile is an input stream to read data from
                 * @param _o_size is a valid pointer to uint32_t instance that specifies the buffer size
                 */
                char *DecodeFileToMemory(std::ifstream &_ifile, uint32_t *_o_size);
        };
    }
}

#endif
#endif
