#include <string>
#include <fstream>
#include <cstring>
#include <iostream>
#include <unordered_map>
#include <array>
#include <vector>

#include <Iterators.h>
#include <Points.h>
#include <Vector.h>
#include <Matrix.h>

#include <LibdasAssert.h>
#include <FileNameString.h>
#include <AsciiStreamReader.h>
#include <AsciiLineReader.h>
#include <ErrorHandlers.h>
#include <DasStructures.h>
#include <DasWriterCore.h>
#include <STLStructures.h>
#include <STLParser.h>
#include <STLCompiler.h>


int main(int argc, char *argv[]) {
    if(argc < 2) {
        std::cerr << "Please provide an STL file as an argument" << std::endl;
        std::exit(0);
    }


    // properties to write to out file
    Libdas::DasProperties props;
    props.author = "Karl-Mihkel Ott";
    props.compression = false;
    props.copyright = "idc";
    props.moddate = static_cast<uint64_t>(time(NULL));
    props.model = "An example STL model to test";
    
    std::string in_file = argv[1];
    std::string out_file = Libdas::String::ReplaceFileExtension(in_file, "das");
    out_file = Libdas::String::ExtractFileName(out_file);
    bool is_ascii = Libdas::STLFunctions::Identify(argv[1]);

    if(is_ascii) {
        Libdas::AsciiSTLParser parser(in_file);
        parser.Parse();
        Libdas::STLCompiler(parser.GetObjects(), props, out_file);
    } else {
        Libdas::BinarySTLParser parser(in_file);
        parser.Parse();

        std::vector<Libdas::STLObject> objects;
        objects.push_back(parser.GetObject());
        Libdas::STLCompiler(objects, props, out_file);
    }


    return 0;
}
