// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: DASWriterCore.h - DAS writer parent class header
// author: Karl-Mihkel Ott

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

    #include <LibdasAssert.h>
    #include <ErrorHandlers.h>
    #include <Iterators.h>
    #include <Vector.h>
    #include <Matrix.h>
    #include <Quaternion.h>
    #include <DasStructures.h>
    #include <TextureReader.h>
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
            std::ofstream m_out_stream;
            bool m_use_compression = false;

        protected:
            std::string m_file_name;

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
                m_out_stream.write(reinterpret_cast<const char*>(&_value), sizeof(T));
                m_out_stream.write(LIBDAS_DAS_NEWLINE, strlen(LIBDAS_DAS_NEWLINE));
            }
            /**
             * Write an array value to the stream
             * @param _value_name is a value name string that is used to define the value
             * @param _n is the total value count
             * @param _values is a T pointer value to the memory area that contains all the array values
             */
            template<typename T>
            void _WriteArrayValue(const std::string &_value_name, uint32_t _n, T *_values) {
                LIBDAS_ASSERT(m_out_stream.is_open());

                m_out_stream.write(_value_name.c_str(), _value_name.size());
                m_out_stream.write(": ", 2);
                m_out_stream.write(reinterpret_cast<const char*>(_values), sizeof(T) * _n);
                m_out_stream.write(LIBDAS_DAS_NEWLINE, strlen(LIBDAS_DAS_NEWLINE));
            }
            /**
             * Write a generic data value to the stream
             * @param _data is a pointer to the valid data that will be written 
             * @param _len is a length of the data in bytes
             * @param _append_nl is a boolean flag indicating if the appendix newline should be written or not
             * @param _value_name is a value name string that is used to define the value
             */
            void _WriteGenericDataValue(const char *_data, const size_t _len, bool _append_nl = true, const std::string &_value_name = "");
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
             * Write a scope ending declaration
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
             * Close the stream if opened
             */
            void CloseStream();
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
             * Write texture buffers from given texture images files
             * @param _textures is a const reference to std::vector that contains all texture file names in std::string type
             */
            void WriteTextureBuffer(const std::vector<std::string> &_textures);
            /**
             * Write a model information to the file
             * @param _model is a reference to DasModel object
             */
            void WriteMesh(const DasMesh &_mesh);
            /**
             * Write a node scope to the file
             * @param _node specifies a reference to DasNode object
             */
            void WriteNode(const DasNode &_node);
            /**
             * Write a scene info to the file
             * @param _scene is a reference to DasScene object
             */
            void WriteScene(const DasScene &_scene);
            /**
             * Write skeleton info to the file
             * @param _skeleton is a reference to DasSkeleton object
             */
            void WriteSkeleton(const DasSkeleton &_skeleton);
            /**
             * Write skeleton joint info to the file
             * @param _joint is a reference to DasSkeletonJoint object
             */
            void WriteSkeletonJoint(const DasSkeletonJoint &_joint);
            /**
             * Write an animation information to the file
             * @param _animation is a reference to DasAnimation object
             */
            void WriteAnimation(const DasAnimation &_animation);
            /**
             * Check if compression is needed and if it is then compress it using Huffman algorithm
             */
            static void CreateDASC(const std::string &_file_name, bool delete_original = true);
    };
}

#endif
