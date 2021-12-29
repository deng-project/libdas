// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: JSONParser.cpp - JSON parsing class implementation
// author: Karl-Mihkel Ott

#define JSON_PARSER_CPP
#include <JSONParser.h>

namespace Libdas {
    
    JSONParser::JSONParser(ModelFormat _format, const std::string &_file_name) : 
        AsciiStreamReader(_file_name, DEFAULT_CHUNK, "}"), m_error(_format), m_file_name(_file_name) {}


    JSONToken JSONParser::_CheckForToken() {
        switch(*m_rd_ptr) {
            case '{': return JSON_TOKEN_SCOPE_START;
            case '}': return JSON_TOKEN_SCOPE_END;
            case '[': return JSON_TOKEN_ARRAY_START;
            case ']': return JSON_TOKEN_ARRAY_END;
            case '\'': 
                m_str_statement_beg = '\'';
                return JSON_TOKEN_STRING_STATEMENT;
            case '\"': 
                m_str_statement_beg = '\"';
                return JSON_TOKEN_STRING_STATEMENT;
            case '\n': 
                m_line_nr++;
                break;
            case ',': return JSON_TOKEN_NEXT_ELEMENT;
            case ':': return JSON_TOKEN_KEY_VALUE_DECL;
            case 't':
            case 'f':
                return JSON_TOKEN_BOOLEAN;
            default: break;
        }

        if(*m_rd_ptr >= '0' && *m_rd_ptr <= '9')
            return JSON_TOKEN_NUMERICAL;

        return JSON_TOKEN_UNKNOWN;
    }


    void JSONParser::_HandleScopeStartToken() {
        // check if array was previously opened and if so add a new value regarding the the subobject
        if(m_active_node->is_array_open) {
            m_active_node->values.push_back(std::make_pair(JSON_TYPE_OBJECT, JSONNode()));
            std::any_cast<JSONNode&>(m_active_node->values.back().second).parent = m_active_node;
            std::any_cast<JSONNode&>(m_active_node->values.back().second).name = "";
            m_active_node = &std::any_cast<JSONNode&>(m_active_node->values.back().second);
            m_active_node->is_scope_open = true;
            m_prev_decl = false;
        }

        else {
            m_active_node->is_scope_open = true;
            m_prev_decl = false;
        }
    }


    void JSONParser::_HandleScopeEndToken() {
        // check if loose string value is available and if it is, push it to values
        if(m_loose_string != "") {
            m_active_node->values.push_back(std::make_pair(JSON_TYPE_STRING, m_loose_string));

            // fallback to parent
            if(m_active_node->parent)
                m_active_node = m_active_node->parent;
            m_loose_string = "";
        }

        // error: scope was previously closed, but new scope closure requested
        if(!m_active_node->is_scope_open)
            m_error.Error(LIBDAS_ERROR_SCOPE_ALREADY_CLOSED, m_line_nr, m_active_node->name);

        m_active_node->is_scope_open = false;
        m_prev_decl = false;

        // fallback to parent node
        if(m_active_node->parent)
            m_active_node = m_active_node->parent;
    }


    void JSONParser::_HandleArrayStartToken() {
        // error: array was previously opened, but new array opening requested
        if(m_active_node->is_array_open)
            m_error.Error(LIBDAS_ERROR_INCOMPLETE_SCOPE, m_line_nr);

        m_active_node->is_array_open = true;
    }


    void JSONParser::_HandleArrayEndToken() {
        // error: array was previously closed, but new array closure requested
        if(!m_active_node->is_array_open)
            m_error.Error(LIBDAS_ERROR_INCOMPLETE_SCOPE, m_line_nr);

        // loose string is available, push it to values
        if(m_loose_string != "") {
            m_active_node->values.push_back(std::make_pair(JSON_TYPE_STRING, m_loose_string));
            m_loose_string = "";
        }
        m_active_node->is_array_open = false;

        // fallback to parent node if possible
        if(m_active_node->parent)
            m_active_node = m_active_node->parent;
        m_prev_decl = false;
    }


