// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: WavefrontObjParser.cpp - Wavefront Obj parser source file
// author: Karl-Mihkel Ott


#define WAVEFRONT_OBJ_PARSER_CPP
#include "das/WavefrontObjParser.h"

namespace Libdas {

    WavefrontObjParser::WavefrontObjParser(const std::string &_file_name, size_t _chunk_size) : 
        MAR::AsciiLineReader(_file_name, _chunk_size, "\n"), m_file_name(_file_name), m_error(MODEL_FORMAT_WOBJ) {
        std::vector<std::string> names = {"main"};
        m_parsed_data.groups.emplace_back(std::vector<std::string>{ "default" });
        _Tokenize();
    }
    

    void WavefrontObjParser::_Tokenize() {
        m_statement_map["v"] = {
            WAVEFRONT_OBJ_STATEMENT_VERTEX,
            WavefrontObjFunctions::VertexKeywordArgsCallback
        };

        m_statement_map["vp"] = {
            WAVEFRONT_OBJ_STATEMENT_POINT,
            WavefrontObjFunctions::PointKeywordArgsCallback
        };

        m_statement_map["vn"] = {
            WAVEFRONT_OBJ_STATEMENT_VERTEX_NORMAL,
            WavefrontObjFunctions::VertexNormalKeywordArgsCallback
        };

        m_statement_map["vt"] = {
            WAVEFRONT_OBJ_STATEMENT_VERTEX_TEXTURE,
            WavefrontObjFunctions::TextureVertexKeywordArgsCallback
        };
        
        m_statement_map["cstype"] = {
            WAVEFRONT_OBJ_STATEMENT_CSTYPE,
            nullptr
        };

        m_statement_map["deg"] = {
            WAVEFRONT_OBJ_STATEMENT_POLYNOMIAL_DEGREE,
            nullptr
        };

        m_statement_map["bmat"] = {
            WAVEFRONT_OBJ_STATEMENT_BASIS_MATRIX,
            nullptr
        };

        m_statement_map["step"] = {
            WAVEFRONT_OBJ_STATEMENT_STEP,
            nullptr
        };

        m_statement_map["p"] = {
            WAVEFRONT_OBJ_STATEMENT_POINTS,
            WavefrontObjFunctions::PointsArgsCallback
        };

        m_statement_map["l"] = {
            WAVEFRONT_OBJ_STATEMENT_LINE,
            nullptr
        };

        m_statement_map["f"] = {
            WAVEFRONT_OBJ_STATEMENT_FACE,
            WavefrontObjFunctions::FaceArgsCallback
        };

        m_statement_map["curv"] = {
            WAVEFRONT_OBJ_STATEMENT_CURV,
            nullptr
        };

        m_statement_map["curv2d"] = {
            WAVEFRONT_OBJ_STATEMENT_CURV2D,
            nullptr
        };

        m_statement_map["surf"] = {
            WAVEFRONT_OBJ_STATEMENT_SURFACE,
            nullptr
        };

        m_statement_map["parm"] = {
            WAVEFRONT_OBJ_STATEMENT_PARAMETER,
            nullptr
        };

        m_statement_map["trim"] = {
            WAVEFRONT_OBJ_STATEMENT_TRIMMING_CURV,
            //WavefrontObjFunctions::TrimmingCurveArgsCallback
            nullptr
        };

        m_statement_map["hole"] = {
            WAVEFRONT_OBJ_STATEMENT_HOLE_CURV,
            //WavefrontObjFunctions::HoleCurveArgsCallback
            nullptr
        };

        m_statement_map["scrv"] = {
            WAVEFRONT_OBJ_STATEMENT_SPECIAL_CURV,
            //WavefrontObjFunctions::SpecialCurveArgsCallback
            nullptr
        };

        m_statement_map["sp"] = {
            WAVEFRONT_OBJ_STATEMENT_SPECIAL_POINT,
            //WavefrontObjFunctions::SpecialPointArgsCallback
            nullptr
        };

        m_statement_map["end"] = {
            WAVEFRONT_OBJ_STATEMENT_END,
            nullptr
        };

        m_statement_map["g"] = {
            WAVEFRONT_OBJ_STATEMENT_GROUP,
            WavefrontObjFunctions::GroupArgsCallback
        };

        m_statement_map["s"] = {
            WAVEFRONT_OBJ_STATEMENT_SHADING_GROUP,
            //WavefrontObjFunctions::ShadingGroupCallback
            nullptr
        };

        m_statement_map["mg"] = {
            WAVEFRONT_OBJ_STATEMENT_MERGE_GROUP,
            //WavefrontObjFunctions::MergeGroupCallback
            nullptr
        };

        m_statement_map["o"] = {
            WAVEFRONT_OBJ_STATEMENT_OBJECT,
            WavefrontObjFunctions::ObjectNameCallback
        };

        m_statement_map["bevel"] = {
            WAVEFRONT_OBJ_STATEMENT_BEVEL_INTERPOLATION,
            WavefrontObjFunctions::ObjectNameCallback
        };

        m_statement_map["c_interp"] = {
            WAVEFRONT_OBJ_STATEMENT_COLOR_INTERPOLATION,
            //WavefrontObjFunctions::ColorInterpolationArgsCallback
            nullptr
        };

        m_statement_map["d_interp"] = {
            WAVEFRONT_OBJ_STATEMENT_DIFFUSE_INTERPOLATION,
            //WavefrontObjFunctions::DiffuseInterpolationArgsCallback
            nullptr
        };

        m_statement_map["lod"] = {
            WAVEFRONT_OBJ_STATEMENT_LEVEL_OF_DETAIL,
            //WavefrontObjFunctions::LevelOfDetailArgsCallback
            nullptr
        };

        m_statement_map["usemap"] = {
            WAVEFRONT_OBJ_STATEMENT_USE_MAP,
            //WavefrontObjFunctions::UseMapArgsCallback
            nullptr
        };

        m_statement_map["maplib"] = {
            WAVEFRONT_OBJ_STATEMENT_MAP_LIBRARY,
            //WavefrontObjFunctions::MapLibraryArgsCallback
            nullptr
        };

        m_statement_map["usemtl"] = {
            WAVEFRONT_OBJ_STATEMENT_USE_MAP,
            //WavefrontObjFunctions::UseMapArgsCallback
            nullptr
        };

        m_statement_map["mtllib"] = {
            WAVEFRONT_OBJ_STATEMENT_MATERIAL_LIBRARY,
            //WavefrontObjFunctions::MaterialLibraryArgsCallback
            nullptr
        };

        m_statement_map["shadow_obj"] = {
            WAVEFRONT_OBJ_STATEMENT_SHADOW_OBJECT,
            //WavefrontObjFunctions::ShadowObjectArgsCallback
            nullptr
        };

        m_statement_map["trace_obj"] = {
            WAVEFRONT_OBJ_STATEMENT_RAY_TRACING_OBJECT,
            //WavefrontObjFunctions::RayTracingObjectArgsCallback
            nullptr
        };

        m_statement_map["ctech"] = {
            WAVEFRONT_OBJ_STATEMENT_CURV_TECHNIQUE,
            //WavefrontObjFunctions::CurveTechniqueArgsCallback
            nullptr
        };

        m_statement_map["stech"] = {
            WAVEFRONT_OBJ_STATEMENT_SURFACE_TECHNIQUE,
            //WavefrontObjFunctions::SurfaceTechniqueArgsCallback
            nullptr
        };
    }


