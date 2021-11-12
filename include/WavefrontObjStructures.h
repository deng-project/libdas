#ifndef WAVEFRONT_OBJ_STRUCTURES_H
#define WAVEFRONT_OBJ_STRUCTURES_H


#ifdef WAVEFRONT_OBJ_STRUCTURES_CPP
    #include <vector>
    #include <string>
    #include <queue>
    #include <cstdint>
    #include <cstdlib>
    #include <cmath>
    #include <unordered_map>
    
    #include <ParserErrorHandler.h>
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


    enum WavefrontObjFaceType {
        WAVEFRONT_OBJ_FACE_TYPE_TRIANGLE,
        WAVEFRONT_OBJ_FACE_TYPE_QUAD,
        WAVEFRONT_OBJ_FACE_TYPE_PENTAGON,
        WAVEFRONT_OBJ_FACE_TYPE_HEXAGON,
        WAVEFRONT_OBJ_FACE_TYPE_UNSUPPORTED
    };


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
        std::vector<Point4D<float>> position;
        std::vector<Point3D<float>> points;
        std::vector<Point3D<float>> normals;
        std::vector<Point3D<float>> texture;
    };


    struct WavefrontObjFace {
        std::vector<uint32_t> verts;
        std::vector<uint32_t> textures;
        std::vector<uint32_t> normals;
    };


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
        typedef std::queue<WavefrontObjGroup> Groups;

        // line and args
        typedef std::pair<uint32_t, std::vector<std::string>> ArgsType;

        /**********************************************/
        /*************** Error Checkers ***************/
        /**********************************************/

        /**
         * Check if correct amount of arguments are supplied
         */
        void _ArgCountCheck(AsciiFormatErrorHandler &_error, const std::string &_keyword, int _line, uint32_t _arg_c, 
                            uint32_t _min_args, uint32_t _max_args);

        /**
         * Check if arguments correspond to float data type, assuming that they are tightly packed together
         * @param _error is an instance to AsciiFormatErrorHandler
         * @param _beg is the beginning pointer of the float array
         * @param _end is the ending pointer of the float array
         * @param _keyword is the statement keyword that is used for float arguments
         * @param _arg_offset is the offset of arguments that are read from array
         */
        void _CheckFloatArgs(AsciiFormatErrorHandler &_error, float *_beg, float *_end, size_t _arg_offset, 
                             const std::string &_keyword, ArgsType &_args);


        /**
         * Check if arguments correspond to integer data type, assuming that they are tightly packed together
         * @param _error is an instance to AsciiFormatErrorHandler
         * @param _beg is the beginning pointer of the integer array
         * @param _end is the ending pointer of the integer array
         * @param _arg is a string argument used in parsing
         * @param _keyword is keyword that is used for integer element arguments
         * @param _line is the current line that is parsed
         */
        void _CheckElementIntArgs(AsciiFormatErrorHandler &_error, uint32_t *_beg, uint32_t *_end, const std::string &_arg,
                                  const std::string &_keyword, uint32_t _line);


        Point2D<uint32_t> _ParseDoubleIndexBlock(AsciiFormatErrorHandler &_error, const std::string &_block, 
                                                 const std::string &_keyword, const uint32_t _line);


        /** 
         * Parse index block that contains maximum 3 indices
         * @param _block is a string block that is going to be parsed
         * @param _keyword is the keyword that is associated with the index block
         */
        Point3D<uint32_t> _ParseTripleIndexBlock(AsciiFormatErrorHandler &_error, const std::string &_block, 
                                                 const std::string &_keyword, const uint32_t _line);

    
        /**
         * Universal function pointer for keyword arg reading
         */
        typedef void (*PTR_KeywordArgParser)(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);

        /// Functions to use as parsing function pointer
        void ParseVertexKeywordArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        void ParsePointKeywordArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        void ParseVertexNormalKeywordArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        void ParseTextureVertexKeywordArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        void ParseCSTypeArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        void ParsePolynomialDegreeArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        void ParseBasisMatrixArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        void ParseCSStepArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        void ParsePointsArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void ParseLineArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        void ParseFaceArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void ParseCurveArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void Parse2DCurveArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void ParseSurfaceArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void ParseParameterArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void ParseTrimmingCurveArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void ParseHoleCurveArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void ParseSpecialCurveArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void ParseSpecialPointArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        void ParseGroupArgs(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void ParseShadingGroup(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        //void ParseMergeGroup(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
        void ParseObjectName(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args);
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

#endif
