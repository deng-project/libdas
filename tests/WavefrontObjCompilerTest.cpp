#include <vector>
#include <cstdint>
#include <climits>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include <memory>
#include <unordered_map>

#include <Points.h>
#include <Vector.h>
#include <Matrix.h>
#include <AsciiStreamReader.h>
#include <AsciiLineReader.h>
#include <ErrorHandlers.h>
#include <LibdasAssert.h>
#include <FileNameString.h>
#include <DasStructures.h>
#include <DasWriterCore.h>
#include <WavefrontObjStructures.h>
#include <WavefrontObjParser.h>
#include <WavefrontObjCompiler.h>


int main(int argc, char *argv[]) {
    if(argc < 2) {
        std::cerr << "Please provide a Wavefront OBJ file name as an argument" << std::endl;
        std::exit(-1);
    }

    std::string ext = Libdas::String::ExtractFileExtension(argv[1]);
    std::string file_name = Libdas::String::ExtractFileName(argv[1]);

    if(ext != "obj" && ext != "OBJ") {
        std::cerr << "Invalid file name " << file_name << std::endl;
        std::exit(LIBDAS_ERROR_INVALID_FILE);
    }

    Libdas::WavefrontObjParser parser(argv[1]);
    parser.Parse();
    file_name += ".das";

    Libdas::DasProperties props;
    props.author = "inugami-dev64";
    props.compression = false;
    props.copyright = "GPL or something like that";
    props.moddate = static_cast<uint64_t>(time(NULL));
    props.model = argv[1];

    Libdas::WavefrontObjCompiler compiler(parser.GetParsedGroups(), props, file_name);

    return 0;
}
