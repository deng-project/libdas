/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: STLStructures.cpp - STL parsing functions and structures implementation
/// author: Karl-Mihkel Ott

#define STL_STRUCTURES_CPP
#include <STLStructures.h>


namespace Libdas {
    
    namespace STLFunctions {

        bool Identify(const std::string &_file_name) {
            std::ifstream file(_file_name, std::ios_base::binary);

            // skip whitespaces
            char ch = file.get();
            while(ch == ' ' || ch == '\n' || ch == '\r')
                ch = file.get();

            file.seekg(-1, std::ios_base::cur);

            // read solid statement
            const std::string exp_statement = "solid ";
            char statement[7] = {};
            file.read(statement, 6);

            file.close();

            // determine the file identity
            if(std::string(statement) == exp_statement)
                return true;
            else return false;
        }


        std::string _ConcatenateArgs(ArgsType &_args) {
            std::string con = "";
            for(size_t i = 0; i < _args.second.size(); i++) {
                con += _args.second[i];

                if(i != _args.second.size() - 1)
                    con += " ";
            }

            return con;
        }

        void _SolidCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args, void *custom) {
            const std::string keyword = "solid";
            _error.ArgCountCheck(keyword, _args.first, static_cast<uint32_t>(_args.second.size()), 1, UINT32_MAX, TERMINATE);


            bool *p_is_solid = reinterpret_cast<bool*>(custom);
            if(*p_is_solid)
                _error.Error(LIBDAS_ERROR_INCOMPLETE_SCOPE, _args.first, keyword);

            *p_is_solid = true;

            // concatenate arguments into one name
            std::string name = _ConcatenateArgs(_args);
            _groups.push_back(STLObject(std::move(name)));
        }


        void _FacetCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args, void *custom) {
            const std::array<std::string, 2> keywords = {"facet", "normal"};
            const std::string con_keyword = keywords[0] + " " + keywords[1];
            _error.ArgCountCheck(keywords[0], _args.first, static_cast<uint32_t>(_args.second.size()), 4, 4, TERMINATE);

            // check that the second keyword is present
            if(_args.second[0] != keywords[1])
                _error.Error(LIBDAS_ERROR_INVALID_KEYWORD, _args.first, keywords[0], "", TERMINATE);
            
            Point3D<float> normal;
            normal.x = std::stof(_args.second[1]);
            normal.y = std::stof(_args.second[2]);
            normal.z = std::stof(_args.second[3]);
            _error.CheckFloatArgs(&normal.x, &normal.z, 1, con_keyword, _args);

            bool *p_is_facet = reinterpret_cast<bool*>(custom);
            
            // check if facet is already declared
            if(*p_is_facet)
                _error.Error(LIBDAS_ERROR_INCOMPLETE_SCOPE, _args.first, con_keyword);
            *p_is_facet = true;

            _groups.back().facets.push_back(STLFacet());
            _groups.back().facets.back().normal = std::move(normal);
        }


        void _OuterLoopCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args, void *custom) {
            const std::array<std::string, 2> keywords = {"outer", "loop"};
            const std::string con_keyword = keywords[0] + " " + keywords[1];
            _error.ArgCountCheck(keywords[0], _args.first, static_cast<uint32_t>(_args.second.size()), 1, 1, TERMINATE);
            
            // check that the second keyword is present
            if(_args.second[0] != keywords[1])
                _error.Error(LIBDAS_ERROR_INVALID_KEYWORD, _args.first, keywords[0], "", TERMINATE);

            bool *p_is_loop = reinterpret_cast<bool*>(custom);
            if(*p_is_loop)
                _error.Error(LIBDAS_ERROR_INCOMPLETE_SCOPE, _args.first, con_keyword);

            *p_is_loop = true;
        }


        void _VertexCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args, void *custom) {
            const std::string keyword = "vertex";
            _error.ArgCountCheck(keyword, _args.first, static_cast<uint32_t>(_args.second.size()), 3, 3, TERMINATE);

            Point3D<float> vertex;
            vertex.x = std::stof(_args.second[0]);
            vertex.y = std::stof(_args.second[1]);
            vertex.z = std::stof(_args.second[2]);
            _error.CheckFloatArgs(&vertex.x, &vertex.z, 0, keyword, _args);

            size_t &index = _groups.back().facets.back().vert_index;
            _groups.back().facets.back().vertices.at(index) = vertex;
            index++;
        }


        void _EndLoopCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args, void *custom) {
            const std::string keyword = "endloop";
            _error.ArgCountCheck(keyword, _args.first, static_cast<uint32_t>(_args.second.size()), 0, 0, TERMINATE);
            
            bool *p_is_loop = reinterpret_cast<bool*>(custom);
            if(!(*p_is_loop))
                _error.Error(LIBDAS_ERROR_UNEXPECTED_END_STATEMENT, _args.first, keyword);

            *p_is_loop = false;
        }


        void _EndFacetCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args, void *custom) {
            const std::string keyword = "endfacet";
            _error.ArgCountCheck(keyword, _args.first, static_cast<uint32_t>(_args.second.size()), 0, 0, TERMINATE);

            bool *p_is_facet = reinterpret_cast<bool*>(custom);
            if(!(*p_is_facet)) 
                _error.Error(LIBDAS_ERROR_UNEXPECTED_END_STATEMENT, _args.first, keyword);

            *p_is_facet = false;
        }


        void _EndSolidCallback(Groups &_groups, AsciiFormatErrorHandler &_error, ArgsType &_args, void *custom) {
            const std::string keyword = "endsolid";
            _error.ArgCountCheck(keyword, _args.first, static_cast<uint32_t>(_args.second.size()), 1, 1, TERMINATE);
            std::string con_name = _ConcatenateArgs(_args);
            bool *p_is_solid = reinterpret_cast<bool*>(custom);

            // check if the solid statement was previously declared
            if(!(*p_is_solid))
                _error.Error(LIBDAS_ERROR_UNEXPECTED_END_STATEMENT, _args.first, keyword);

            *p_is_solid = false;

            // check if the concatenated name is valid for the solid group
            if(con_name != _groups.back().name)
                _error.Error(LIBDAS_ERROR_INVALID_ARGUMENT, _args.first, keyword, con_name);
        }
    }
}
