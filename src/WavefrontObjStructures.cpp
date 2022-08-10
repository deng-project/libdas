// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: WavefrontObjStructures.cpp - Wavefront Obj parsing functions and structures implementation
// author: Karl-Mihkel Ott

#define WAVEFRONT_OBJ_STRUCTURES_CPP
#include "das/WavefrontObjStructures.h"

namespace Libdas {

    namespace WavefrontObjFunctions {

        /************************************************/
        /*************** Custom functions ***************/
        /************************************************/

        TRS::Point3D<uint32_t> _TripleIndexBlockCallback(AsciiFormatErrorHandler &_error, const std::string &_block, 
                                                    const std::string &_keyword, const uint32_t _line) {
            // get all separator locations if they even exist
            size_t fsep = std::string::npos, ssep = std::string::npos;
            fsep = _block.find("/", 0);

            if(fsep != std::string::npos) {
                ssep = _block.find("/", fsep + 1);
            }

            TRS::Point3D<uint32_t> index_block(UINT32_MAX, UINT32_MAX, UINT32_MAX);
            std::string pos, tex, normal;

            // first seperator was not found
            if(fsep == std::string::npos)
                index_block.x = static_cast<uint32_t>(std::atoi(_block.c_str()));
            else {
                index_block.x = static_cast<uint32_t>(std::atoi(_block.substr(0, fsep).c_str()));

                if(ssep != std::string::npos && ssep - fsep > 1)
                    index_block.y = static_cast<uint32_t>(std::atoi(_block.substr(fsep + 1, ssep - fsep).c_str()));
                else if(ssep == std::string::npos)
                    index_block.y = static_cast<uint32_t>(std::atoi(_block.substr(fsep + 1).c_str()));

                // error if there is no index after second seperator
                if(ssep == _block.size() - 1)
                    _error.Error(LIBDAS_ERROR_INVALID_ARGUMENT, _line, _keyword, _block);
                else if(ssep != std::string::npos) 
                    index_block.z = static_cast<uint32_t>(std::atoi(_block.substr(ssep + 1, _block.size() - ssep - 1).c_str()));
            }

            _error.CheckIntArgs(&index_block.x, &index_block.z, _block, _keyword, _line);

            return index_block;
        }


        /***********************************************/
        /*************** Parser callback ***************/
        /***********************************************/

        // NOTE: Only x, y and z coordinates are allowed
        // w coordinates will be simply ignored
        void VertexKeywordArgsCallback(WavefrontObjData &_wobj_data, AsciiFormatErrorHandler &_error, ArgsType &_args) {
            const std::string keyword = "v";
            _error.ArgCountCheck(keyword, _args.first, static_cast<uint32_t>(_args.second.size()), 3, 4, TERMINATE);
            TRS::Point3D<float> pt(0.0f, 0.0f, 0.0f);

            pt.x = std::stof(_args.second[0]);
            pt.y = std::stof(_args.second[1]);
            pt.z = std::stof(_args.second[2]);

            _error.CheckFloatArgs(&pt.x, &pt.z, 0, keyword, _args);
            _wobj_data.vertices.position.push_back(pt);
        }


        void PointKeywordArgsCallback(WavefrontObjData &_wobj_data, AsciiFormatErrorHandler &_error, ArgsType &_args) {
            const std::string keyword = "vp";
            _error.ArgCountCheck(keyword, _args.first, static_cast<uint32_t>(_args.second.size()), 1, 3, TERMINATE);
            TRS::Point3D<float> pt(0.0f, 0.0f, 0.0f);

            pt.x = std::stof(_args.second[0]);
            
            // at least v argument is present
            if(_args.second.size() > 1) {
                pt.y = std::stof(_args.second[1]);
                if(_args.second.size() == 3)
                    pt.z = std::stof(_args.second[2]);
            }

            _error.CheckFloatArgs(&pt.x, &pt.y, 0, keyword, _args);
            _wobj_data.vertices.points.push_back(pt);
        }


        void VertexNormalKeywordArgsCallback(WavefrontObjData &_wobj_data, AsciiFormatErrorHandler &_error, ArgsType &_args) {
            const std::string keyword = "vn";
            _error.ArgCountCheck(keyword, _args.first, static_cast<uint32_t>(_args.second.size()), 3, 3, TERMINATE);
            TRS::Point3D<float> pt(0.0f, 0.0f, 0.0f);

            pt.x = std::stof(_args.second[0]);
            pt.y = std::stof(_args.second[1]);
            pt.z = std::stof(_args.second[2]);

            _error.CheckFloatArgs(&pt.x, &pt.y, 0, keyword, _args);
            _wobj_data.vertices.normals.push_back(pt);
        }


