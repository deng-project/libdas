
// INPUT: on line one first string and on line two the search string
// OUTPUT: single integer describing the search string occurence in the first string
#include <iostream>
#include <vector>

int *GenerateLSPArray(const std::string &_search) {
    int *lsp = new int[_search.size()];
    lsp[0] = 0;

    for(int i = 1; i < static_cast<int>(_search.size()); i++) {
        int j = lsp[i - 1];

        // skip invalid prefixes
        while(j > 0 && _search[i] != _search[j])
            j = lsp[j - 1];

        if(_search[i] == _search[j])
            j++;

        lsp[i] = j;
    }

    return lsp;
}


std::vector<int> Search(int *lsp, const std::string &_str, const std::string &_search) {
    std::vector<int> occurences;
    occurences.reserve(_str.size()); // worst case

    for(int i = 0, j = 0; i < static_cast<int>(_str.size()); i++) {
        // fallback on the pattern if no match was found
        while(j > 0 && _str[i] != _search[j])
            j = lsp[j - 1];

        if(_str[i] == _search[j]) {
            j++;
            if(j == static_cast<int>(_search.size()))
                occurences.push_back(i - j + 1);
        }
    }

    return occurences;
}


int main() {
    std::string str, search;
    std::getline(std::cin, str);
    std::getline(std::cin, search);

    int *lsp = GenerateLSPArray(search);
    std::vector<int> occurences = Search(lsp, str, search);

    for(int i : occurences)
        std::cout << i << " ";

    std::cout << std::endl;

    return 0;
}