    void JSONParser::_HandleStringToken() {
        // error: new element adding is not allowed
        if(!m_allow_next_element)
            m_error.Error(LIBDAS_ERROR_NO_IDENTIFIER, m_line_nr, ",");

        char *beg = m_rd_ptr;
        char *end_str = nullptr; 
        m_rd_ptr++;

        while(!end_str) {
            // check for the beginning statement
            if(m_str_statement_beg == '\"') end_str = strchr(m_rd_ptr, '\"');
            else if(m_str_statement_beg == '\'') end_str = strchr(m_rd_ptr, '\'');

            char *end_nl = strchr(m_rd_ptr, '\n');
            if((!end_nl || end_nl > end_str) && end_str) {
                if(*(end_str - 1) != '\\')
                    m_loose_string = beg + 1 == end_str ? std::string("(null)") : std::string(beg + 1, end_str);
                else {
                    m_rd_ptr = end_str + 1;
                    end_str = nullptr;
                }
            }

            // error unclosed string
            else if(end_nl < end_str || !_ReadNewChunk()) m_error.Error(LIBDAS_ERROR_INCOMPLETE_NEWLINE, m_line_nr);
        }

        m_rd_ptr = end_str;
        m_str_statement_beg = 0;
    }


    void JSONParser::_HandleBooleanToken() {
        // no previous value declaration was made, thus throw an error
        if(!m_prev_decl)
            m_error.Error(LIBDAS_ERROR_INCOMPLETE_SCOPE, m_line_nr, m_loose_string);

        std::string bool_str;

        while(m_rd_ptr < m_buffer + m_buffer_size && *m_rd_ptr != ',' && *m_rd_ptr != ' ' &&
              *m_rd_ptr != '\n' && *m_rd_ptr != '}' && *m_rd_ptr != ']' && *m_rd_ptr != '\r') {
            bool_str += *m_rd_ptr;
            m_rd_ptr++;
        }

        if(bool_str == "false" || bool_str == "true")
            m_active_node->values.push_back(std::make_pair(JSON_TYPE_BOOLEAN, bool_str == "true" ? true : false));
        else m_error.Error(LIBDAS_ERROR_INVALID_VALUE, m_line_nr, m_active_node->name);

        m_rd_ptr--;

        // check if fallback to parent scope should be made
        if(!m_active_node->is_scope_open && !m_active_node->is_array_open)
            m_active_node = m_active_node->parent;
    }


    void JSONParser::_HandleNumericalToken() {
        // no previous value declaration was made, thus throw an error
        if(!m_prev_decl)
            m_error.Error(LIBDAS_ERROR_INCOMPLETE_SCOPE, m_line_nr, m_loose_string);

        // extract a number string from the buffer
        bool is_fl = false;
        char *end = m_rd_ptr;
        while(end < m_buffer + m_buffer_size && ((*end >= '0' && *end <= '9') || *end == '.')) {
            if(*end == '.')
                is_fl = true;
            end++;
        }

        std::string num_str = std::string(m_rd_ptr, end - m_rd_ptr);
        std::variant<JSONInteger, JSONNumber> var_int;

        // detected a float value
        if(is_fl) {
            var_int = std::stof(num_str.c_str());
            m_active_node->values.push_back(std::make_pair(JSON_TYPE_FLOAT, var_int));
        }
        // detected integer value
        else {
            var_int = std::stoi(num_str.c_str());
            m_active_node->values.push_back(std::make_pair(JSON_TYPE_INTEGER, var_int));
        }

        // check if fallback to parent scope should be made
        if(!m_active_node->is_scope_open && !m_active_node->is_array_open)
            m_active_node = m_active_node->parent;

        m_rd_ptr = end - 1;
    }


