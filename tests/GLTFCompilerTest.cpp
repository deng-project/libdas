#include <any>
#include <vector>
#include <fstream>
#include <cfloat>
#include <map>
#include <unordered_map>
#include <variant>
#include <memory>
#include <iostream>

#include <Api.h>
#include <Points.h>
#include <Vector.h>
#include <Matrix.h>
#include <Quaternion.h>
#include <DasStructures.h>
#include <TextureReader.h>
#include <DasWriterCore.h>
#include <GLTFStructures.h>
#include <AsciiStreamReader.h>
#include <AsciiLineReader.h>
#include <ErrorHandlers.h>
#include <JSONParser.h>
#include <GLTFParser.h>
#include <GLTFCompiler.h>


int main(int argc, char *argv[]) {
    if(argc < 2) {
        std::cerr << "Please provide a gltf file name as an argument to continue" << std::endl;
        std::exit(LIBDAS_ERROR_INVALID_ARGUMENT);
    }

    Libdas::GLTFParser parser(argv[1]);
    parser.Parse();

    parser.GetRootObject();
    return 0;
}
