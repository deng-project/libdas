#include <iostream>
#include <cstring>
#include <array>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <queue>

#include <Points.h>
#include <ErrorHandlers.h>
#include <AsciiStreamReader.h>
#include <AsciiLineReader.h>
#include <STLStructures.h>
#include <STLParser.h>


int main(int argc, char *argv[]) {
    if(argc < 2) {
        std::cout << "Please provide a binary STL file name as an argument" << std::endl;
        std::exit(-1);
    }

    Libdas::BinarySTLParser stl_parser(argv[1]);
    stl_parser.Parse();

    Libdas::STLObject obj = stl_parser.GetObject();
    std::cout << "Object name: " << obj.name << std::endl;
    std::cout << "Facets count: " << obj.facets.size() << std::endl;
    std::cout << "Vertices count: " << obj.facets.size() * 3 << std::endl;

    return 0;
}