        void TextureVertexKeywordArgsCallback(WavefrontObjData &_wobj_data, AsciiFormatErrorHandler &_error, ArgsType &_args) {
            const std::string keyword = "vt";
            _error.ArgCountCheck(keyword, _args.first, static_cast<uint32_t>(_args.second.size()), 1, 3, TERMINATE);
            TRS::Point2D<float> pt(0.0f, 0.0f);

            pt.x = std::stof(_args.second[0]);
            if(_args.second.size() > 1) {
                pt.y = std::stof(_args.second[1]);
            }

            _error.CheckFloatArgs(&pt.x, &pt.y, 0, keyword, _args);
            _wobj_data.vertices.texture.push_back(pt);
        };


        // curves and surfaces are not yet supported ! ! !
        //void CSTypeArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void PolynomialDegreeArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void BasisTRS::MatrixArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void CSStepArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        
        void PointsArgsCallback(WavefrontObjData &_wobj_data, AsciiFormatErrorHandler &_error, ArgsType &_args) {
            const std::string keyword = "p";
            _error.ArgCountCheck(keyword, _args.first, static_cast<uint32_t>(_args.second.size()), 1, UINT32_MAX, TERMINATE);
            std::vector<uint32_t> verts;
            verts.reserve(_args.second.size());

            for(size_t i = 0; i < _args.second.size(); i++) {
                uint32_t index = static_cast<uint32_t>(std::atoi(_args.second[i].c_str()));
                if(!index)
                    _error.Error(LIBDAS_ERROR_INVALID_ARGUMENT, _args.first, keyword, _args.second[i], TERMINATE);
                
                verts.push_back(index);
            }

            _wobj_data.groups.back().indices.pt.push_back(verts);
        }


        //void LineArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args) {
            //const std::string keyword = "l";
            //_ArgCountCheck(_error, keyword, _args.first, static_cast<uint32_t>(_args.second.size()), 3, UINT32_MAX, TERMINATE);
            //std::vector<uint32_t> verts;
            //verts.reserve(_args.second.size() * 2);
        //}


        void FaceArgsCallback(WavefrontObjData &_wobj_data, AsciiFormatErrorHandler &_error, ArgsType &_args) {
            const std::string keyword = "f";
            _error.ArgCountCheck(keyword, _args.first, static_cast<uint32_t>(_args.second.size()), 1, UINT32_MAX, TERMINATE);
            _wobj_data.groups.back().indices.faces.emplace_back(_args.second.size());
            auto &face  = _wobj_data.groups.back().indices.faces.back();

            for(size_t i = 0; i < _args.second.size(); i++) {
                TRS::Point3D<uint32_t> elem = _TripleIndexBlockCallback(_error, _args.second[i], keyword, _args.first);

                if(elem.x != UINT32_MAX)
                    face[i].vert = elem.x - 1;

                if(elem.y != UINT32_MAX)
                    face[i].texture = elem.y - 1;

                if(elem.z != UINT32_MAX)
                    face[i].normal = elem.z - 1;
            }
        }

        //void CurveArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void 2DCurveArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void SurfaceArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void ParameterArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void TrimmingCurveArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void HoleCurveArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void SpecialCurveArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void SpecialTRS::PointArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        

        void GroupArgsCallback(WavefrontObjData &_wobj_data, AsciiFormatErrorHandler &_error, ArgsType &_args) {
            const std::string keyword = "g";
            _error.ArgCountCheck(keyword, _args.first, static_cast<uint32_t>(_args.second.size()), 0, UINT32_MAX, TERMINATE);
            _wobj_data.groups.emplace_back(_args.second);
        }


        //void ShadingGroupCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void MergeGroupCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        
        
        void ObjectNameCallback(WavefrontObjData &_wobj_data, AsciiFormatErrorHandler &_error, ArgsType &_args) {
            const std::string keyword = "o";
            _error.ArgCountCheck(keyword, _args.first, static_cast<uint32_t>(_args.second.size()), 1, 1, TERMINATE);
            _wobj_data.groups.emplace_back(_args.second);
        }


        //void BevelInterpolationArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void ColorInterpolationArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void DiffuseInterpolationArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void LevelOfDetailArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void UseMapArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void MapLibraryArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void UseMaterialArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void MaterialLibraryArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void ShadowObjectArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void RayTracingObjectArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void CurveTechniqueArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void SurfaceTechniqueArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
    }
}
