// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: JSONParser.h - JSON parsing class header
// author: Karl-Mihkel Ott

#ifndef JSON_PARSER_H
#define JSON_PARSER_H


#ifdef JSON_PARSER_CPP
    #include <map>
    #include <fstream>
    #include <vector>
    #include <string>
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
        JSON_TOKEN_STATEMENT_END,
        JSON_TOKEN_NUMERICAL,
        JSON_TOKEN_KEY_VALUE_DECL,
        JSON_TOKEN_UNKNOWN
    };


    enum JSONType {
        JSON_TYPE_STRING,
        JSON_TYPE_FLOAT,
        JSON_TYPE_INTEGER
    };


    union JSONValue {
        std::string str_val;
        float fl_val;
        uint32_t int_val;
    };


    struct JSONNode {
        std::map<std::string, JSONNode> sub_nodes = {};
        std::vector<std::pair<JSONType, JSONValue>> values;
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

            // variable for accounting lines
            size_t m_line_nr = 1;

        private:
            ////////////////////////////////
            // ***** Token handling ***** //
            ////////////////////////////////
            
            void _HandleScopeStartToken();
            void _HandleScopeEndToken();
            void _HandleArrayStartToken();
            void _HandleArrayEndToken();
            void _HandleStringToken();
            void _HandleStatementEndToken(); // unnecessary?
            void _HandleNumericalToken();
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
            JSONParser(const std::string &_file_name = "");
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
