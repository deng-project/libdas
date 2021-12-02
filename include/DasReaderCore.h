/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: DasReaderCore.h - DAS file format reader class header
/// author: Karl-Mihkel Ott

#ifndef DAS_READER_CORE_H
#define DAS_READER_CORE_H


#ifdef DAS_READER_CORE_CPP
    #include <any>
    #include <cstring>
    #include <climits>
    #include <string>
    #include <fstream>
#ifdef _DEBUG
    #include <iostream>
#endif
    #include <vector>
    #include <unordered_map>
    
    #include <Vector.h>
    #include <Matrix.h>
    #include <AsciiStreamReader.h>
    #include <AsciiLineReader.h>
    #include <DasStructures.h>
    #include <LibdasAssert.h>
    #include <ErrorHandlers.h>
#endif


namespace Libdas {

    /**
     * Enumeral values to specify all possible das scopes
     */
    enum DasScopeType {
        LIBDAS_DAS_SCOPE_PROPERTIES,
        LIBDAS_DAS_SCOPE_BUFFER,
        LIBDAS_DAS_SCOPE_MODEL,
        LIBDAS_DAS_SCOPE_ANIMATION,
        LIBDAS_DAS_SCOPE_KEYFRAME,
        LIBDAS_DAS_SCOPE_SCENE,
        LIBDAS_DAS_SCOPE_SCENE_NODE,
        LIBDAS_DAS_SCOPE_SCENE_UNDEFINED
    };


    /**
     * Enumeral values to specify all unique value declaration types
     */
    enum DasUniqueValueType {
        // PROPERTIES
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_MODEL,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_AUTHOR,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_COPYRIGHT,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_MODDATE,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_COMPRESSION,

        // BUFFER
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_BUFFER_TYPE,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_DATA_LEN,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_DATA,

        // MODEL
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_INDEX_BUFFER_ID,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_INDEX_BUFFER_OFFSET,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_INDICES_COUNT,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_TEXTURE_ID,

        // ANIMATION
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_LENGTH,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_INTERPOLATION,

        // KEYFRAME
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_TIMESTAMP,

        // NODE
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_CHILDREN_COUNT,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_CHILDREN,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_MODEL_COUNT,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_MODELS,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_ANIMATION_COUNT,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_ANIMATIONS,

        // Not so unique value types, since these values can be present in multiple scopes
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_NAME,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_TRANSFORM,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_BUFFER_ID,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_BUFFER_OFFSET,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_TEXTURE_MAP_BUFFER_ID,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_TEXTURE_MAP_BUFFER_OFFSET,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_NORMAL_BUFFER_ID,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_NORMAL_BUFFER_OFFSET,

        // reserved value
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_UNKNOWN
    };


    /**
     * Parent class to read data from DAS file format
     */
    class DasReaderCore : private AsciiLineReader {
        private:
            const std::string &m_file_name;
            std::ifstream m_in_stream;
            BinaryFormatErrorHandler m_error;

            // scope parsing maps
            std::unordered_map<std::string, DasScopeType> m_scope_name_map;
            std::unordered_map<std::string, DasUniqueValueType> m_unique_val_map;

        private:
            /**
             * Create scope name value hashmap for efficient type lookup
             */
            void _CreateScopeNameMap();
            /**
             * Create scope value non-buffer types sizes' map
             */
            void _CreateScopeValueTypeMap();
            /**
             * Parse the next scope name and retrieve the scope type
             * @return DasScopeType value to specify recently parsed scope type
             */
            DasScopeType _FindScopeType();
            /**
             * Get the unique value type from specified value string
             * @param _value is a string value specifying the value declaration 
             * @return DasUniqueValueType enumeral, that defines the unique value type
             */
            DasUniqueValueType _FindUniqueValueType(const std::string &_value);
            /**
             * Parse the next value declaration and retrieve information about the type and size of the value
             * @param _parent is a parent scope type
             * @param _value is a string value declaration name that is used
             * @return pair value specifying the value type and value length respectively
             */
            std::pair<std::any, size_t> _GetValueInformation(const DasScopeType _parent, const std::string &_value);

        public:
            DasReaderCore(const std::string &_file_name = "");
            /**
             * Open a new file for reading and close previous one if needed
             * @param _file_name is a specified file name to use for file streams
             */
            void NewFile(const std::string &_file_name);
            /**
             * Close previously opened stream if necessary
             */
            void CloseStream();
            /**
             * Read and verify file signature
             */
            void ReadSignature();
            /**
             * Read file properties and return their instance
             * @return DasProperties value defining the properties for given file
             */
           DasProperties ReadProperties();

           // incomplete list ...
    };
}

#endif
