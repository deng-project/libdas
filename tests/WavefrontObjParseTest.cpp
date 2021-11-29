/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: WavefrontObjParseTest.h - WavefrontObjParser class testing application
/// test purpose: Read Wavefront Obj file and display its read groups to stdout
/// author: Karl-Mihkel Ott

#include <cstdlib>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <queue>
#include <utility>
#include <memory>
#include <unordered_map>

#include <Points.h>
#include <LibdasAssert.h>
#include <ErrorHandlers.h>
#include <AsciiStreamReader.h>
#include <AsciiLineReader.h>
#include <WavefrontObjStructures.h>
#include <WavefrontObjParser.h>

using Groups = Libdas::WavefrontObjFunctions::Groups;


int main(int argc, char *argv[]) {
    if(argc < 2) {
        std::cout << "Please provide a Wavefront OBJ file as an argument" << std::endl;
        std::exit(-1);
    }

    Libdas::WavefrontObjParser parser;
    parser.Parse(argv[1]);
    parser.TriangulateGroups();

    const Groups &groups = parser.GetParsedGroups();
    
    for(const Libdas::WavefrontObjGroup &group : groups) {
        // display group names 
        std::cout << "Group names: ";
        for(size_t i = 0; i < group.names.size(); i++)
            std::cout << "'" << group.names[i] << "' ";
        std::cout << std::endl;

        // display vertex counts
        std::cout << "Position vertices count: " << group.vertices.position.size() << std::endl;
        std::cout << "Texture vertices count: " << group.vertices.texture.size() << std::endl;
        std::cout << "Vertex normals count: " << group.vertices.normals.size() << std::endl;

        // display faces counts
        std::cout << "Faces: " << group.indices.faces.size() << std::endl << std::endl;
    }

    return 0;
}
