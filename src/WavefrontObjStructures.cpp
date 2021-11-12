#define WAVEFRONT_OBJ_STRUCTURES_CPP
#include <WavefrontObjStructures.h>

namespace Libdas {

    namespace WavefrontObjFunctions {

        /************************************************/
        /*************** Custom functions ***************/
        /************************************************/
        
        void _ArgCountCheck(AsciiFormatErrorHandler &_error, const std::string &_keyword, int _line, uint32_t _arg_c, 
                            uint32_t _min_args, uint32_t _max_args, TerminationType _terminate) {
            if(_arg_c < _min_args)
                _error.SyntaxError(LIBDAS_ERROR_NOT_ENOUGH_ATTRIBUTES, _line, _keyword, "", _terminate);
            else if(_arg_c > _max_args)
                _error.SyntaxError(LIBDAS_ERROR_TOO_MANY_ATTRIBUTES, _line, _keyword, "", _terminate);

        }


        void _CheckFloatArgs(AsciiFormatErrorHandler &_error, float *_beg, float *_end, size_t _arg_offset, 
                             const std::string &_keyword, ArgsType &_args) {
            for(float *ptr = _beg; ptr < _end; ptr++) {
                // NaN detected, throw an error
                if(std::isnan(*ptr)) {
                    _error.SyntaxError(LIBDAS_ERROR_INVALID_ARGUMENT, _args.first, _keyword, 
                                       _args.second[_arg_offset + (ptr - _beg)], TERMINATE);
                }
            }
        }


        void _CheckIntArgs(AsciiFormatErrorHandler &_error, uint32_t *_beg, uint32_t *_end, const std::string &_arg, 
                           const std::string &_keyword, uint32_t _line) {
            for(uint32_t *ptr = _beg; ptr < _end; ptr++) {
                if(*ptr == 0) {
                    _error.SyntaxError(LIBDAS_ERROR_INVALID_ARGUMENT, _line, _keyword, _arg, TERMINATE);
                }
            }
        }


        Point3D<uint32_t> _ParseTripleIndexBlock(AsciiFormatErrorHandler &_error, const std::string &_block, 
                                                 const std::string &_keyword, const uint32_t _line) {
            // get all separator locations if they even exist
            size_t fsep = std::string::npos, ssep = std::string::npos;
            fsep = _block.find("/", 0);

            if(fsep != std::string::npos)
                ssep = _block.find("/", fsep + 1);

            Point3D<uint32_t> index_block(-1, -1, -1);
            std::string pos, tex, normal;

            // first seperator was found
            if(fsep == std::string::npos)
                index_block.x = static_cast<uint32_t>(std::atoi(_block.c_str()));
            else {
                index_block.x = static_cast<uint32_t>(std::atoi(_block.substr(0, fsep).c_str()));

                if(ssep != std::string::npos && ssep - fsep > 1)
                    index_block.y = static_cast<uint32_t>(std::atoi(_block.substr(fsep + 1, ssep - fsep).c_str()));

                // error if there is no index after second seperator
                if(ssep == _block.size() - 1)
                    _error.SyntaxError(LIBDAS_ERROR_INVALID_ARGUMENT, _line, _keyword, _block);

                else index_block.z = static_cast<uint32_t>(std::atoi(_block.substr(ssep + 1, _block.size() - ssep - 1).c_str()));
            }

            _CheckIntArgs(_error, &index_block.x, &index_block.z, _block, _keyword, _line);

            return index_block;
        }


        /***********************************************/
        /*************** Parser callback ***************/
        /***********************************************/

        void ParseVertexKeywordArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args) {
            const std::string keyword = "v";
            _ArgCountCheck(_error, keyword, _args.first, static_cast<uint32_t>(_args.second.size()), 3, 4, TERMINATE);
            Point4D<float> pt(0.0f, 0.0f, 0.0f, 1.0f);

            pt.x = std::stof(_args.second[0]);
            pt.y = std::stof(_args.second[1]);
            pt.z = std::stof(_args.second[2]);

            // w argument is present
            if(_args.second.size() == 4)
                pt.w = std::stof(_args.second[3]);

            _CheckFloatArgs(_error, &pt.x, &pt.w, 0, keyword, _args);
            _groups.back().vertices.position.push_back(pt);
        }


