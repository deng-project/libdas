/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: WavefrontObjParser.cpp - Wavefront Obj parser source file
/// author: Karl-Mihkel Ott


#define WAVEFRONT_OBJ_PARSER_CPP
#include <WavefrontObjParser.h>

namespace Libdas {

    WavefrontObjParser::WavefrontObjParser(size_t _chunk_size) : 
        StreamReader(_chunk_size, '\n'), m_error(MODEL_FORMAT_WOBJ) {
        std::vector<std::string> names = {"main"};
        m_groups.push(WavefrontObjGroup(names));
        _Tokenize();

    }
    
    WavefrontObjParser::WavefrontObjParser(const std::string &_file_name, size_t _chunk_size) 
        : StreamReader(_chunk_size, _file_name, '\n'), m_file_name(_file_name), m_error(MODEL_FORMAT_WOBJ) {
        std::vector<std::string> names = {"main"};
        m_groups.push(WavefrontObjGroup(names));
        _Tokenize();
        Parse();
    }


    void WavefrontObjParser::_Tokenize() {
        m_statement_map["v"] = {
            WAVEFRONT_OBJ_STATEMENT_VERTEX,
            WavefrontObjFunctions::ParseVertexKeywordArgs
        };

        m_statement_map["vp"] = {
            WAVEFRONT_OBJ_STATEMENT_POINT,
            WavefrontObjFunctions::ParsePointKeywordArgs
        };

        m_statement_map["vn"] = {
            WAVEFRONT_OBJ_STATEMENT_VERTEX_NORMAL,
            WavefrontObjFunctions::ParseVertexNormalKeywordArgs
        };

        m_statement_map["vt"] = {
            WAVEFRONT_OBJ_STATEMENT_VERTEX_TEXTURE,
            WavefrontObjFunctions::ParseTextureVertexKeywordArgs
        };
        
        m_statement_map["cstype"] = {
            WAVEFRONT_OBJ_STATEMENT_CSTYPE,
            //WavefrontObjFunctions::ParseCSTypeArgs
            nullptr
        };

        m_statement_map["deg"] = {
            WAVEFRONT_OBJ_STATEMENT_POLYNOMIAL_DEGREE,
            //WavefrontObjFunctions::ParsePolynomialDegreeArgs
            nullptr
        };

        m_statement_map["bmat"] = {
            WAVEFRONT_OBJ_STATEMENT_BASIS_MATRIX,
            //WavefrontObjFunctions::ParseBasisMatrixArgs
            nullptr
        };

        m_statement_map["step"] = {
            WAVEFRONT_OBJ_STATEMENT_STEP,
            //WavefrontObjFunctions::ParseCSStepArgs
            nullptr
        };

        m_statement_map["p"] = {
            WAVEFRONT_OBJ_STATEMENT_POINTS,
            WavefrontObjFunctions::ParsePointsArgs
        };

        m_statement_map["l"] = {
            WAVEFRONT_OBJ_STATEMENT_LINE,
            //WavefrontObjFunctions::ParseLineArgs
            nullptr
        };

        m_statement_map["f"] = {
            WAVEFRONT_OBJ_STATEMENT_FACE,
            WavefrontObjFunctions::ParseFaceArgs
        };

        m_statement_map["curv"] = {
            WAVEFRONT_OBJ_STATEMENT_CURV,
            //WavefrontObjFunctions::ParseCurveArgs
            nullptr
        };

        m_statement_map["curv2d"] = {
            WAVEFRONT_OBJ_STATEMENT_CURV2D,
            //WavefrontObjFunctions::Parse2DCurveArgs
            nullptr
        };

        m_statement_map["surf"] = {
            WAVEFRONT_OBJ_STATEMENT_SURFACE,
            //WavefrontObjFunctions::ParseSurfaceArgs
            nullptr
        };

        m_statement_map["parm"] = {
            WAVEFRONT_OBJ_STATEMENT_PARAMETER,
            //WavefrontObjFunctions::ParseParameterArgs
            nullptr
        };

        m_statement_map["trim"] = {
            WAVEFRONT_OBJ_STATEMENT_TRIMMING_CURV,
            //WavefrontObjFunctions::ParseTrimmingCurveArgs
            nullptr
        };

        m_statement_map["hole"] = {
            WAVEFRONT_OBJ_STATEMENT_HOLE_CURV,
            //WavefrontObjFunctions::ParseHoleCurveArgs
            nullptr
        };

        m_statement_map["scrv"] = {
            WAVEFRONT_OBJ_STATEMENT_SPECIAL_CURV,
            //WavefrontObjFunctions::ParseSpecialCurveArgs
            nullptr
        };

        m_statement_map["sp"] = {
            WAVEFRONT_OBJ_STATEMENT_SPECIAL_POINT,
            //WavefrontObjFunctions::ParseSpecialPointArgs
            nullptr
        };

        m_statement_map["end"] = {
            WAVEFRONT_OBJ_STATEMENT_END,
            nullptr
        };

        m_statement_map["g"] = {
            WAVEFRONT_OBJ_STATEMENT_GROUP,
            WavefrontObjFunctions::ParseGroupArgs
        };

        m_statement_map["s"] = {
            WAVEFRONT_OBJ_STATEMENT_SHADING_GROUP,
            //WavefrontObjFunctions::ParseShadingGroup
            nullptr
        };

        m_statement_map["mg"] = {
            WAVEFRONT_OBJ_STATEMENT_MERGE_GROUP,
            //WavefrontObjFunctions::ParseMergeGroup
            nullptr
        };

        m_statement_map["o"] = {
            WAVEFRONT_OBJ_STATEMENT_OBJECT,
            WavefrontObjFunctions::ParseObjectName
        };

        m_statement_map["bevel"] = {
            WAVEFRONT_OBJ_STATEMENT_BEVEL_INTERPOLATION,
            WavefrontObjFunctions::ParseObjectName
        };

        m_statement_map["c_interp"] = {
            WAVEFRONT_OBJ_STATEMENT_COLOR_INTERPOLATION,
            //WavefrontObjFunctions::ParseColorInterpolationArgs
            nullptr
        };

        m_statement_map["d_interp"] = {
            WAVEFRONT_OBJ_STATEMENT_DIFFUSE_INTERPOLATION,
            //WavefrontObjFunctions::ParseDiffuseInterpolationArgs
            nullptr
        };

        m_statement_map["lod"] = {
            WAVEFRONT_OBJ_STATEMENT_LEVEL_OF_DETAIL,
            //WavefrontObjFunctions::ParseLevelOfDetailArgs
            nullptr
        };

        m_statement_map["usemap"] = {
            WAVEFRONT_OBJ_STATEMENT_USE_MAP,
            //WavefrontObjFunctions::ParseUseMapArgs
            nullptr
        };

        m_statement_map["maplib"] = {
            WAVEFRONT_OBJ_STATEMENT_MAP_LIBRARY,
            //WavefrontObjFunctions::ParseMapLibraryArgs
            nullptr
        };

        m_statement_map["usemtl"] = {
            WAVEFRONT_OBJ_STATEMENT_USE_MAP,
            //WavefrontObjFunctions::ParseUseMapArgs
            nullptr
        };

        m_statement_map["mtllib"] = {
            WAVEFRONT_OBJ_STATEMENT_MATERIAL_LIBRARY,
            //WavefrontObjFunctions::ParseMaterialLibraryArgs
            nullptr
        };

        m_statement_map["shadow_obj"] = {
            WAVEFRONT_OBJ_STATEMENT_SHADOW_OBJECT,
            //WavefrontObjFunctions::ParseShadowObjectArgs
            nullptr
        };

        m_statement_map["trace_obj"] = {
            WAVEFRONT_OBJ_STATEMENT_RAY_TRACING_OBJECT,
            //WavefrontObjFunctions::ParseRayTracingObjectArgs
            nullptr
        };

        m_statement_map["ctech"] = {
            WAVEFRONT_OBJ_STATEMENT_CURV_TECHNIQUE,
            //WavefrontObjFunctions::ParseCurveTechniqueArgs
            nullptr
        };

        m_statement_map["stech"] = {
            WAVEFRONT_OBJ_STATEMENT_SURFACE_TECHNIQUE,
            //WavefrontObjFunctions::ParseSurfaceTechniqueArgs
            nullptr
        };

        m_statement_map["#"] = {
            WAVEFRONT_OBJ_STATEMENT_COMMENT,
            nullptr
        };

    }


