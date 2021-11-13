/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: WavefrontObjParser.h - Wavefront Obj parser header file
/// author: Karl-Mihkel Ott


/// NOTE: Curves and surfaces are not yet implemented, coming later

#ifndef WAVEFRONT_OBJ_PARSER_H
#define WAVEFRONT_OBJ_PARSER_H


#ifdef WAVEFRONT_OBJ_PARSER_CPP
    #include <string>
    #include <cstring>
    #include <fstream>
    #include <iostream>
    #include <vector>
    #include <queue>
    #include <utility>
    #include <unordered_map>
    #include <memory>

    #include <Points.h>
    #include <ParserErrorHandler.h>
    #include <AsciiStreamReader.h>
    #include <AsciiLineReader.h>
#endif


/// Import all structures
#include <WavefrontObjStructures.h>


namespace Libdas {

    struct WavefrontObjStatementReader {
        WavefrontObjStatementType type;
        WavefrontObjFunctions::PTR_KeywordArgParser keyword_parser;

        WavefrontObjStatementReader() : type(WAVEFRONT_OBJ_STATEMENT_NONE), keyword_parser(nullptr) {}

        WavefrontObjStatementReader(const WavefrontObjStatementType &_type, const WavefrontObjFunctions::PTR_KeywordArgParser &_kw_parser) :
            type(_type), keyword_parser(_kw_parser) {}

        void operator=(const WavefrontObjStatementReader &_val) {
            type = _val.type;
            keyword_parser = _val.keyword_parser;
        }
    };


    /**
     * Interface to parse Wavefront OBJ files
     */
    class WavefrontObjParser : private AsciiLineReader {
        private:
            std::unordered_map<std::string, WavefrontObjStatementReader> m_statement_map;
            std::vector<std::unique_ptr<AsciiStreamReader>> m_mtl_streams;
            uint32_t m_parse_pos = 1;
            std::string m_file_name;
            AsciiFormatErrorHandler m_error;
            WavefrontObjFunctions::Groups m_groups;

        private:
            /** 
             * Create all possible tokens to consider for Wavefront Obj format
             */
            void _Tokenize();
            /**
             * Analyse keyword meaning and return the keyword type or throw unknown keyword error
             * @param _pos is a pointer that defines the keyword starting position
             * @param _end is a pointer that defines the keyword ending position
             * @return WavefrontObjStatementType if the given keyword is valid
             */
            WavefrontObjStatementReader _AnalyseKeyword(char *_end);
            /**
             * Analyse arguments and push them into correct data structures
             * @param _keyword is the given statement keyword 
             * @param _args are all arguments given in a string value
             */
            void _AnalyseArgs(WavefrontObjStatementReader _reader, std::vector<std::string> &_args);

        public:
            WavefrontObjParser(size_t _chunk_size = DEFAULT_CHUNK, const std::string &_file_name = "");
            /** 
             * Parse the whole new file 
             * @param _file_name is an optional argument to use when parsing the Wavefront OBJ file
             */
            void Parse(const std::string &_file_name);
            /** 
             * Parse the whole currently open file 
             */
            void Parse();
            /**
             * Retrieve all read groups
             */
            WavefrontObjGroup PopFromGroupQueue();
            /**
             * Check if the group queue is empty
             */
            bool IsGroupQueueEmpty();
    };
}

#endif

