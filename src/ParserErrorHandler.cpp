/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: ParseErrorHandler.cpp - Parser error handler class implementation
/// author: Karl-Mihkel Ott

#define ERROR_HANDLER_CPP
#include <ParserErrorHandler.h>

namespace Libdas {

    AsciiFormatErrorHandler::AsciiFormatErrorHandler(ModelFormat _format) : m_format(_format) {}

    std::string AsciiFormatErrorHandler::_GetFormatErrorMsg() {
        switch(m_format) {
            case MODEL_FORMAT_WOBJ: return "Wavefront Obj error: ";
            case MODEL_FORMAT_STL: return "STL error:";
            case MODEL_FORMAT_FBX: return "ASCII FBX error:";
            case MODEL_FORMAT_GLTF: return "GLTF error:";
            default: LIBDAS_ASSERT(false);
        }

        return "";
    }


    /*** Syntax error handler definitions ***/

    void AsciiFormatErrorHandler::SyntaxError(ErrorType _type, int _line, TerminationType _terminate) {
        switch(_type) {
            case LIBDAS_ERROR_INCOMPLETE_NEWLINE:
                std::cerr << "Incomplete newline after line " << _line << std::endl;
                break;

            default:
                LIBDAS_ASSERT(false);
        }

        if(_terminate) exit(_type);
    }


    void AsciiFormatErrorHandler::SyntaxError(ErrorType _type, int _line, const std::string &_keyword, 
                                              const std::string &_arg, TerminationType _terminate) {
        std::string err_beg = _GetFormatErrorMsg();

        switch(_type) {
            case LIBDAS_ERROR_INVALID_KEYWORD:
                std::cerr << err_beg << " Invalid keyword '" << _keyword << "' at line " << _line << std::endl;
                break;

            case LIBDAS_ERROR_INVALID_ARGUMENT:
                std::cerr << err_beg << "Invalid argument '" << _arg << "' for keyword '" << _keyword << "' at line " << _line << std::endl;
                break;

            case LIBDAS_ERROR_TOO_MANY_ATTRIBUTES:
                std::cerr << err_beg << " Too many attributes for keyword '" << _keyword << "' in line " << _line << std::endl;
                break;

            case LIBDAS_ERROR_NOT_ENOUGH_ATTRIBUTES:
                std::cerr << err_beg << " Not enough attributes for keyword '" << _keyword << "' in line " << _line << std::endl;
                break;

            case LIBDAS_ERROR_INVALID_CSTYPE:
                std::cerr << err_beg << " Invalid cstype '" << _keyword << "' in line " << _line << std::endl;
                break;

            case LIBDAS_ERROR_INVALID_SYMBOL:
                std::cerr << err_beg << " Invalid symbol '" << _keyword << "' in line " << _line << std::endl;
                break;

            default:
                LIBDAS_ASSERT(false);
        }

        if(_terminate) exit(_type);
    }


    /// Warning message displaying method
    void AsciiFormatErrorHandler::WarningMessage(ErrorType _type, std::vector<WarningElement> &_warn_elems) {
        switch(_type) {
            case LIBDAS_WARNING_UNUSED_STATEMENT:
                std::cerr << "WARNING: Unused statements: ";
                break;

            case LIBDAS_WARNING_MULTIPLE_OBJECTS:
                std::cerr << "WARNING: Multiple objects: ";
                break;
        }

        for(size_t i = 0; i < _warn_elems.size(); i++) {
            if(_warn_elems[i].end_line != INT_MAX)
                std::cerr << _warn_elems[i].name << " line(" << _warn_elems[i].beg_line << ")";
            else {
                std::cerr << _warn_elems[i].name << " lines(" << _warn_elems[i].beg_line << ", " <<
                    _warn_elems[i].end_line << ")";
            }

            if(i != _warn_elems.size() - 1)
                std::cerr << ", ";
        }
        std::cerr << std::endl;
    }
}