    bool WavefrontObjParser::_NextLine() {
        char *new_end = nullptr;

        // line ending is saved
        if(m_line_end != 0 && m_line_end + 1 < m_buffer + m_last_read) {
            new_end = std::strchr(m_line_end + 1, static_cast<int>('\n'));
            if(!new_end) new_end = m_buffer + m_last_read;
            m_line_beg = m_line_end + 1;
            m_line_end = new_end;
        }

        else if(m_line_end + 1 >= m_buffer + m_last_read)
            return false;

        else {
            // no line ending is saved
            m_line_beg = m_buffer;
            m_line_end = std::strchr(m_line_beg, static_cast<int>('\n'));
        }
        return true;
    }


    void WavefrontObjParser::_SkipSkippableCharacters(char *_end) {
        // skip all whitespaces till keyword is found
        for(; m_rd_ptr < m_line_end; m_rd_ptr++) {
            if(*m_rd_ptr != ' ' && *m_rd_ptr != 0x00 && *m_rd_ptr != '\t' && *m_rd_ptr != '\r')
                break;
        }
    }


    char *WavefrontObjParser::_ExtractWord() {
        char *end = m_rd_ptr;
        while(true) {
            if(*end == ' ' || *end == 0x00 || *end == '\t' || *end == '\n' || *end == '\r')
                break;

            end++;
        }

        return end;
    }


