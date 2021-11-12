/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: ParseErrorHandler.h - Parser error handler class header
/// author: Karl-Mihkel Ott

#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

/// Source file includes
#ifdef ERROR_HANDLER_CPP
    #include <cstdint>
    #include <climits>
    #include <LibdasAssert.h>

    #include <iostream>
    #include <string>
    #include <vector>
#endif

/// Termination definations
typedef bool TerminationType;
#define TERMINATE true
#define NO_TERMINATE false

/// Errors
typedef uint16_t ErrorType;
#define LIBDAS_ERROR_INVALID_KEYWORD                1 // statement error
#define LIBDAS_ERROR_INVALID_ARGUMENT               2 // statement error
#define LIBDAS_ERROR_TOO_MANY_ATTRIBUTES            3 // statement error
#define LIBDAS_ERROR_NOT_ENOUGH_ATTRIBUTES          4 // statement error
#define LIBDAS_ERROR_INCOMPLETE_NEWLINE             5 // non-statement error
#define LIBDAS_ERROR_INVALID_CSTYPE                 6 // Wavefront OBJ only!!!
#define LIBDAS_ERROR_INVALID_SYMBOL                 7 // non-statement error
#define LIBDAS_ERROR_INVALID_FILE                   8 // misc

/// Warnings
#define LIBDAS_WARNING_UNUSED_STATEMENT             9
#define LIBDAS_WARNING_MULTIPLE_OBJECTS             10


namespace Libdas {
    
    enum ModelFormat {
        MODEL_FORMAT_WOBJ,
        MODEL_FORMAT_STL,
        MODEL_FORMAT_FBX,
        MODEL_FORMAT_GLTF,
        MODEL_FORMAT_GLB,
        MODEL_FORMAT_DAS
    };


    /// Data structure for displaying warnings
    struct WarningElement {
        std::string name;
        int beg_line;
        int end_line;
    };


    /// Error handling class for ASCII formats
    class AsciiFormatErrorHandler {
        private:
            ModelFormat m_format;

        private:
            /**
             * Generate correct error message beginning according to the format used
             */
            std::string _GetFormatErrorMsg();

        public:
            AsciiFormatErrorHandler(ModelFormat _format);

            /// Syntax error handler declarations
            void SyntaxError(ErrorType _type, int _line, TerminationType _terminate = NO_TERMINATE);
            void SyntaxError(ErrorType _type, int _line, const std::string &_keyword, const std::string &_arg = "", TerminationType _terminate = NO_TERMINATE);

            /// Warning message displaying method
            void WarningMessage(ErrorType _type, std::vector<WarningElement> &_warn_elems);
    };
}

#endif
