/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: DASWriterCore.h - DAS writer parent class header
/// author: Karl-Mihkel Ott

#ifndef DAS_WRITER_CORE_H
#define DAS_WRITER_CORE_H


#ifdef DAS_WRITER_CORE_CPP
    #include <string>
    #include <cstring>
    #include <fstream>
    #include <type_traits>
    #include <vector>
    #include <iostream>
    #include <chrono>

    #include <Vector.h>
    #include <Matrix.h>
    #include <DasStructures.h>
    #include <ParserErrorHandler.h>
    #include <LibdasAssert.h>
#endif


// make sure that carriage return is respected in windows
#ifdef _WIN32
    #define LIBDAS_DAS_NEWLINE "\r\n"
#else
    #define LIBDAS_DAS_NEWLINE "\n"
#endif

namespace Libdas {

    class DasWriterCore {
        private:
            std::string m_file_name;
            std::ofstream m_out_stream;
            bool m_use_compression = false;

        private:
            /**
             * Check if the current m_out_file string contains extension .das
             * If not then add it
             */
            void _CheckAndAddFileExtension();
            /**
             * Open a output file stream with m_out_file as a used file name
             */
            void _OpenFileStream();
            /**
             * Write a single string value to the stream
             * @param _value_name is a value name that is used
             * @param _value is a string value that is outputted to the stream
             */
            void _WriteStringValue(const std::string &_value_name, const std::string &_value);
            /**
             * Write a numerical value to the stream
             * @param _value_name is a value name string that is used to define the value
             * @param _value is a value in type T that is outputted to stream
             */
            template <typename T>
            void _WriteNumericalValue(const std::string &_value_name, T _value) {
                LIBDAS_ASSERT(std::is_floating_point<T>::value || std::is_integral<T>::value);
                LIBDAS_ASSERT(m_out_stream.is_open());

                m_out_stream.write(_value_name.c_str(), _value_name.size());
                m_out_stream.write(": ", 2);
                m_out_stream.write(reinterpret_cast<char*>(&_value), sizeof(T));
                m_out_stream.write(LIBDAS_DAS_NEWLINE, strlen(LIBDAS_DAS_NEWLINE));
            }
            /**
             * Write a generic data value to the stream
             * @param _value_name is a value name string that is used to define the value
             * @param _data is a pointer to the valid data that will be written 
             * @param _len is a length of the data in bytes
             */
            void _WriteGenericDataValue(const std::string &_value_name, const char *_data, const size_t _len);
            /**
             * Write a generic 4x4 matrix value to the stream
             * @param _value_name is a value name string that is used to define the value
             * @param _mat is a reference to Matrix4<T> object that is written to the stream
             */
            template <typename T>
            void _WriteMatrixValue(const std::string &_value_name, const Matrix4<T> &_mat) {
                LIBDAS_ASSERT(std::is_floating_point<T>::value || std::is_integral<T>::value);
                LIBDAS_ASSERT(m_out_stream.is_open());

                m_out_stream.write(_value_name.c_str(), _value_name.size());
                m_out_stream.write(": ", 2);

                for(auto it = _mat.BeginRowMajor(); it != _mat.EndRowMajor(); it++) {
                    m_out_stream.write(reinterpret_cast<const char*>(it.GetData()), sizeof(T));
                }

                m_out_stream.write(LIBDAS_DAS_NEWLINE, strlen(LIBDAS_DAS_NEWLINE));
            }
            /**
             * Start a new scope definition
             * @param _scope_name is a specified scope name to use when starting a new scope
             */
            void _WriteScopeBeginning(const std::string &_scope_name);
            /**
             * End a scope declaration
             */
            void _EndScope();


        public:
            DasWriterCore(const std::string &_file_name = "");
            ~DasWriterCore();
            /**
             * Open a new file for output
             * @param _file_name is a new file name to use when opening a stream
             */
            void NewFile(const std::string &_file_name);
            /**
             * Write a file signature with its properties section
             * @param _properties is a reference to DasProperties
             */
            void InitialiseFile(const DasProperties &_properties);
            /**
             * Write a generic buffer scope into a file
             * @param _buffer is a reference to DasBuffer object
             */
            void WriteBuffer(const DasBuffer &_buffer);
            /**
             * Write a model information to a file
             * @param _model is a reference to DasModel object
             */
            void WriteModel(const DasModel &_model);
    };
}

#endif
