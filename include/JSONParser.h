// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: JSONParser.h - JSON parsing class header
// author: Karl-Mihkel Ott

#ifndef JSON_PARSER_H
#define JSON_PARSER_H


#ifdef JSON_PARSER_CPP
    #include <any>
    #include <variant>
    #include <map>
    #include <fstream>
    #include <memory>
    #include <vector>
    #include <string>
    #include <cstring>
    #include <utility>

    #include <ErrorHandlers.h>
    #include <AsciiStreamReader.h>
#endif


namespace Libdas {

    enum JSONToken {
        JSON_TOKEN_SCOPE_START,
        JSON_TOKEN_SCOPE_END,
        JSON_TOKEN_ARRAY_START,
        JSON_TOKEN_ARRAY_END,
        JSON_TOKEN_STRING_STATEMENT,
        JSON_TOKEN_BOOLEAN,
        JSON_TOKEN_NUMERICAL,
        JSON_TOKEN_KEY_VALUE_DECL,
        JSON_TOKEN_NEXT_ELEMENT,
        JSON_TOKEN_UNKNOWN
    };


    typedef uint8_t JSONType;
    #define JSON_TYPE_STRING    0x01
    #define JSON_TYPE_FLOAT     0x02
    #define JSON_TYPE_INTEGER   0x04
    #define JSON_TYPE_BOOLEAN   0x08
    #define JSON_TYPE_OBJECT    0x10
    #define JSON_TYPE_UNKNOWN   0x20


    // JSON data type definitions
    typedef std::string JSONString;
    typedef float JSONNumber;
    typedef int32_t JSONInteger;
    typedef bool JSONBoolean;


    // Value and subnode type declarations for easier typing
    struct JSONNode;
    typedef std::vector<std::pair<JSONType, std::any>> JSONValues;
    typedef std::map<std::string, std::shared_ptr<JSONNode>> SubnodeMap;

    struct JSONNode {
        JSONNode() {}
        JSONNode(const JSONNode &_node) { *this = _node; }

        JSONNode *parent = nullptr;
        std::string name = "root";
        SubnodeMap sub_nodes = {};
        JSONValues values;
        uint32_t key_val_decl_line = 1;

        // flags for data reading
        bool is_scope_open = false;
        bool is_array_open = false;
    };


    class JSONParser : public AsciiStreamReader {
        private:
            AsciiFormatErrorHandler m_error;
            JSONNode m_root_node;
            JSONNode *m_active_node = &m_root_node;

            std::string m_file_name;
            char *m_rd_ptr = m_buffer;
            char m_str_statement_beg = 0;
            bool m_allow_next_element = true; // flag that determines if 

            std::string m_loose_string = "";

            // variable for accounting lines
            size_t m_line_nr = 1;
            bool m_prev_decl = false; // boolean flag for identifying previous key value statement

        private:
            ////////////////////////////////
            // ***** Token handling ***** //
            ////////////////////////////////
            
            void _HandleScopeStartToken();
            void _HandleScopeEndToken();
            void _HandleArrayStartToken();
            void _HandleArrayEndToken();
            void _HandleStringToken();
            void _HandleBooleanToken();
            void _HandleNumericalToken();
            void _HandleNextElementToken();
            void _HandleKeyValueDeclToken();

            /**
             * Check the current char and idenitfy possible token value
             * @return JSONToken value specifying the identified token
             */
            JSONToken _CheckForToken();
            /**
             * Check which action to take according to the specified token
             * @param _token specifies the token which determines the action to be taken
             */
            void _CheckTokenAction(JSONToken _token);
            /**
             * Skip all whitespaces and other skippable characters 
             * @param _no_nl is a boolean value that specifies, if true, that no newlines would be skipped
             */
            void _SkipSkippableCharacters(bool _no_nl = false);

        public:
            JSONParser(ModelFormat _format, const std::string &_file_name = "");
            /**
             * Parse the JSON file and create appropriate maps for each node
             * @param _file_name optionally specifies the JSON file name to use, can be ignored if the file name was provided in constructor
             */
            void Parse(const std::string &_file_name = "");
            /**
             * Get the root node of parsed JSON nodes
             * @return reference to JSONNode object that is the root of all other objects
             */
            JSONNode &GetRootNode();
    };
}


#endif
