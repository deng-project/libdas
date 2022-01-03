// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: STLStructures.h - STL parsing functions and structures header
// author: Karl-Mihkel Ott

#ifndef STL_STRUCTURES_H
#define STL_STRUCTURES_H


#ifdef STL_STRUCTURES_CPP
    #include <cstdint>
    #include <string>
    #include <vector>
    #include <fstream>
    #include <array>

    #include <Api.h>
    #include <Points.h>
    #include <ErrorHandlers.h>
#endif


namespace Libdas {

    struct LIBDAS_API STLFacet {
        Point3D<float> normal;
        std::array<Point3D<float>, 3> vertices;
        size_t vert_index = 0;
    };

    struct LIBDAS_API STLObject {
        std::string name;
        std::vector<STLFacet> facets;

        STLObject(std::string &&_name = "Binary") : name(std::move(_name)) {}
    };

    namespace STLFunctions {

        /**
         * Identify the STL format type (ascii or binary)
         * @param _file_name specifies the file name to use
         * @return true is the format is Ascii STL and false otherwards
         */
        LIBDAS_API bool Identify(const std::string &_file_name);

        typedef std::vector<STLObject> Groups;
        
        /**
         * Universal function pointer type declaration for STL keyword action callback
         */
        typedef void (*PTR_KeywordCallback)(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args, void *custom);


        /**
         * Concatenate arguments into single string object
         */
        LIBDAS_API std::string _ConcatenateArgs(ArgsType &_args);

        /// Keyword parsing functions 
        /// NOTE: Following functions require custom data
        ///  * _SolidCallback - m_is_solid
        ///  * _FacetCallback - m_is_facet
        ///  * _OuterLoopCallback - m_is_loop
        ///  * _EndLoopCallback - m_is_loop
        ///  * _EndFacetCallback - m_is_facet
        ///  * _EndSolidCallback - m_is_solid

        LIBDAS_API void _SolidCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args, void *custom);
        LIBDAS_API void _FacetCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args, void *custom);
        LIBDAS_API void _OuterLoopCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args, void *custom);
        LIBDAS_API void _VertexCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args, void *custom);
        LIBDAS_API void _EndLoopCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args, void *custom);
        LIBDAS_API void _EndFacetCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args, void *custom);
        LIBDAS_API void _EndSolidCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args, void *custom);
    }
}

#endif
