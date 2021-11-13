/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: STLParser.cpp - STL parser class implementation
/// author: Karl-Mihkel Ott

#define STL_PARSER_CPP
#include <STLParser.h>


namespace Libdas {

    AsciiSTLParser::AsciiSTLParser(const std::string &_file_name, size_t _chunk_size) : 
        AsciiLineReader(_chunk_size, "\n", _file_name), m_error(MODEL_FORMAT_STL)
    {
        _Tokenise();
    }


    void AsciiSTLParser::_Tokenise() {
        m_tokens["solid"] = {
            ASCII_STL_STATEMENT_SOLID,
            STLFunctions::_SolidCallback
        };

        m_tokens["facet"] = {
            ASCII_STL_STATEMENT_FACET,
            STLFunctions::_FacetCallback
        };

        m_tokens["outer"] = {
            ASCII_STL_STATEMENT_OUTER_LOOP,
            STLFunctions::_OuterLoopCallback
        };

        m_tokens["vertex"] = {
            ASCII_STL_STATEMENT_VERTEX,
            STLFunctions::_VertexCallback
        };

        m_tokens["endloop"] = {
            ASCII_STL_STATEMENT_ENDLOOP,
            STLFunctions::_EndLoopCallback
        };

        m_tokens["endfacet"] = {
            ASCII_STL_STATEMENT_ENDFACET,
            STLFunctions::_EndFacetCallback
        };

        m_tokens["endsolid"] = {
            ASCII_STL_STATEMENT_ENDSOLID,
            STLFunctions::_EndSolidCallback
        };

    }


    AsciiSTLStatementCallback AsciiSTLParser::_AnalyseKeyword(const std::string &_key) {
        // key is empty string
        if(_key == "") return AsciiSTLStatementCallback();
        // no such key is present, throw an error
        auto callback = m_tokens.find(_key);
        if(callback == m_tokens.end()) {
            m_error.Error(LIBDAS_ERROR_INVALID_KEYWORD, m_parse_pos, _key);
        }

        return m_tokens[_key];
    }


    void AsciiSTLParser::_AnalyseStatement(AsciiSTLStatementCallback &_callback, std::vector<std::string> &_args) {
        if(_callback.keyword_callback) {
            auto arg_pair = std::make_pair(m_parse_pos, _args);
            void *custom = _GetCustomData(_callback.type);
            _callback.keyword_callback(m_objects, m_error, arg_pair, custom);
        }
    }


    void *AsciiSTLParser::_GetCustomData(AsciiSTLStatementType _type) {
        switch(_type) {
            case ASCII_STL_STATEMENT_SOLID:
            case ASCII_STL_STATEMENT_ENDSOLID:
                return reinterpret_cast<void*>(&m_is_solid_obj);

            case ASCII_STL_STATEMENT_FACET:
            case ASCII_STL_STATEMENT_ENDFACET:
                return reinterpret_cast<void*>(&m_is_facet);

            case ASCII_STL_STATEMENT_OUTER_LOOP:
            case ASCII_STL_STATEMENT_ENDLOOP:
                return reinterpret_cast<void*>(&m_is_loop);

            default: return nullptr;
        }
    }

    
    void AsciiSTLParser::Parse(const std::string &_file_name) {
        LIBDAS_ASSERT(_file_name == "");
        OpenStream(_file_name);

        if(!ReadNewChunk())
            m_error.Error(LIBDAS_ERROR_INVALID_FILE, 0, _file_name, "", TERMINATE);

        Parse();
    }


    void AsciiSTLParser::Parse() {
        // chunk can be read
        do {
            // line can be read
            while(_NextLine()) {
                m_rd_ptr = m_line_beg;
                // search the first keyword
                _SkipSkippableCharacters();

                // extract keyword
                char *end = _ExtractWord();
                std::string key = std::string(m_rd_ptr, end - m_rd_ptr);
                m_rd_ptr = end;
                AsciiSTLStatementCallback callback = _AnalyseKeyword(key);

                if(callback.type != ASCII_STL_STATEMENT_NONE) {
                    std::vector<std::string> args = _ReadStatementArgs();
                    _AnalyseStatement(callback, args);
                } else {
                    m_error.Error(LIBDAS_ERROR_INVALID_KEYWORD, m_parse_pos, key);
                }

                m_parse_pos++;
            }
        } while(ReadNewChunk());
    }


    STLObject AsciiSTLParser::PopSTLObjectFromQueue() {
        STLObject obj = std::move(m_objects.front());
        m_objects.pop();

        return obj;
    }


    bool AsciiSTLParser::IsObjectQueueEmpty() {
        return m_objects.empty();
    }
}