    WavefrontObjStatementCallback WavefrontObjParser::_AnalyseKeyword(char *_end) {
        char *rd = _GetReadPtr();
        std::string key = std::string(rd, _end - rd);
        if(key == "") return WavefrontObjStatementCallback();

        // check if keyword starts with comment statement
        if(key[0] == '#')
            return { WAVEFRONT_OBJ_STATEMENT_COMMENT, nullptr };

        // verify that the key is valid
        auto res = m_statement_map.find(key);
        if(res == m_statement_map.end()) {
            m_error.Error(LIBDAS_ERROR_INVALID_KEYWORD, m_parse_pos, key, "", TERMINATE);
        }

        return m_statement_map[key];
    }


    void WavefrontObjParser::_AnalyseArgs(WavefrontObjStatementCallback _callback, std::vector<std::string> &_args) {
        if(_callback.keyword_callback != nullptr) {
            auto arg_pair = std::make_pair(m_parse_pos, _args);
            _callback.keyword_callback(m_parsed_data, m_error, arg_pair);
        }

        _SetReadPtr(_GetLineBounds().second);
    }


    void WavefrontObjParser::Parse(const std::string &_file_name) {
        // check if new file was provided
        if(_file_name != m_file_name) {
            m_file_name = _file_name;
            NewFile(m_file_name);
            if(!_ReadNewChunk()) return;
        }

        Parse();
    }


    void WavefrontObjParser::Parse() {
        // chunk can be read
        do {
            // new line can be read
            while(_NextLine()) {
                std::pair<char*, char*> bounds = _GetLineBounds();
                _SetReadPtr(bounds.first);
                char *rd = _GetReadPtr();

                // start by searching the first keyword
                _SkipSkippableCharacters();

                // line is empty skip iteration
                if(rd != bounds.second && rd != m_buffer + m_last_read) {
                    char *w_end = _ExtractWord();
                    WavefrontObjStatementCallback statement_reader = _AnalyseKeyword(w_end);
                    _SetReadPtr(w_end);
                    rd = _GetReadPtr();

                    // Check if valid statement was extracted
                    if(statement_reader.type != WAVEFRONT_OBJ_STATEMENT_NONE) {
                        std::vector<std::string> args = _ReadStatementArgs();
                        if(statement_reader.type == WAVEFRONT_OBJ_STATEMENT_COMMENT)
                            continue;

                        _AnalyseArgs(statement_reader, args);
                    }
                }
                else if(rd == m_buffer + m_last_read)
                    break;

                m_parse_pos++;
            }

            _SetLineBounds(std::make_pair(nullptr, nullptr));
        } while(_ReadNewChunk());

    }
}
