/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: WavefrontObjStructures.h - Wavefront Obj parsing functions and structures header
/// author: Karl-Mihkel Ott

#ifndef WAVEFRONT_OBJ_STRUCTURES_H
#define WAVEFRONT_OBJ_STRUCTURES_H


#ifdef WAVEFRONT_OBJ_STRUCTURES_CPP
    #include <vector>
    #include <string>
    #include <array>
    #include <queue>
    #include <cstdint>
    #include <cstdlib>
    #include <cmath>
    #include <unordered_map>
    
    #include <Api.h>
    #include <ErrorHandlers.h>
    #include <Points.h>
#endif

namespace Libdas {

    enum WavefrontObjStatementType {
        WAVEFRONT_OBJ_STATEMENT_NONE,                   // special value to define empty newline
        WAVEFRONT_OBJ_STATEMENT_VERTEX,                 // v
        WAVEFRONT_OBJ_STATEMENT_POINT,                  // vp
        WAVEFRONT_OBJ_STATEMENT_VERTEX_NORMAL,          // vn
        WAVEFRONT_OBJ_STATEMENT_VERTEX_TEXTURE,         // vt
        WAVEFRONT_OBJ_STATEMENT_CSTYPE,                 // cstype
        WAVEFRONT_OBJ_STATEMENT_POLYNOMIAL_DEGREE,      // deg
        WAVEFRONT_OBJ_STATEMENT_BASIS_MATRIX,           // bmat
        WAVEFRONT_OBJ_STATEMENT_STEP,                   // step
        WAVEFRONT_OBJ_STATEMENT_POINTS,                 // p
        WAVEFRONT_OBJ_STATEMENT_LINE,                   // l
        WAVEFRONT_OBJ_STATEMENT_FACE,                   // f
        WAVEFRONT_OBJ_STATEMENT_CURV,                   // curv
        WAVEFRONT_OBJ_STATEMENT_CURV2D,                 // curv2d
        WAVEFRONT_OBJ_STATEMENT_SURFACE,                // surf
        WAVEFRONT_OBJ_STATEMENT_PARAMETER,              // parm
        WAVEFRONT_OBJ_STATEMENT_TRIMMING_CURV,          // trim
        WAVEFRONT_OBJ_STATEMENT_HOLE_CURV,              // hole
        WAVEFRONT_OBJ_STATEMENT_SPECIAL_CURV,           // scrv
        WAVEFRONT_OBJ_STATEMENT_SPECIAL_POINT,          // sp
        WAVEFRONT_OBJ_STATEMENT_END,                    // end (noargs)
        WAVEFRONT_OBJ_STATEMENT_GROUP,                  // g
        WAVEFRONT_OBJ_STATEMENT_SHADING_GROUP,          // s
        WAVEFRONT_OBJ_STATEMENT_MERGE_GROUP,            // mg
        WAVEFRONT_OBJ_STATEMENT_OBJECT,                 // o
        WAVEFRONT_OBJ_STATEMENT_BEVEL_INTERPOLATION,    // bevel
        WAVEFRONT_OBJ_STATEMENT_COLOR_INTERPOLATION,    // c_interp
        WAVEFRONT_OBJ_STATEMENT_DIFFUSE_INTERPOLATION,  // d_interp
        WAVEFRONT_OBJ_STATEMENT_LEVEL_OF_DETAIL,        // lod
        WAVEFRONT_OBJ_STATEMENT_USE_MAP,                // usemap
        WAVEFRONT_OBJ_STATEMENT_MAP_LIBRARY,            // maplib
        WAVEFRONT_OBJ_STATEMENT_USE_MATERIAL,           // usemtl
        WAVEFRONT_OBJ_STATEMENT_MATERIAL_LIBRARY,       // mtllib
        WAVEFRONT_OBJ_STATEMENT_SHADOW_OBJECT,          // shadow_obj
        WAVEFRONT_OBJ_STATEMENT_RAY_TRACING_OBJECT,     // trace_obj
        WAVEFRONT_OBJ_STATEMENT_CURV_TECHNIQUE,         // ctech
        WAVEFRONT_OBJ_STATEMENT_SURFACE_TECHNIQUE,      // stech
        WAVEFRONT_OBJ_STATEMENT_COMMENT                 // #
    };


    /**
     * Reserved for future use
     */
    enum WavefrontObjFaceType {
        WAVEFRONT_OBJ_FACE_TYPE_TRIANGLE,
        WAVEFRONT_OBJ_FACE_TYPE_QUAD,
        WAVEFRONT_OBJ_FACE_TYPE_PENTAGON,
        WAVEFRONT_OBJ_FACE_TYPE_HEXAGON,
        WAVEFRONT_OBJ_FACE_TYPE_UNSUPPORTED
    };


    /**
     * Reserved for future use
     */
    enum WavefrontObjCSType {
        WAVEFRONT_OBJ_CS_TYPE_BMATRIX,
        WAVEFRONT_OBJ_CS_TYPE_BEZIER,
        WAVEFRONT_OBJ_CS_TYPE_BSPLINE,
        WAVEFRONT_OBJ_CS_TYPE_CARDINAL,
        WAVEFRONT_OBJ_CS_TYPE_TAYLOR
    };


