#include <string>
#include <vector>
#include <iostream>

#include <Api.h>
#define LIBDAS_DEFS_ONLY
#include <HuffmanCompression.h>
#undef LIBDAS_DEFS_ONLY

#include <Base64Decoder.h>

int main() {
    std::string encoded_str;
    std::getline(std::cin, encoded_str);

    Libdas::Base64Decoder decoder;
    std::vector<char> decoded_str = decoder.Decode(encoded_str);

    for(char ch : decoded_str)
        std::cout << ch;
    std::cout << std::endl;
    return 0;
}