    WavefrontObjStatementReader WavefrontObjParser::_AnalyseKeyword(char *_end) {
        std::string key = std::string(m_rd_ptr, _end - m_rd_ptr);
        if(key == "") return WavefrontObjStatementReader();

        // verify that the key is valid
        auto res = m_statement_map.find(key);
        if(res == m_statement_map.end()) {
            m_error.SyntaxError(LIBDAS_ERROR_INVALID_KEYWORD, m_parse_pos, key, "", TERMINATE);
        }

        return m_statement_map[key];
    }


    std::vector<std::string> WavefrontObjParser::_ReadStatementArgs() {
        std::vector<std::string> args;
        while(true) {
            char *end = nullptr;
            _SkipSkippableCharacters(m_line_end);
            if(m_rd_ptr >= m_line_end) break;
            
            end = _ExtractWord();
            std::string arg = std::string(m_rd_ptr, end - m_rd_ptr);
            m_rd_ptr = end;
            if(arg == "\\\\") {
                _NextLine();
                m_rd_ptr = m_line_beg;
                continue;
            }

            args.push_back(arg);
        }

        return args;
    }


    void WavefrontObjParser::_AnalyseArgs(WavefrontObjStatementReader _reader, std::vector<std::string> &_args) {
        if(_reader.keyword_parser != nullptr) {
            auto arg_pair = std::make_pair(m_parse_pos, _args);
            _reader.keyword_parser(m_groups, m_error, arg_pair);
        }
        // tmp for testing purposes
        m_rd_ptr = m_line_end;
    }


    void WavefrontObjParser::Parse(const std::string &_file_name) {
        // check if new file was provided
        if(_file_name != m_file_name) {
            m_file_name = _file_name;
            OpenStream(m_file_name);
            if(!ReadNewChunk()) return;
        }

        Parse();
    }


    void WavefrontObjParser::Parse() {
        // chunk can be read
        do {
            // new line can be read
            while(_NextLine()) {
                m_rd_ptr = m_line_beg;
                // start by searching the first keyword
                _SkipSkippableCharacters(m_line_end);

                // line is empty skip iteration
                if(m_rd_ptr != m_line_end && m_rd_ptr != m_buffer + m_last_read) {
                    char *w_end = _ExtractWord();
                    WavefrontObjStatementReader statement_reader = _AnalyseKeyword(w_end);
                    m_rd_ptr = w_end + 1;

                    // Check if valid statement was extracted
                    if(statement_reader.type != WAVEFRONT_OBJ_STATEMENT_NONE) {
                        std::vector<std::string> args = _ReadStatementArgs();
                        _AnalyseArgs(statement_reader, args);
                    }
                }
                else if(m_rd_ptr == m_buffer + m_last_read)
                    break;

                m_parse_pos++;
            }

            m_line_beg = nullptr;
            m_line_end = nullptr;
        } while(ReadNewChunk());
    }


    WavefrontObjGroup WavefrontObjParser::PopFromGroupQueue() {
        WavefrontObjGroup group = std::move(m_groups.front());
        m_groups.pop();
        return group;
    }


    bool WavefrontObjParser::IsGroupQueueEmpty() {
        return m_groups.empty();
    }
}
