/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: WavefrontObjStructures.cpp - Wavefront Obj parsing functions and structures implementation
/// author: Karl-Mihkel Ott

#define WAVEFRONT_OBJ_STRUCTURES_CPP
#include <WavefrontObjStructures.h>

namespace Libdas {

    namespace WavefrontObjFunctions {

        /************************************************/
        /*************** Custom functions ***************/
        /************************************************/

        Point3D<uint32_t> _TripleIndexBlockCallback(AsciiFormatErrorHandler &_error, const std::string &_block, 
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
                    _error.Error(LIBDAS_ERROR_INVALID_ARGUMENT, _line, _keyword, _block);

                else index_block.z = static_cast<uint32_t>(std::atoi(_block.substr(ssep + 1, _block.size() - ssep - 1).c_str()));
            }

            _error.CheckIntArgs(&index_block.x, &index_block.z, _block, _keyword, _line);

            return index_block;
        }


        /***********************************************/
        /*************** Parser callback ***************/
        /***********************************************/

        void VertexKeywordArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args) {
            const std::string keyword = "v";
            _error.ArgCountCheck(keyword, _args.first, static_cast<uint32_t>(_args.second.size()), 3, 4, TERMINATE);
            Point4D<float> pt(0.0f, 0.0f, 0.0f, 1.0f);

            pt.x = std::stof(_args.second[0]);
            pt.y = std::stof(_args.second[1]);
            pt.z = std::stof(_args.second[2]);

            // w argument is present
            if(_args.second.size() == 4)
                pt.w = std::stof(_args.second[3]);

            _error.CheckFloatArgs(&pt.x, &pt.w, 0, keyword, _args);
            _groups.back().vertices.position.push_back(pt);
        }


        void PointKeywordArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args) {
            const std::string keyword = "vp";
            _error.ArgCountCheck(keyword, _args.first, static_cast<uint32_t>(_args.second.size()), 1, 3, TERMINATE);
            Point3D<float> pt(0.0f, 0.0f, 0.0f);

            pt.x = std::stof(_args.second[0]);
            
            // at least v argument is present
            if(_args.second.size() > 1) {
                pt.y = std::stof(_args.second[1]);
                if(_args.second.size() == 3)
                    pt.z = std::stof(_args.second[2]);
            }

            _error.CheckFloatArgs(&pt.x, &pt.y, 0, keyword, _args);
            _groups.back().vertices.points.push_back(pt);
        }


        void VertexNormalKeywordArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args) {
            const std::string keyword = "vn";
            _error.ArgCountCheck(keyword, _args.first, static_cast<uint32_t>(_args.second.size()), 3, 3, TERMINATE);
            Point3D<float> pt(0.0f, 0.0f, 0.0f);

            pt.x = std::stof(_args.second[0]);
            pt.y = std::stof(_args.second[1]);
            pt.z = std::stof(_args.second[2]);

            _error.CheckFloatArgs(&pt.x, &pt.y, 0, keyword, _args);
            _groups.back().vertices.normals.push_back(pt);
        }


        void TextureVertexKeywordArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args) {
            const std::string keyword = "vt";
            _error.ArgCountCheck(keyword, _args.first, static_cast<uint32_t>(_args.second.size()), 1, 3, TERMINATE);
            Point3D<float> pt(0.0f, 0.0f, 0.0f);

            pt.x = std::stof(_args.second[0]);
            if(_args.second.size() > 1) {
                pt.y = std::stof(_args.second[1]);

                if(_args.second.size() == 3)
                    pt.z = std::stof(_args.second[2]);
            }

            _error.CheckFloatArgs(&pt.x, &pt.y, 0, keyword, _args);
            _groups.back().vertices.texture.push_back(pt);
        };


        // curves and surfaces are not yet supported ! ! !
        //void CSTypeArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void PolynomialDegreeArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void BasisMatrixArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void CSStepArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        
        void PointsArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args) {
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

            _groups.back().indices.pt.push_back(verts);
        }

        //void LineArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args) {
            //const std::string keyword = "l";
            //_ArgCountCheck(_error, keyword, _args.first, static_cast<uint32_t>(_args.second.size()), 3, UINT32_MAX, TERMINATE);
            //std::vector<uint32_t> verts;
            //verts.reserve(_args.second.size() * 2);
        //}


        void FaceArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args) {
            const std::string keyword = "f";
            _error.ArgCountCheck(keyword, _args.first, static_cast<uint32_t>(_args.second.size()), 1, UINT32_MAX, TERMINATE);
            WavefrontObjFace face;
            face.verts.reserve(_args.second.size());
            face.textures.reserve(_args.second.size());
            face.normals.reserve(_args.second.size());

            for(size_t i = 0; i < _args.second.size(); i++) {
                Point3D<uint32_t> elem = _TripleIndexBlockCallback(_error, _args.second[i], keyword, _args.first);
                face.verts.push_back(elem.x);
                face.textures.push_back(elem.y);
                face.normals.push_back(elem.z);
            }

            _groups.back().indices.faces.push_back(face);
        }

        //void CurveArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void 2DCurveArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void SurfaceArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void ParameterArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void TrimmingCurveArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void HoleCurveArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void SpecialCurveArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void SpecialPointArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        

        void GroupArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args) {
            const std::string keyword = "g";
            _error.ArgCountCheck(keyword, _args.first, static_cast<uint32_t>(_args.second.size()), 0, UINT32_MAX, TERMINATE);
            _groups.push(WavefrontObjGroup(_args.second));
        }


        //void ShadingGroupCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void MergeGroupCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        
        
        void ObjectNameCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args) {
            const std::string keyword = "o";
            _error.ArgCountCheck(keyword, _args.first, static_cast<uint32_t>(_args.second.size()), 1, 1, TERMINATE);
            _groups.push(WavefrontObjGroup(_args.second));
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