    /** 
     * Structure for containing information about object vertices
     */
    struct WavefrontObjVertices {
        std::vector<Point3D<float>> position;
        std::vector<Point3D<float>> points;
        std::vector<Point3D<float>> normals;
        std::vector<Point3D<float>> texture;
    };


    /**
     * Structure for containing single face index properties
     */
    struct WavefrontObjIndex {
        uint32_t vert;
        uint32_t texture;
        uint32_t normal;
    };


    /**
     * Data type for containing face indicies
     */
    typedef std::vector<WavefrontObjIndex> WavefrontObjFace;


    /**
     * Structure for containing information about object indices
     */
    struct WavefrontObjIndices {
        std::vector<WavefrontObjFace> faces;
        std::vector<std::vector<uint32_t>> pt;
        std::vector<Point2D<int32_t>> lines;

        uint32_t global_face_offset = 0;
        uint32_t global_pt_offset = 0;
        uint32_t global_line_offset = 0;
    };


    /**
     * Structure for containing information about object rendering attributes
     */
    struct WavefrontObjRenderAttributes {
        bool use_bevel = false;
        bool use_color_interp = false;
        bool use_diffuse_interp = false;
        uint32_t lod = 0;
        std::unordered_map<std::string, uint32_t> maplibs;  // associate maplib file name with index (starting from 1)
        uint32_t map_id = 0;                                // reserved value 0 means that no map is used
        std::unordered_map<std::string, uint32_t> mtllibs;
        uint32_t mtl_id = 0;                                // reserved value 0 means none
        std::string shadow_obj = "";                        // reserved value "" means none
        std::string trace_obj = "";                         // reserved value "" means none
    };


    /**
     * Structure for containing information about single name group
     * Libdas parser consideres objects and groups to be same structures
     */
    struct WavefrontObjGroup {
        WavefrontObjGroup(const std::vector<std::string> &names) : names(std::move(names)) {}

        std::vector<std::string> names;
        WavefrontObjRenderAttributes rend_attrs;
        WavefrontObjVertices vertices;
        WavefrontObjIndices indices;
    };


    /// Namespace for containing all Wavefront OBJ keyword parsing functions
    namespace WavefrontObjFunctions {
        typedef std::vector<WavefrontObjGroup> Groups;
        /**
         * Universal function pointer for keyword action callback
         */
        typedef void (*PTR_KeywordCallback)(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);

        /***************************************************/
        /*************** Index block parsing ***************/
        /***************************************************/
    
        /**
         * Parse index block that contains maximum 2 indices
         * @param _error is a reference to AsciiFormatErrorHandler
         * @param _block is a string block that is going to be parsed
         * @param _keyword is a keyword that is used for the statement
         * @param _line is the current statement line
         * @return Point2D instance, which contains parsed double indicies
         */
        LIBDAS_API Point2D<uint32_t> _DoubleIndexBlockCallback(AsciiFormatErrorHandler &_error, const std::string &_block,
                                                 const std::string &_keyword, const uint32_t _line);
        /** 
         * Parse index block that contains maximum 3 indices
         * @param _error is a reference to AsciiFormatErrorHandler
         * @param _block is a string block that is going to be parsed
         * @param _keyword is the keyword that is associated with the index block
         * @param _line is the current statement line
         * @return Point3D instance, which contains parsed triple indicies
         */
        LIBDAS_API Point3D<uint32_t> _TripleIndexBlockCallback(AsciiFormatErrorHandler &_error, const std::string &_block,
                                                 const std::string &_keyword, const uint32_t _line);

        /// Functions to use as parsing function pointer
        LIBDAS_API void VertexKeywordArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        LIBDAS_API void PointKeywordArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        LIBDAS_API void VertexNormalKeywordArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        LIBDAS_API void TextureVertexKeywordArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        LIBDAS_API void CSTypeArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        LIBDAS_API void PolynomialDegreeArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        LIBDAS_API void BasisMatrixArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        LIBDAS_API void CSStepArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        LIBDAS_API void PointsArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //LIBDAS_API void LineArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        LIBDAS_API void FaceArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //LIBDAS_API void CurveArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //LIBDAS_API void 2DCurveArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //LIBDAS_API void SurfaceArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //LIBDAS_API void ParameterArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //LIBDAS_API void TrimmingCurveArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //LIBDAS_API void HoleCurveArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //LIBDAS_API void SpecialCurveArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //LIBDAS_API void SpecialPointArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        LIBDAS_API void GroupArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //LIBDAS_API void ShadingGroupCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //LIBDAS_API void MergeGroupCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        LIBDAS_API void ObjectNameCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //LIBDAS_API void BevelInterpolationArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //LIBDAS_API void ColorInterpolationArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //LIBDAS_API void DiffuseInterpolationArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //LIBDAS_API void LevelOfDetailArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //LIBDAS_API void UseMapArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //LIBDAS_API void MapLibraryArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //LIBDAS_API void UseMaterialArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //LIBDAS_API void MaterialLibraryArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //LIBDAS_API void ShadowObjectArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //LIBDAS_API void RayTracingObjectArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //LIBDAS_API void CurveTechniqueArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //LIBDAS_API void SurfaceTechniqueArgsCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
    }
}

#endif
