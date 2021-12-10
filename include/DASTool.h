/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: DASTool.h - header for CLI tool that manages DAS files and creates new DAS files from other 3D model formats 
/// author: Karl-Mihkel Ott
#ifndef DAS_TOOL_H
#define DAS_TOOL_H

#ifdef DAS_TOOL_CPP
    #include <any>
    #include <string>
    #include <fstream>
    #include <iostream>
    #include <cstring>
    #include <vector>
    #include <unordered_map>
    
    #include <FileNameString.h>
    #include <LibdasAssert.h>
    #include <ErrorHandlers.h>
    #include <AsciiStreamReader.h>
    #include <AsciiLineReader.h>
    #include <Points.h>
    #include <Vector.h>
    #include <Matrix.h>
    #include <DasStructures.h>
    #include <DasWriterCore.h>
    #include <DasReaderCore.h>
    #include <DasParser.h>
    #include <STLStructures.h>
    #include <STLParser.h>
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

// unused

//enum FileType {
    //FILE_STL,
    //FILE_WAVEFRONT_OBJ,
    //FILE_GLTF,
    //FILE_GLB,
    //FILE_DAS,
    //FILE_DASC
//};


class DASTool {
    private:
        const std::string m_help_text = "Usage: DASTool convert|list <input file> [output options]\n";
        std::unordered_map<std::string, FlagType> m_flag_map = {
            std::make_pair("-c", OUTPUT_FLAG_COMPRESSED),
            std::make_pair("--compressed", OUTPUT_FLAG_COMPRESSED),
            std::make_pair("--no-curves", OUTPUT_FLAG_NO_CURVES),
            std::make_pair("--author", OUTPUT_FLAG_AUTHOR),
            std::make_pair("--copyright", OUTPUT_FLAG_COPYRIGHT),
            std::make_pair("--model", OUTPUT_FLAG_MODEL)
        };
        FlagType m_flags = 0;
        Libdas::DasProperties m_props;
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
        void _ConvertWavefrontOBJ(const std::string &_input_file);
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
        /**
         * Parse all string output flags into bitmask values
         * @param _opts is a const std::vector reference that holds all string flags
         */
        void _ParseFlags(const std::vector<std::string> &_opts);

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
        void List(const std::string &_input_file);
        /**
         * Get DASTool help text
         */
        const std::string &GetHelpText();
};

#endif