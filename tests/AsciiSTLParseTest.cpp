#include <string>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <queue>
#include <array>

#include <Points.h>
#include <ParserErrorHandler.h>
#include <AsciiStreamReader.h>
#include <AsciiLineReader.h>
#include <STLStructures.h>
#include <STLParser.h>


int main(int argc, char *argv[]) {
    if(argc < 2) {
        std::cout << "Please provide an STL file name as an argument" << std::endl;
        std::exit(-1);
    }

    Libdas::AsciiSTLParser stl_parser(argv[1]);
    stl_parser.Parse();

    while(!stl_parser.IsObjectQueueEmpty()) {
        Libdas::STLObject obj = stl_parser.PopSTLObjectFromQueue();

        std::cout << "Object name: " << obj.name << std::endl;
        std::cout << "Facets count: " << obj.facets.size() << std::endl;
        std::cout << "Vertices count: " << obj.facets.size() * 3 << std::endl;
    }

    return 0;
}
