#define BASE64_DECODER_CPP
#include <Base64Decoder.h>

namespace Libdas {

    Base64Decoder::Base64Decoder(const std::string &_base64_str) : m_base64_str(_base64_str) {
        // least significant 6 bits are relevant
        m_base64_table['A'] = 0b00000000;
        m_base64_table['B'] = 0b00000100;
        m_base64_table['C'] = 0b00001000;
        m_base64_table['D'] = 0b00001100;
        m_base64_table['E'] = 0b00010000;
        m_base64_table['F'] = 0b00010100;
        m_base64_table['G'] = 0b00011000;
        m_base64_table['H'] = 0b00011100;
        m_base64_table['I'] = 0b00100000;
        m_base64_table['J'] = 0b00100100;
        m_base64_table['K'] = 0b00101000;
        m_base64_table['L'] = 0b00101100;
        m_base64_table['M'] = 0b00110000;
        m_base64_table['N'] = 0b00110100;
        m_base64_table['O'] = 0b00111000;
        m_base64_table['P'] = 0b00111100;
        m_base64_table['Q'] = 0b01000000;
        m_base64_table['R'] = 0b01000100;
        m_base64_table['S'] = 0b01001000;
        m_base64_table['T'] = 0b01001100;
        m_base64_table['U'] = 0b01010000;
        m_base64_table['V'] = 0b01010100;
        m_base64_table['W'] = 0b01011000;
        m_base64_table['X'] = 0b01011100;
        m_base64_table['Y'] = 0b01100000;
        m_base64_table['Z'] = 0b01100100;
                                      
        m_base64_table['a'] = 0b01101000;
        m_base64_table['b'] = 0b01101100;
        m_base64_table['c'] = 0b01110000;
        m_base64_table['d'] = 0b01110100;
        m_base64_table['e'] = 0b01111000;
        m_base64_table['f'] = 0b01111100;
        m_base64_table['g'] = 0b10000000;
        m_base64_table['h'] = 0b10000100;
        m_base64_table['i'] = 0b10001000;
        m_base64_table['j'] = 0b10001100;
        m_base64_table['k'] = 0b10010000;
        m_base64_table['l'] = 0b10010100;
        m_base64_table['m'] = 0b10011000;
        m_base64_table['n'] = 0b10011100;
        m_base64_table['o'] = 0b10100000;
        m_base64_table['p'] = 0b10100100;
        m_base64_table['q'] = 0b10101000;
        m_base64_table['r'] = 0b10101100;
        m_base64_table['s'] = 0b10110000;
        m_base64_table['t'] = 0b10110100;
        m_base64_table['u'] = 0b10111000;
        m_base64_table['v'] = 0b10111100;
        m_base64_table['w'] = 0b11000000;
        m_base64_table['x'] = 0b11000100;
        m_base64_table['y'] = 0b11001000;
        m_base64_table['z'] = 0b11001100;
                                      
        m_base64_table['0'] = 0b11010000;
        m_base64_table['1'] = 0b11010100;
        m_base64_table['2'] = 0b11011000;
        m_base64_table['3'] = 0b11011100;
        m_base64_table['4'] = 0b11100000;
        m_base64_table['5'] = 0b11100100;
        m_base64_table['6'] = 0b11101000;
        m_base64_table['7'] = 0b11101100;
        m_base64_table['8'] = 0b11110000;
        m_base64_table['9'] = 0b11110100;
        m_base64_table['+'] = 0b11111000;
        m_base64_table['/'] = 0b11111100;
    }


    std::vector<char> Base64Decoder::Decode(const std::string &_base64_str) {
        if(_base64_str != "")
            m_base64_str = "";

        std::vector<char> decode_str;

        // bytes that do not have their complete value yet,
        //  first - byte value itself
        //  second - free bits in byte to use
        std::pair<char, int32_t> loose_bytes[2] = {};
        loose_bytes[0].second = 8;
        loose_bytes[1].second = 8;
        for(size_t i = 0; i < _base64_str.size(); i++) {
            int32_t shift = 8 - loose_bytes[0].second;
            loose_bytes[0].first |= (m_base64_table[_base64_str[i]] >> shift);
            loose_bytes[0].second = std::max<int32_t>(static_cast<int32_t>(loose_bytes[0].second) - 6, 0);
            
            // add remaning bits to second byte
            if(shift > 2) {
                loose_bytes[1].first |= m_base64_table[_base64_str[i]] << (8 - shift);
                loose_bytes[1].second = 8 - shift + 2;
            }
            
            // byte can be added to decode_str
            if(!loose_bytes[0].second) {
                decode_str.push_back(loose_bytes[0].first);
                loose_bytes[0] = loose_bytes[1];
                loose_bytes[1].first = 0;
                loose_bytes[1].second = 8;
            }
        }

        return decode_str;
    }
}
