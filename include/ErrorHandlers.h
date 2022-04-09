/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: ErrorHandlers.h - Error handler classes header
/// author: Karl-Mihkel Ott

#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

/// Source file includes
#ifdef ERROR_HANDLER_CPP
    #include <cstdint>
    #include <cmath>
    #include <climits>
    #include <LibdasAssert.h>

    #include <iostream>
    #include <string>
    #include <vector>
#endif

#include <LibdasAssert.h>

/// Termination definations
typedef bool TerminationType;
#define TERMINATE true
#define NO_TERMINATE false

/// Ascii format errors
typedef uint16_t ErrorType;
#define LIBDAS_ERROR_INVALID_KEYWORD                1   // statement error
#define LIBDAS_ERROR_INVALID_TYPE                   2   // statement error (JSON mostly, because of its loose typing)
#define LIBDAS_ERROR_NO_IDENTIFIER                  3   // statement error
#define LIBDAS_ERROR_INVALID_ARGUMENT               4   // statement error
#define LIBDAS_ERROR_TOO_MANY_ATTRIBUTES            5   // statement error
#define LIBDAS_ERROR_NOT_ENOUGH_ATTRIBUTES          6   // statement error
#define LIBDAS_ERROR_INCOMPLETE_NEWLINE             7   // non-statement error
#define LIBDAS_ERROR_INVALID_CSTYPE                 8   // Wavefront OBJ only!!!
#define LIBDAS_ERROR_INVALID_SYMBOL                 9   // non-statement error
#define LIBDAS_ERROR_UNEXPECTED_END_STATEMENT       10  // statement error
#define LIBDAS_ERROR_INCOMPLETE_SCOPE               11  // statement error
#define LIBDAS_ERROR_SCOPE_ALREADY_CLOSED           12  // statement error

/// Binary format errors
#define LIBDAS_ERROR_INVALID_SIGNATURE              13  // binary error
#define LIBDAS_ERROR_INVALID_DATA_LENGTH            14  // binary error
#define LIBDAS_ERROR_INVALID_VALUE                  15  // binary or ascii error

/// Misc
#define LIBDAS_ERROR_INVALID_FILE                   16  // misc

/// Warnings
#define LIBDAS_WARNING_UNUSED_STATEMENT             17  // statement warning
#define LIBDAS_WARNING_MULTIPLE_OBJECTS             18  // non-statement warning

// DEBUG: loggers
#ifdef _DEBUG
    #define DEBUG_LOG(x)            std::cout << "DEBUG: " << x << std::endl
    #define EXIT_ON_ERROR(code)     std::abort()
#else
    #define DEBUG_LOG(x)
    #define EXIT_ON_ERROR(code)     std::exit(code)
#endif

// no LIBDAS_DEFS_ONLY
#ifndef LIBDAS_DEFS_ONLY

namespace Libdas {

    // line and args
    typedef std::pair<uint32_t, std::vector<std::string>> ArgsType;
    
    enum ModelFormat {
        MODEL_FORMAT_WOBJ,
        MODEL_FORMAT_STLA,
        MODEL_FORMAT_STLB,
        MODEL_FORMAT_FBXA,
        MODEL_FORMAT_FBXB,
        MODEL_FORMAT_GLTF,
        MODEL_FORMAT_GLB,
        MODEL_FORMAT_JSON,
        MODEL_FORMAT_DAS,
        MODEL_FORMAT_DASC
    };


    /////////////////////////////////////////////////
    // **** Warnings are pretty much useless ***** //
    /////////////////////////////////////////////////
    
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

            /// Error handler declarations
            void Error(ErrorType _type, int _line, TerminationType _terminate = TERMINATE);
            void Error(ErrorType _type, int _line, const std::string &_keyword, const std::string &_arg = "", TerminationType _terminate = TERMINATE);

            /** 
             * Display warning message
             * @param _type is the type of warning declared with LIBDAS_WARNING_* definition
             * @param _warn_elems is all warning elements to display in the message
             */
            void WarningMessage(ErrorType _type, std::vector<WarningElement> &_warn_elems);
            /**
             * Check if correct amount of arguments are supplied
             */
            void ArgCountCheck(const std::string &_keyword, int _line, uint32_t _arg_c, uint32_t _min_args, uint32_t _max_args, 
                               TerminationType _terminate = TERMINATE);
            /**
             * Check if arguments correspond to float data type, assuming that they are tightly packed together
             * @param _beg is the beginning pointer of the float array
             * @param _end is the ending pointer of the float array
             * @param _keyword is the statement keyword that is used for float arguments
             * @param _arg_offset is the offset of arguments that are read from array
             */
            void CheckFloatArgs(float *_beg, float *_end, size_t _arg_offset, const std::string &_keyword, ArgsType &_args);
            /**
             * Check if arguments correspond to integer data type, assuming that they are tightly packed together
             * @param _error is an instance to AsciiFormatErrorHandler
             * @param _beg is the beginning pointer of the integer array
             * @param _end is the ending pointer of the integer array
             * @param _arg is a string argument used in parsing
             * @param _keyword is keyword that is used for integer element arguments
             * @param _line is the current line that is parsed
             */
            void CheckIntArgs(uint32_t *_beg, uint32_t *_end, const std::string &_arg, const std::string &_keyword, uint32_t _line);
    };


    /// Error handling class for binary formats
    class BinaryFormatErrorHandler {
        private:
            ModelFormat m_format;

        private:
            /**
             * Generate correct error message beginning according to the format used
             */
            std::string _GetFormatErrorMsg();

        public:
            BinaryFormatErrorHandler(ModelFormat _format);
            /**
             * Throw an binary format error
             * @param _type specifies the binary format error type
             * @param _terminate specifies whether the program should be terminated or not
             */
            void Error(ErrorType _type, const std::string &_arg = "", TerminationType _terminate = TERMINATE);
    };
}

#endif
#endif
