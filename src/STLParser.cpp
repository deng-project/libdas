/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: STLParser.cpp - STL parser class implementations
/// author: Karl-Mihkel Ott

#define STL_PARSER_CPP
#include <STLParser.h>


namespace Libdas {


    /***********************************************/
    /***** AsciiSTLParser class implementation *****/
    /***********************************************/

    AsciiSTLParser::AsciiSTLParser(const std::string &_file_name, size_t _chunk_size) : 
        AsciiLineReader(_file_name, _chunk_size, "\n"), m_error(MODEL_FORMAT_STLA)
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
        NewFile(_file_name);

        if(!_ReadNewChunk())
            m_error.Error(LIBDAS_ERROR_INVALID_FILE, 0, _file_name, "", TERMINATE);

        Parse();
    }


    void AsciiSTLParser::Parse() {
        // chunk can be read
        do {
            // line can be read
            while(_NextLine()) {
                _SetReadPtr(_GetLineBounds().first);
                // search the first keyword
                _SkipSkippableCharacters();
                char *rd = _GetReadPtr();

                // extract keyword
                char *end = _ExtractWord();
                std::string key = std::string(rd, end - rd);
                _SetReadPtr(end);
                rd = _GetReadPtr();
                AsciiSTLStatementCallback callback = _AnalyseKeyword(key);

                if(callback.type != ASCII_STL_STATEMENT_NONE) {
                    std::vector<std::string> args = _ReadStatementArgs();
                    _AnalyseStatement(callback, args);
                } else {
                    m_error.Error(LIBDAS_ERROR_INVALID_KEYWORD, m_parse_pos, key);
                }

                m_parse_pos++;
            }

            _SetLineBounds(std::make_pair(nullptr, nullptr));
        } while(_ReadNewChunk());
    }


    std::vector<STLObject> &AsciiSTLParser::GetObjects() {
        return m_objects;
    }


    /************************************************/
    /***** BinarySTLParser class implementation *****/
    /************************************************/

    BinarySTLParser::BinarySTLParser(const std::string &_file_name) : 
        m_error(MODEL_FORMAT_STLB)
    {
        // check if appropriate file name was provided
        if(_file_name != "") {
            m_file_name = _file_name;
            _OpenFileStream();
        }
    }


    BinarySTLParser::~BinarySTLParser() {
        // close the file stream if it is open
        if(m_stream.is_open())
            m_stream.close();
    }


    void BinarySTLParser::_OpenFileStream() {
        // close the stream if previously opened
        if(m_stream.is_open())
            m_stream.close();

        m_stream.open(m_file_name, std::ios_base::binary);
        if(m_stream.fail())
            m_error.Error(LIBDAS_ERROR_INVALID_FILE);
    }


    void BinarySTLParser::_LoadHeader() {
        m_stream.read(reinterpret_cast<char*>(&m_header), sizeof(BinarySTLHeader));
        m_object.name = _ExtractModelName();
    }


    std::string BinarySTLParser::_ExtractModelName() {
        std::string name = "Binary STL object";
        for(uint32_t i = 0; i < 80; i++) {
            if(m_header.signature[i] == 0x00)
                name = std::string(m_header.signature, i + 1);
        }

        return name;
    }


    void BinarySTLParser::Parse(const std::string &_file_name) {
        // check if new file name was specified
        if(_file_name != "") {
            m_file_name = _file_name;
            _OpenFileStream();
        }
        
        _LoadHeader();

        // allocate enough memory for facets
        m_object.facets.resize(m_header.facet_c);
        
        // read all vertices blocks
        for(uint32_t i = 0; i < m_header.facet_c; i++) {
            // read normals
            m_stream.read(reinterpret_cast<char*>(&m_object.facets[i].normal), sizeof(Point3D<float>));

            // read vertices
            m_stream.read(reinterpret_cast<char*>(m_object.facets[i].vertices.data()), 3 * sizeof(Point3D<float>));

            // skip atribute byte count
            std::size_t pos = m_stream.tellg();
            m_stream.seekg(pos + 2, std::ios_base::beg);

            // error check
            if(m_stream.fail()) m_error.Error(LIBDAS_ERROR_INVALID_DATA);
        }
    }


    STLObject BinarySTLParser::GetObject() {
        return m_object;
    }
}
