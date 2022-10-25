// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: DASTool.h - header for CLI tool that manages DAS files and creates new DAS files from other 3D model formats 
// author: Karl-Mihkel Ott

#ifndef DAS_TOOL_H
#define DAS_TOOL_H

#ifdef DAS_TOOL_CPP
    #include <any>
    #include <array>
    #include <variant>
    #include <map>
    #include <cfloat>
    #include <memory>
    #include <string>
    #include <ctime>
    #include <fstream>
    #include <iostream>
    #include <cstring>
    #include <cmath>
    #include <vector>
    #include <stack>
    #include <stdexcept>
    #include <filesystem>
    #include <unordered_map>
    
    #include "trs/Iterators.h"
    #include "trs/Points.h"
    #include "trs/Vector.h"
    #include "trs/Matrix.h"
    #include "trs/Quaternion.h"

    #include "mar/AsciiStreamReader.h"
    #include "mar/AsciiLineReader.h"

    #include "das/Api.h"
    #include "das/Version.h"
    #include "das/Algorithm.h"
    #include "das/LibdasAssert.h"
    #include "das/ErrorHandlers.h"
    #include "das/Hash.h"
    #include "das/DasStructures.h"
    #include "das/TextureReader.h"
    #include "das/DasWriterCore.h"
    #include "das/DasReaderCore.h"
    #include "das/DasParser.h"
    #include "das/STLStructures.h"
    #include "das/STLParser.h"
    #include "das/STLCompiler.h"
#define LIBDAS_DEFS_ONLY
    #include "das/HuffmanCompression.h"
#undef LIBDAS_DEFS_ONLY
    #include "das/Base64Decoder.h"
    #include "das/URIResolver.h"
    #include "das/GLTFStructures.h"
    #include "das/JSONParser.h"
    #include "das/GLTFParser.h"
    #include "das/GLTFCompiler.h"
    #include "das/WavefrontObjStructures.h"
    #include "das/WavefrontObjParser.h"
    #include "das/WavefrontObjCompiler.h"
    #include "das/DasValidator.h"
#endif

typedef uint16_t FlagType;
#define USAGE_FLAG_NONE             0x0000
#define USAGE_FLAG_AUTHOR           0x0001
#define USAGE_FLAG_COPYRIGHT        0x0002
#define USAGE_FLAG_EMBED_TEXTURE    0x0004
#define USAGE_FLAG_MODEL            0x0008
#define USAGE_FLAG_OUT_FILE         0x0010
#define USAGE_FLAG_HELP             0x0020
#define USAGE_FLAG_VERBOSE          0x0040


class DASTool {
    private:
        const std::string m_help_text =
            "DASTool version " + std::to_string(LIBDAS_VERSION_MAJOR) + "." + std::to_string(LIBDAS_VERSION_MINOR) + "." + std::to_string(LIBDAS_VERSION_REVISION) + "\n"\
            "Usage: DASTool convert|list|validate <input file> [output options]\n"\
            "Valid conversion options:\n"\
            "--author \"<Author>\" - specify model author's name\n"\
            "--copyright \"<Message>\" - specify copyright message as an argument string\n"\
            "--embed-texture \"<FileName>\" - embed an image file to the output\n"\
            "--model \"<ModelName>\" - specify model name\n"\
            "-o / --output \"<OutFile>\" - specify output file name\n"\
            "-h / --help - display help text\n"\
            "Valid listing options:\n"\
            "-v / --verbose - output verbose message about the object\n"\
            "-h / --help - display help text\n";

        FlagType m_flags = 0;
        Libdas::DasProperties m_props;
        std::string m_author = std::string("DASTool v") + std::to_string(LIBDAS_VERSION_MAJOR) + std::string(".") + std::to_string(LIBDAS_VERSION_MINOR) + "." + std::to_string(LIBDAS_VERSION_REVISION);
        std::string m_copyright;
        std::vector<std::string> m_embedded_textures;
        std::vector<uint32_t> m_removed_textures;
        std::string m_model;
        std::string m_out_file;

    private:
        ////////////////////////////////////
        // ***** Conversion methods ***** //
        ////////////////////////////////////
        
        void _ConvertDAS(const std::string &_input_file);
        void _ConvertSTL(const std::string &_input_file);
        void _ConvertWavefrontObj(const std::string &_input_file);
        void _ConvertGLTF(const std::string &_input_file);
        void _ConvertGLB(const std::string &_input_file);
        void _ConvertFBX(const std::string &_input_file);

        /////////////////////////////////
        // ***** Listing methods ***** //
        /////////////////////////////////

        void _ListWavefrontObjRenderAttributes(const Libdas::WavefrontObjGroup &_group);
        void _ListSTL(const std::string &_input_file);
        void _ListWavefrontObj(const std::string &_input_file);
        void _ListGLTF(const std::string &_input_file);
        void _ListGLB(const std::string &_input_file);
        void _ListDasProperties(const Libdas::DasProperties &_props);
        void _ListDasBuffers(Libdas::DasParser &_parser);
        void _ListDasMeshes(Libdas::DasParser &_parser);
        void _ListDasMeshPrimitive(Libdas::DasParser &_parser, uint32_t _rel_id, uint32_t _id); // called from _ListDasMeshes()
        void _ListDasMorphTarget(Libdas::DasParser &_parser, uint32_t _rel_id, uint32_t _id);   // called from _ListDasMeshPrimitive()
        void _ListDasSkeletons(Libdas::DasParser &_parser);
        void _ListDasSkeletonJoints(Libdas::DasParser &_parser);
        void _ListDasScenes(Libdas::DasParser &_parser);
        void _ListDasNodes(Libdas::DasParser &_parser);
        void _ListDasAnimationChannels(Libdas::DasParser &_parser);
        void _ListDasAnimations(Libdas::DasParser &_parser);
        void _ListDas(const std::string &_input_file);

        
        void _FetchArg(FlagType _type, const std::string &_arg);
        void _MakeOutputFile(const std::string &_input_file);
        void _MakeProps();
        void _ParseFlags(const std::vector<std::string> &_opts);
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
         * Validate DAS file
         * @param _input_file specifies the input file used for reading
         */
        void Validate(const std::string &_input_file);
        /**
         * Get DASTool help text
         */
        inline const std::string &GetHelpText() {
            return m_help_text;
        }
};

#endif
