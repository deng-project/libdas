/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: DASTool.h - CLI tool for managing DAS files and creating new DAS files from other 3D model formats 
/// author: Karl-Mihkel Ott

#define DAS_TOOL_CPP
#include <DASTool.h>

///////////////////////////////
// ***** DASTool class ***** //
///////////////////////////////


// incomplete method
void DASTool::_ConvertSTL(const std::string &_input_file) {
    std::cerr << "Feature not yet implemented" << std::endl; 
    LIBDAS_ASSERT(false); // necessary measures rn

    bool is_ascii = Libdas::STLFunctions::Identify(_input_file);
    if(is_ascii) {
        Libdas::AsciiSTLParser parser(_input_file);
        parser.Parse();
    } else {
        Libdas::BinarySTLParser parser(_input_file);
        parser.Parse();
    }
}


void DASTool::_ConvertWavefrontOBJ(const std::string &_input_file) {
    std::string &out_file = m_out_file; 

    if(out_file == "") {
        out_file = Libdas::String::ReplaceFileExtension(_input_file, "das");
        out_file = Libdas::String::ExtractFileName(out_file);
    }

    Libdas::WavefrontObjParser parser(_input_file);
    parser.Parse();

    Libdas::WavefrontObjCompiler(parser.GetParsedGroups(), m_props, out_file);
}


void DASTool::Convert(const std::string &_input_file, const std::vector<std::string> &_opts) {
    for(size_t i = 0; i < _opts.size(); i++) {
        // invalid flag
        if(m_flag_map.find(_opts[i]) == m_flag_map.end()) {
            std::cerr << "Invalid output option '" << _opts[i] << "'" << std::endl;
            std::cout << m_help_text;
            std::exit(1);
        }

        else m_flags |= m_flag_map[_opts[i]];
    }

    std::string ext = Libdas::String::ExtractFileExtension(_input_file);
    if(ext == "stl") {
        _ConvertSTL(_input_file);
        return;
    }
    else if(ext == "obj") {
        _ConvertWavefrontOBJ(_input_file);
        return;
    }

    LIBDAS_ASSERT(false);
}


void DASTool::List(const std::string &_input_file) {
    std::cerr << "Feature not yet implemented" << std::endl;
    LIBDAS_ASSERT(false);
}


const std::string &DASTool::GetHelpText() {
    return m_help_text;
}


// main method
int main(int argc, char *argv[]) {
    DASTool tool;
    if(argc < 3) {
        std::cout << tool.GetHelpText();
        return 0;
    }


    std::vector<std::string> opts;
    if(argc > 4) {
        opts.reserve(argc - 3);
        for(int i = 3; i < argc; i++)
            opts.push_back(argv[i]);
    }

    if(std::string(argv[1]) == "convert")
        tool.Convert(argv[2], opts);
    else if(std::string(argv[1]) == "list")
        tool.List(argv[2]);

    return 0;
}
