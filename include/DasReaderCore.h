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
        LIBDAS_DAS_SCOPE_UNDEFINED,
        LIBDAS_DAS_SCOPE_END
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
    class DasReaderCore : public AsciiLineReader {
        private:
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
             * Get the unique value type from specified value string
             * @param _value is a string value specifying the value declaration 
             * @return DasUniqueValueType enumeral, that defines the unique value type
             */
            DasUniqueValueType _FindUniqueValueType(const std::string &_value);
            /**
             * Parse the next value declaration and retrieve information about the type and size of the value
             * @param _parent is a parent scope type
             * @param _value is a string value declaration name that is used
             * @return std::any value specifying any scope value type 
             */
            std::any _GetValueInformation(const DasScopeType _parent, const std::string &_value);

            ////////////////////////////////////////////////
            // ***** Property value reading methods ***** //
            ////////////////////////////////////////////////

            /**
             * Read properties scope value according to the value type
             * @param _props is a reference to DasProperties instance, where all data is stored
             * @param _type is a type value specifying the current value type that is read
             */
            void _ReadPropertiesValue(DasProperties *_props, DasProperties::ValueType _type);
            /**
             * Read buffer scope value according to the value type 
             * @param _buffer is a reference to DasBuffer instance, where all data is stored
             * @param _type is a type value specifying the current value type
             */
            void _ReadBufferValue(DasBuffer *_buffer, DasBuffer::ValueType _type);
            /**
             * Read model scope value according to the specified value type
             * @param _model is a reference to DasModel instance, where all data is stored
             * @param _type is a type value specifying the current value type
             */
            void _ReadModelValue(DasModel *_model, DasModel::ValueType _type);
            /**
             * Read animation scope value according to the specified value type
             * @param _animation is a reference to DasAnimation instance, where all data is stored
             * @param _type is a type value specifying the current value type
             */
            void _ReadAnimationValue(DasAnimation *_animation, DasAnimation::ValueType _type);
            /**
             * Read keyframe scope value according to the specified value type
             * @param _keyframe is a reference to DasKeyframe instance, where all data is stored
             * @param _type is a value specifying the current value type
             */
            void _ReadKeyframeValue(DasKeyframe *_keyframe, DasKeyframe::ValueType _type);
            /**
             * Read scene scope value according to the specified value type
             * @param _scene is a reference to DasScene instance, where all data is stored
             * @param _type is a value specifying the current value type
             */
            void _ReadSceneValue(DasScene *_scene, DasScene::ValueType _type);
            /**
             * Read scene node value according to the specified value type
             * @param _node is a reference to DasSceneNode instance, where all data is stored
             * @param _type is a value specifying the current value type
             */
            void _ReadSceneNodeValue(DasSceneNode *_node, DasSceneNode::ValueType _type);
            /**
             * Call correct scope value data reader method according to the type specified
             * @param _scope specifies any _scope value that is read
             * @param _type specifies that scope type
             * @param _value_type specifies the scope value type
             */
            void _ReadScopeValueDataCaller(std::any &_scope, DasScopeType _type, std::any &_value_type);
            /**
             * Verify parsed subscope and throw an error if the subscope does not belong to the parent scope
             * @param _type specifes the parent scope type
             * @param _sub_type specifies the subscope type
             * @param _scope is any parent scope data structure reference 
             * @param _sub_scope is any subscope data structure reference
             */
            void _VerifySubScope(DasScopeType _type, DasScopeType _sub_type, const std::any &_scope, const std::any &_sub_scope);
            /**
             * Retrieve correct initialised scope structure in std::any format
             * @param _type specifies the scope type
             * @return std::any instance specifying the structure value
             */
            std::any _GetAnyScopeStructure(DasScopeType _type);

        public:
            DasReaderCore(const std::string &_file_name = "");
            /**
             * Read and verify file signature
             */
            void ReadSignature();
            /**
             * Read scope values and return its instance
             * @param _type specifies the current scope type
             * @return DasProperties value defining the properties for given file
             */
            std::any ReadScopeData(DasScopeType _type);
            /**
             * Read new scope declaration and return its enumeral value
             * @return DasScopeType value that specifies currently parsed scope
             */
            DasScopeType ParseScopeDeclaration(const std::string &_scope_str = "");

           // incomplete list ...
    };
}

#endif
