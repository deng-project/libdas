#include <cstring>
#include <string>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <queue>
#include <array>

#include <Points.h>
#include <ErrorHandlers.h>
#include <AsciiStreamReader.h>
#include <AsciiLineReader.h>
#include <STLStructures.h>
#include <STLParser.h>


int main(int argc, char *argv[]) {
    if(argc < 2) {
        std::cout << "Please provide an Ascii STL file name as an argument" << std::endl;
        std::exit(-1);
    }

    Libdas::AsciiSTLParser stl_parser(argv[1]);
    stl_parser.Parse();

    std::vector<Libdas::STLObject> &objects = stl_parser.GetObjects();
    for(Libdas::STLObject &obj : objects) {
        std::cout << "Object name: " << obj.name << std::endl;
        std::cout << "Facets count: " << obj.facets.size() << std::endl;
        std::cout << "Vertices count: " << obj.facets.size() * 3 << std::endl;
    }

    return 0;
}
