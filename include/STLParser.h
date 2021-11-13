/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: STLParser.h - STL parser class header
/// author: Karl-Mihkel Ott

#ifndef STL_PARSER_H
#define STL_PARSER_H


#ifdef STL_PARSER_CPP
    #include <string>
    #include <iostream>
    #include <queue>
    #include <fstream>
    #include <unordered_map>

    #include <Points.h>
    #include <LibdasAssert.h>
    #include <ParserErrorHandler.h>
    #include <AsciiStreamReader.h>
    #include <AsciiLineReader.h>
    #include <STLStructures.h>
#endif

namespace Libdas {

    enum AsciiSTLStatementType {
        ASCII_STL_STATEMENT_NONE,                       // special value to define empty newline
        ASCII_STL_STATEMENT_SOLID,                      // solid
        ASCII_STL_STATEMENT_FACET,                      // facet
        ASCII_STL_STATEMENT_OUTER_LOOP,                 // outer loop
        ASCII_STL_STATEMENT_VERTEX,                     // vertex
        ASCII_STL_STATEMENT_ENDLOOP,                    // endloop
        ASCII_STL_STATEMENT_ENDFACET,                   // endfacet
        ASCII_STL_STATEMENT_ENDSOLID                    // endsolid
    };


    struct AsciiSTLStatementCallback {
        AsciiSTLStatementType type;
        STLFunctions::PTR_KeywordCallback keyword_callback; 

        AsciiSTLStatementCallback() : type(ASCII_STL_STATEMENT_NONE), keyword_callback(nullptr) {}

        AsciiSTLStatementCallback(const AsciiSTLStatementType &_type, const STLFunctions::PTR_KeywordCallback &_kw_callback) :
            type(_type), keyword_callback(_kw_callback) {}
    };


    class AsciiSTLParser : private AsciiLineReader {
        private:
            AsciiFormatErrorHandler m_error;
            std::unordered_map<std::string, AsciiSTLStatementCallback> m_tokens;
            std::queue<STLObject> m_objects;
            uint32_t m_parse_pos = 1;

            // boolean flags for defining facet, vertex loop and solid object reading statuses
            bool m_is_solid_obj = false;
            bool m_is_facet = false;
            bool m_is_loop = false;

        private:
            /** 
             * Create all possible tokens to consider for Wavefront Obj format
             */
            void _Tokenise();
            /**
             * Analyse the keyword and if it is valid return its callback
             * @param _key is the key to use in retrieving from the keyword map
             * @return on success AsciiSTLStatementCallback instance that includes a valid type and data pointer to statement callback
             */
            AsciiSTLStatementCallback _AnalyseKeyword(const std::string &_key);
            /**
             * Analyse the statement and call its callback function if it exists
             * @param _callback is an AsciiSTLStatement instance that defines the function pointer
             * @param _args is an std::vector instance of all read arguments
             */
            void _AnalyseStatement(AsciiSTLStatementCallback &_callback, std::vector<std::string> &_args);
            /**
             * Get the correct callback function custom data according to the specified type
             * @param _type is a AsciiSTLStatementType value that describes the statement type
             */
            void *_GetCustomData(AsciiSTLStatementType _type);
        
        public:
            AsciiSTLParser(const std::string &_file_name = "", size_t _chunk_size = DEFAULT_CHUNK);
            /**
             * Parse the currently loaded file given as a parameter in constructor
             */
            void Parse();
            /**
             * Parse completely new file
             */
            void Parse(const std::string &_file_name);
            /**
             * Pop parsed STL group from the queue
             * @return most front STLObject instance from the queue
             */
            STLObject PopSTLObjectFromQueue();
            /**
             * Check if the STLObject queue is empty or not
             * @return true if the queue is empty, false otherwise
             */
            bool IsObjectQueueEmpty();
    };
}

#endif