    void JSONParser::_HandleKeyValueDeclToken() {
        // check if key value declaration was previously initiated and if so throw an error
        if(m_prev_decl)
            m_error.Error(LIBDAS_ERROR_INVALID_SYMBOL, m_line_nr, std::string(":"));

        // empty loose string declaration
        if(m_loose_string == "")
            m_error.Error(LIBDAS_ERROR_INVALID_KEYWORD, m_line_nr, "m_loose_string");
        
        // check if the previous node has its scope or array open to create a new subnode 
        if(m_active_node->is_array_open || m_active_node->is_scope_open) {
            // check if the key value exists and if it does throw an error
            if(m_active_node->sub_nodes.find(m_loose_string) != m_active_node->sub_nodes.end())
                m_error.Error(LIBDAS_ERROR_INVALID_KEYWORD, m_line_nr, m_loose_string);

            m_active_node->sub_nodes[m_loose_string] = std::make_shared<JSONNode>();
            m_active_node->sub_nodes[m_loose_string]->parent = m_active_node;
            m_active_node = m_active_node->sub_nodes[m_loose_string].get();
            m_active_node->name = m_loose_string;
            m_active_node->key_val_decl_line = m_line_nr;
            m_prev_decl = true;
        }

        else {
            // check if parent is present, otherwise throw an error
            if(!m_active_node->parent)
                m_error.Error(LIBDAS_ERROR_INVALID_KEYWORD, m_line_nr, m_loose_string);

            // check if subnode already exists in current context
            if(m_active_node->parent->sub_nodes.find(m_loose_string) != m_active_node->parent->sub_nodes.end())
                m_error.Error(LIBDAS_ERROR_INVALID_KEYWORD, m_line_nr, m_loose_string);

            m_active_node = m_active_node->parent;
            m_active_node->sub_nodes[m_loose_string] = std::make_shared<JSONNode>();
            m_active_node->sub_nodes[m_loose_string]->parent = m_active_node;
            m_active_node = m_active_node->sub_nodes[m_loose_string].get();
            m_active_node->name = m_loose_string;
            m_active_node->key_val_decl_line = m_line_nr;
            m_prev_decl = true;
        }

        m_loose_string = "";
    }


    void JSONParser::_HandleNextElementToken() {
        // check if there is a loose string to add into values
        if(m_loose_string != "") {
            m_active_node->values.push_back(std::make_pair(JSON_TYPE_STRING, m_loose_string));
            m_loose_string = "";
        }

        if(!m_active_node->is_array_open)
            m_prev_decl = false;
    }


    void JSONParser::_CheckTokenAction(JSONToken _token) {
        switch(_token) {
            case JSON_TOKEN_SCOPE_START:
                _HandleScopeStartToken();
                break;

            case JSON_TOKEN_SCOPE_END:
                _HandleScopeEndToken();
                break;

            case JSON_TOKEN_ARRAY_START:
                _HandleArrayStartToken();
                break;

            case JSON_TOKEN_ARRAY_END:
                _HandleArrayEndToken();
                break;

            case JSON_TOKEN_STRING_STATEMENT:
                _HandleStringToken();
                break;

            case JSON_TOKEN_NEXT_ELEMENT:
                _HandleNextElementToken();
                break;

            case JSON_TOKEN_BOOLEAN:
                _HandleBooleanToken();
                break;

            case JSON_TOKEN_NUMERICAL:
                _HandleNumericalToken();
                break;

            case JSON_TOKEN_KEY_VALUE_DECL:
                _HandleKeyValueDeclToken();
                break;

            default:
                break;
        }
    }


    void JSONParser::_SkipSkippableCharacters(bool _no_nl) {
        // no newlines will be skipped
        if(_no_nl) {
            // skips: whitespaces, tabs, carriage returns
            while(m_rd_ptr < m_buffer + m_last_read && (*m_rd_ptr == ' ' || *m_rd_ptr == '\t' || *m_rd_ptr == '\r'))
                m_rd_ptr++;
        } else {
            // skips: whitespaces, tabs, carriage returns, newlines
            while(m_rd_ptr < m_buffer + m_last_read && (*m_rd_ptr == '\n' ||
                  *m_rd_ptr == ' ' || *m_rd_ptr == '\t' || *m_rd_ptr == '\r')) {
                if(*m_rd_ptr == '\n') m_line_nr++;
                m_rd_ptr++;
            }
        }
    }


    void JSONParser::Parse(const std::string &_file_name) {
        if(_file_name != "") {
            m_file_name = _file_name;
            NewFile(m_file_name);
            _ReadNewChunk();
        }

        do {
            m_rd_ptr = m_buffer;
            while(m_rd_ptr < m_buffer + m_last_read && *m_rd_ptr != 0) {
                _SkipSkippableCharacters();
                JSONToken token = _CheckForToken();
                _CheckTokenAction(token);
                m_rd_ptr++;
            }
        } while(_ReadNewChunk());
    }


    JSONNode &JSONParser::GetRootNode() {
        return m_root_node;
    }
}
