#include <any>
#include <vector>
#include <fstream>
#include <cfloat>
#include <cmath>
#include <cstring>
#include <map>
#include <unordered_map>
#include <variant>
#include <memory>
#include <iostream>

#include <Api.h>
#include <Points.h>
#include <LibdasAssert.h>
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
#include <Algorithm.h>
#define LIBDAS_DEFS_ONLY
    #include <HuffmanCompression.h>
#undef LIBDAS_DEFS_ONLY
#include <Base64Decoder.h>
#include <URIResolver.h>
#include <GLTFCompiler.h>


int main(int argc, char *argv[]) {
    if(argc < 2) {
        std::cerr << "Please provide a gltf file name as an argument to continue" << std::endl;
        std::exit(LIBDAS_ERROR_INVALID_ARGUMENT);
    }

    Libdas::GLTFParser parser(argv[1]);
    parser.Parse();
    Libdas::GLTFRoot &root = parser.GetRootObject();
    
    std::string path = Libdas::Algorithm::ExtractRootPath(argv[1]);
    std::string out = Libdas::Algorithm::ReplaceFileExtension(argv[1], "das");
    out = Libdas::Algorithm::ExtractFileName(out);

    Libdas::DasProperties props;
    props.author = "Author";
    props.copyright = "Copyright message";
    props.moddate = static_cast<uint64_t>(time(NULL));
    props.model = "Some model";

    Libdas::GLTFCompiler compiler(path, root, props, out);
    return 0;
}
