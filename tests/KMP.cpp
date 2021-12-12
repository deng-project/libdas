
// INPUT: on line one first string and on line two the search string
// OUTPUT: single integer describing the search string occurence in the first string
#include <iostream>
#include <vector>
#include <string>
#include <FileNameString.h>


int main() {
    std::string str, search;
    std::getline(std::cin, str);
    std::getline(std::cin, search);


    std::vector<size_t> occurences = Libdas::String::FindSubstringInstances(str, search);

    for(size_t i : occurences) {
        std::cout << i << " ";
    }

    std::cout << std::endl;
    return 0;
}
