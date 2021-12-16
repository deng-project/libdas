// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: JSONParser.cpp - JSON parsing class implementation
// author: Karl-Mihkel Ott

#define JSON_PARSER_CPP
#include <JSONParser.h>

namespace Libdas {
    
    JSONParser::JSONParser(const std::string &_file_name) : 
        AsciiStreamReader(_file_name, DEFAULT_CHUNK, "}"), m_error(MODEL_FORMAT_GLTF), m_file_name(_file_name) {}


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
            case ';': return JSON_TOKEN_STATEMENT_END;
            case '\n': 
                m_line_nr++;
                break;
            default: break;
        }

        if(*m_rd_ptr >= '0' && *m_rd_ptr <= '9')
            return JSON_TOKEN_NUMERICAL;

        return JSON_TOKEN_UNKNOWN;
    }


    void JSONParser::_HandleScopeStartToken() {
        // error: scope was previously opened but new scope opening requested
        if(m_active_node->is_scope_open)
            m_error.Error(LIBDAS_ERROR_INCOMPLETE_SCOPE, m_line_nr);
        m_active_node->is_scope_open = true;
    }


    void JSONParser::_HandleScopeEndToken() {
        // error: scope was previously closed, but new scope closure requested
        if(!m_active_node->is_scope_open)
            m_error.Error(LIBDAS_ERROR_INCOMPLETE_SCOPE, m_line_nr);
        m_active_node->is_scope_open = false;
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

            case JSON_TOKEN_STATEMENT_END:
                _HandleStatementEndToken(); // might be unnecessary
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
            while(m_rd_ptr < m_buffer + m_buffer_size && (*m_rd_ptr == ' ' || *m_rd_ptr == '\t' || *m_rd_ptr == '\r'))
                m_rd_ptr++;
        } else {
            // skips: whitespaces, tabs, carriage returns, newlines
            while(m_rd_ptr < m_buffer + m_buffer_size && 
                  (*m_rd_ptr == ' ' || *m_rd_ptr == '\t' || *m_rd_ptr == '\r')) {
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
            while(m_rd_ptr < m_buffer + m_buffer_size && *m_rd_ptr != 0) {
                _SkipSkippableCharacters();
                JSONToken token = _CheckForToken();

                m_rd_ptr++;
            }
        } while(_ReadNewChunk());
    }


    JSONNode &JSONParser::GetRootNode() {
        return m_root_node;
    }
}
