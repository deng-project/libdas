// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: DASTool.h - header for CLI tool that manages DAS files and creates new DAS files from other 3D model formats 
// author: Karl-Mihkel Ott

#ifndef DAS_TOOL_H
#define DAS_TOOL_H

#ifdef DAS_TOOL_CPP
    #include <any>
    #include <array>
    #include <string>
    #include <ctime>
    #include <fstream>
    #include <iostream>
    #include <cstring>
    #include <cmath>
    #include <vector>
    #include <stdexcept>
    #include <unordered_map>
    
    #include <Api.h>
    #include <FileNameString.h>
    #include <LibdasAssert.h>
    #include <ErrorHandlers.h>
    #include <AsciiStreamReader.h>
    #include <AsciiLineReader.h>
    #include <Iterators.h>
    #include <Points.h>
    #include <Vector.h>
    #include <Matrix.h>
    #include <Quaternion.h>
    #include <DasStructures.h>
    #include <DasWriterCore.h>
    #include <DasReaderCore.h>
    #include <DasParser.h>
    #include <STLStructures.h>
    #include <STLParser.h>
    #include <STLCompiler.h>
    #include <WavefrontObjStructures.h>
    #include <WavefrontObjParser.h>
    #include <WavefrontObjCompiler.h>
#endif

typedef uint8_t FlagType;
#define OUTPUT_FLAG_COMPRESSED      0x01
#define OUTPUT_FLAG_NO_CURVES       0x02
#define OUTPUT_FLAG_AUTHOR          0x04
#define OUTPUT_FLAG_COPYRIGHT       0x08
#define OUTPUT_FLAG_MODEL           0x10
#define OUTPUT_FLAG_VERBOSE         0x20
#define OUTPUT_FLAG_OUT_FILE        0x40
#define OUTPUT_FLAG

#define VERSION_MAJOR       0
#define VERSION_MINOR       1


class DASTool {
    private:
        const std::string m_help_text = "Usage: DASTool convert|list <input file> [output options]\n";
        std::unordered_map<std::string, FlagType> m_flag_map = {
            std::make_pair("-c", OUTPUT_FLAG_COMPRESSED),
            std::make_pair("--compressed", OUTPUT_FLAG_COMPRESSED),
            std::make_pair("--no-curves", OUTPUT_FLAG_NO_CURVES),
            std::make_pair("--author", OUTPUT_FLAG_AUTHOR),
            std::make_pair("--copyright", OUTPUT_FLAG_COPYRIGHT),
            std::make_pair("--model", OUTPUT_FLAG_MODEL),
            std::make_pair("-v", OUTPUT_FLAG_VERBOSE),
            std::make_pair("--verbose", OUTPUT_FLAG_VERBOSE),
            std::make_pair("-o", OUTPUT_FLAG_OUT_FILE),
            std::make_pair("--output", OUTPUT_FLAG_OUT_FILE)
        };
        FlagType m_flags = 0;
        Libdas::DasProperties m_props;
        std::string m_author = std::string("DASTool v") + std::to_string(VERSION_MAJOR) + std::string(".") + std::to_string(VERSION_MINOR);
        std::string m_model = "";
        std::string m_copyright = "";
        std::string m_out_file;

    private:
        ////////////////////////////////////
        // ***** Conversion methods ***** //
        ////////////////////////////////////
        
        /**
         * Convert STL file into DAS file. Output file will be either provided
         * output file or input file name with das extension
         * @param _input_file is specified STL file to read from
         */
        void _ConvertSTL(const std::string &_input_file);
        /**
         * Convert Wavefront Obj file into DAS file. Output file will be either provided
         * output file or input file name with das extension
         * @param _input_file is specified Wavefront Obj file to read from
         */
        void _ConvertWavefrontObj(const std::string &_input_file);
        /**
         * Convert GLTF file into DAS file. Output file will be either provided
         * output file or input file name with das extension
         * @param _input_file is specified GLTF file to read from
         */
        void _ConvertGLTF(const std::string &_input_file);
        /**
         * Convert GLB file into DAS file. Output file will be either provided output
         * file or input file name with das extension
         * @param _input_file is specified GLB file to read from
         */
        void _ConvertGLB(const std::string &_input_file);

        /////////////////////////////////
        // ***** Listing methods ***** //
        /////////////////////////////////

        /**
         * List render attributes of given Wavefront Obj group
         * @param _group is a reference to WavefrontObjGroup where render attributes are read
         */
        void _ListWavefrontObjRenderAttributes(const Libdas::WavefrontObjGroup &_group);
        /**
         * List data from STL file to stdout
         * @param _input_file specifies the STL file to read
         */
        void _ListSTL(const std::string &_input_file);
        /**
         * List data from Wavefront Obj file to stdout
         * @param _input_file specifies the Wavefront Obj file to read
         */
        void _ListWavefrontObj(const std::string &_input_file);
        /**
         * List data from GLTF file to stdout
         * @param _input_file specifies the GLTF file to read
         */
        void _ListGLTF(const std::string &_input_file);
        /**
         * List data from GLB file to stdout
         * @param _input_file specifies the GLB file to read
         */
        void _ListGLB(const std::string &_input_file);
        /**
         * List data from DasProperties
         * @param _props specifies a reference to DasProperties object where all properties about file are stored
         */
        void _ListDasProperties(Libdas::DasProperties &_props);
        /**
         * List all data about all meshes that were parsed
         * @param _parser is a reference to DasParser instance that specifies the parser used
         */
        void _ListDasMeshes(Libdas::DasParser &_parser);
        /**
         * List all data about skeltons that were parsed
         * @param _parser is a reference to DasParser instance that specifies the parser used
         */
        void _ListDasSkeletons(Libdas::DasParser &_parser);
        /**
         * List all data about skeleton joints that were parsed
         * @param _parser is a reference to DasParser instance that specifies the parser used
         */
        void _ListDasSkeletonJoints(Libdas::DasParser &_parser);
        /**
         * List all data about scenes to stdout
         * @param _parser specifies the parser used for parsing the file
         */
        void _ListDasScenes(Libdas::DasParser &_parser);
        /**
         * List all data about scene nodes to stdout
         * @param _parser specifies a reference to parser used for parsing the file
         */
        void _ListDasNodes(Libdas::DasParser &_parser);
        /**
         * List all data about all animations that were parsed
         * @param _parser is a reference to DasParser instance that specifies the parser used
         */
        void _ListDasAnimations(Libdas::DasParser &_parser);
        /**
         * List data from DAS file to stdout
         * @param _input_file specifies the DAS file to read
         */
        void _ListDas(const std::string &_input_file);
        /**
         * Parse all string output flags into bitmask values
         * @param _opts is a const std::vector reference that holds all string flags
         */
        void _ParseFlags(const std::vector<std::string> &_opts);
        /**
         * Exclude all invalid options according to the specified mode
         * @param _is_convert is true if the convertion mode was requested
         */
        void _ExcludeFlags(bool _is_convert);

    public:
        /**
         * Convert from specified file to DAS file format
         * @param _input_file specifies the input file used for conversion to DAS format
         * @param _opts specifies output options 
         */
        void Convert(const std::string &_input_file, const std::vector<std::string> &_opts);
        /**
         * List all information about current file
         * @param _input_file specifies the input file used for reading
         * @param _opts specifies output options
         */
        void List(const std::string &_input_file, const std::vector<std::string> &_opts);
        /**
         * Get DASTool help text
         */
        const std::string &GetHelpText();
};

#endif