        void ParsePointKeywordArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args) {
            const std::string keyword = "vp";
            _ArgCountCheck(_error, keyword, _args.first, static_cast<uint32_t>(_args.second.size()), 1, 3, TERMINATE);
            Point3D<float> pt(0.0f, 0.0f, 0.0f);

            pt.x = std::stof(_args.second[0]);
            
            // at least v argument is present
            if(_args.second.size() > 1) {
                pt.y = std::stof(_args.second[1]);
                if(_args.second.size() == 3)
                    pt.z = std::stof(_args.second[2]);
            }

            _CheckFloatArgs(_error, &pt.x, &pt.y, 0, keyword, _args);
            _groups.back().vertices.points.push_back(pt);
        }


        void ParseVertexNormalKeywordArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args) {
            const std::string keyword = "vn";
            _ArgCountCheck(_error, keyword, _args.first, static_cast<uint32_t>(_args.second.size()), 3, 3, TERMINATE);
            Point3D<float> pt(0.0f, 0.0f, 0.0f);

            pt.x = std::stof(_args.second[0]);
            pt.y = std::stof(_args.second[1]);
            pt.z = std::stof(_args.second[2]);

            _CheckFloatArgs(_error, &pt.x, &pt.y, 0, keyword, _args);
            _groups.back().vertices.normals.push_back(pt);
        }


        void ParseTextureVertexKeywordArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args) {
            const std::string keyword = "vt";
            _ArgCountCheck(_error, keyword, _args.first, static_cast<uint32_t>(_args.second.size()), 1, 3, TERMINATE);
            Point3D<float> pt(0.0f, 0.0f, 0.0f);

            pt.x = std::stof(_args.second[0]);
            if(_args.second.size() > 1) {
                pt.y = std::stof(_args.second[1]);

                if(_args.second.size() == 3)
                    pt.z = std::stof(_args.second[2]);
            }

            _CheckFloatArgs(_error, &pt.x, &pt.y, 0, keyword, _args);
            _groups.back().vertices.texture.push_back(pt);
        };


        // curves and surfaces are not yet supported ! ! !
        //void ParseCSTypeArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void ParsePolynomialDegreeArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void ParseBasisMatrixArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void ParseCSStepArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        
        void ParsePointsArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args) {
            const std::string keyword = "p";
            _ArgCountCheck(_error, keyword, _args.first, static_cast<uint32_t>(_args.second.size()), 1, UINT32_MAX, TERMINATE);
            std::vector<uint32_t> verts;
            verts.reserve(_args.second.size());

            for(size_t i = 0; i < _args.second.size(); i++) {
                uint32_t index = static_cast<uint32_t>(std::atoi(_args.second[i].c_str()));
                if(!index)
                    _error.SyntaxError(LIBDAS_ERROR_INVALID_ARGUMENT, _args.first, keyword, _args.second[i], TERMINATE);
                
                verts.push_back(index);
            }

            _groups.back().indices.pt.push_back(verts);
        }

        //void ParseLineArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args) {
            //const std::string keyword = "l";
            //_ArgCountCheck(_error, keyword, _args.first, static_cast<uint32_t>(_args.second.size()), 3, UINT32_MAX, TERMINATE);
            //std::vector<uint32_t> verts;
            //verts.reserve(_args.second.size() * 2);
        //}


        void ParseFaceArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args) {
            const std::string keyword = "f";
            _ArgCountCheck(_error, keyword, _args.first, static_cast<uint32_t>(_args.second.size()), 1, UINT32_MAX, TERMINATE);
            WavefrontObjFace face;
            face.verts.reserve(_args.second.size());
            face.textures.reserve(_args.second.size());
            face.normals.reserve(_args.second.size());

            for(size_t i = 0; i < _args.second.size(); i++) {
                Point3D<uint32_t> elem = _ParseTripleIndexBlock(_error, _args.second[i], keyword, _args.first);
                face.verts.push_back(elem.x);
                face.textures.push_back(elem.y);
                face.normals.push_back(elem.z);
            }

            _groups.back().indices.faces.push_back(face);
        }

        //void ParseCurveArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void Parse2DCurveArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void ParseSurfaceArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void ParseParameterArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void ParseTrimmingCurveArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void ParseHoleCurveArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void ParseSpecialCurveArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void ParseSpecialPointArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        

        void ParseGroupArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args) {
            const std::string keyword = "g";
            _ArgCountCheck(_error, keyword, _args.first, static_cast<uint32_t>(_args.second.size()), 0, UINT32_MAX, TERMINATE);
            _groups.push(WavefrontObjGroup(_args.second));
        }


        //void ParseShadingGroup(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void ParseMergeGroup(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        
        
        void ParseObjectName(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args) {
            const std::string keyword = "o";
            _ArgCountCheck(_error, keyword, _args.first, static_cast<uint32_t>(_args.second.size()), 1, 1, TERMINATE);
            _groups.push(WavefrontObjGroup(_args.second));
        }


        //void ParseBevelInterpolationArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void ParseColorInterpolationArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void ParseDiffuseInterpolationArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void ParseLevelOfDetailArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void ParseUseMapArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void ParseMapLibraryArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void ParseUseMaterialArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void ParseMaterialLibraryArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void ParseShadowObjectArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void ParseRayTracingObjectArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void ParseCurveTechniqueArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void ParseSurfaceTechniqueArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
    }
}
