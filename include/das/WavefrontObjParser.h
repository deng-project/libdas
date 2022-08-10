// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: WavefrontObjParser.h - Wavefront Obj parser header file
// author: Karl-Mihkel Ott

// NOTE: Curves and surfaces are not yet implemented, coming later

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

    #include "trs/Points.h"

    #include "mar/AsciiStreamReader.h"
    #include "mar/AsciiLineReader.h"

    #include "das/Api.h"
    #include "das/ErrorHandlers.h"
    #include "das/WavefrontObjStructures.h"
#endif


namespace Libdas {

    struct WavefrontObjStatementCallback {
        WavefrontObjStatementType type = WAVEFRONT_OBJ_STATEMENT_NONE;
        WavefrontObjFunctions::PTR_KeywordCallback keyword_callback = nullptr;

        WavefrontObjStatementCallback() = default;

        WavefrontObjStatementCallback(const WavefrontObjStatementCallback &_cb) : type(_cb.type), keyword_callback(_cb.keyword_callback) {}

        WavefrontObjStatementCallback(const WavefrontObjStatementType &_type, const WavefrontObjFunctions::PTR_KeywordCallback &_kw_callback) :
            type(_type), keyword_callback(_kw_callback) {}

        void operator=(const WavefrontObjStatementCallback &_val) {
            type = _val.type;
            keyword_callback = _val.keyword_callback;
        }
    };


    /**
     * Interface to parse Wavefront OBJ files
     */
    class LIBDAS_API WavefrontObjParser : public MAR::AsciiLineReader {
        private:
            std::unordered_map<std::string, WavefrontObjStatementCallback> m_statement_map;
            //std::vector<std::unique_ptr<MAR::AsciiStreamReader>> m_mtl_streams;
            uint32_t m_parse_pos = 1;
            std::string m_file_name;
            AsciiFormatErrorHandler m_error;
            WavefrontObjData m_parsed_data;

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
            WavefrontObjStatementCallback _AnalyseKeyword(char *_end);
            /**
             * Analyse arguments and push them into correct data structures
             * @param _callback is an instance to WavefrontObjStatementCallback
             * @param _args are all arguments given in a string value
             */
            void _AnalyseArgs(WavefrontObjStatementCallback _callback, std::vector<std::string> &_args);

        public:
            WavefrontObjParser(const std::string &_file_name = "", size_t _chunk_size = DEFAULT_CHUNK);
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
             * Get all parsed WavefrontObj groups
             * @return reference to std::vector<WavefrontObjGroup>
             */
            inline WavefrontObjData &GetParsedData() {
                return m_parsed_data;
            }
    };
}

#endif

