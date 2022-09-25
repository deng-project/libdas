// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: DasReaderCore.h - DAS file format reader class header
// author: Karl-Mihkel Ott

#ifndef DAS_READER_CORE_H
#define DAS_READER_CORE_H


#ifdef DAS_READER_CORE_CPP
    #include <any>
    #include <cstring>
    #include <climits>
    #include <cmath>
    #include <string>
    #include <fstream>
#ifdef __DEBUG
    #include <iostream>
#endif
    #include <vector>
    #include <unordered_map>
    
    #include "trs/Iterators.h"
    #include "trs/Vector.h"
    #include "trs/Matrix.h"
    #include "trs/Points.h"
    #include "trs/Quaternion.h"

    #include "mar/AsciiStreamReader.h"
    #include "mar/AsciiLineReader.h"

    #include "das/Api.h"
    #include "das/DasStructures.h"
    #include "das/LibdasAssert.h"
    #include "das/ErrorHandlers.h"
#endif


namespace Libdas {

    /**
     * Enumeral values to specify all possible das scopes
     */
    enum DasScopeType {
        LIBDAS_DAS_SCOPE_PROPERTIES,
        LIBDAS_DAS_SCOPE_BUFFER,
        LIBDAS_DAS_SCOPE_MESH_PRIMITIVE,
        LIBDAS_DAS_SCOPE_MORPH_TARGET,
        LIBDAS_DAS_SCOPE_MESH,
        LIBDAS_DAS_SCOPE_NODE,
        LIBDAS_DAS_SCOPE_SCENE,
        LIBDAS_DAS_SCOPE_SKELETON_JOINT,
        LIBDAS_DAS_SCOPE_SKELETON,
        LIBDAS_DAS_SCOPE_ANIMATION,
        LIBDAS_DAS_SCOPE_ANIMATION_CHANNEL,
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
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_DEFAULT_SCENE,

        // BUFFER
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_BUFFER_TYPE,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_DATA_LEN,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_DATA,

        // MESH
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_PRIMITIVE_COUNT,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_PRIMITIVES,

        // MESHPRIMITIVE
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_INDEX_BUFFER_ID,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_INDEX_BUFFER_OFFSET,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_INDICES_COUNT,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_TEXTURE_COUNT,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_TEXTURE_IDS,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_COLOR_MUL_COUNT,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_JOINT_SET_COUNT,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_JOINT_INDEX_BUFFER_IDS,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_JOINT_INDEX_BUFFER_OFFSETS,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_JOINT_WEIGHT_BUFFER_IDS,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_JOINT_WEIGHT_BUFFER_OFFSETS,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_MORPH_TARGET_COUNT,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_MORPH_TARGETS,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_MORPH_WEIGHTS,

        // NODE
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_MESH,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_SKELETON,

        // SCENE
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_NODE_COUNT,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_NODES,

        // SKELETON
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_PARENT,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_JOINT_COUNT,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_JOINTS,

        // JOINT
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_INVERSE_BIND_POS,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_SCALE,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_ROTATION,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_TRANSLATION,

        // ANIMATION
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_CHANNEL_COUNT,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_CHANNELS,

        // ANIMATIONCHANNEL
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_NODE_ID,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_JOINT_ID,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_TARGET,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_INTERPOLATION,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_KEYFRAME_COUNT,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_WEIGHT_COUNT,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_KEYFRAMES,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_TANGENTS,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_TARGET_VALUES,

        // Not so unique value types, since these values can be present in multiple scopes
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_NAME,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_TRANSFORM,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_BUFFER_ID,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_BUFFER_OFFSET,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_UV_BUFFER_IDS,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_UV_BUFFER_OFFSETS,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_COLOR_MUL_BUFFER_IDS,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_COLOR_MUL_BUFFER_OFFSETS,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_NORMAL_BUFFER_ID,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_NORMAL_BUFFER_OFFSET,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_TANGENT_BUFFER_ID,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_VERTEX_TANGENT_BUFFER_OFFSET,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_CHILDREN_COUNT,
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_CHILDREN,

        // reserved value
        LIBDAS_DAS_UNIQUE_VALUE_TYPE_UNKNOWN
    };



    /**
     * Parent class to read data from DAS file format
     */
    class LIBDAS_API DasReaderCore : public MAR::AsciiLineReader {
        private:
            BinaryFormatErrorHandler m_error;

            // scope parsing maps
            std::unordered_map<std::string, DasScopeType> m_scope_name_map;
            std::unordered_map<std::string, DasUniqueValueType> m_unique_val_map;
            std::vector<char*> m_buffer_blobs;

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
             * Read mesh primitive value scope according to the specified value type
             * @param _primitive is a valid pointer to DasMeshPrimitive instance
             * @param _type is a type value specifying the current value type
             */
            void _ReadMeshPrimitiveValue(DasMeshPrimitive *_primitive, DasMeshPrimitive::ValueType _type);
            /**
             * Read morph target scope value according to the specified value type
             * @param _morph_target is a valid pointer to DasMorphTarget instance
             * @param _type is a type value specifying the current value type
             */
            void _ReadMorphTargetValue(DasMorphTarget *_morph_target, DasMorphTarget::ValueType _type);
            /**
             * Read mesh scope value according to the specified value type
             * @param _mesh is a valid pointer to DasMesh instance, where all data is stored
             * @param _type is a type value specifying the current value type
             */
            void _ReadMeshValue(DasMesh *_mesh, DasMesh::ValueType _type);
            /**
             * Read node scope value according to the the specified value type
             * @param _node is a valid pointer to DasNode instance, where all data is stored
             * @param _type is a type value specifying the current value type
             */
            void _ReadNodeValue(DasNode *_node, DasNode::ValueType _type);
            /**
             * Read scene scope value according to the specified value type
             * @param _scene is a valid pointer to DasScene instance, where all data is stored
             * @param _type is a value specifying the current value type
             */
            void _ReadSceneValue(DasScene *_scene, DasScene::ValueType _type);
            /**
             * Read specified value about skeleton joint from its scope
             * @param _joint is a valid pointer to DasSkeletonJoint instance, where all data is stored
             * @param _type is a value specifying the current value type
             */
            void _ReadSkeletonJointValue(DasSkeletonJoint *_joint, DasSkeletonJoint::ValueType _type);
            /**
             * Read specified value about skeleton from its scope
             * @param _skeleton is a valid pointer to DasSkeleton instance, where all data is stored
             * @param _type is a value specifying the current value type
             */
            void _ReadSkeletonValue(DasSkeleton *_skeleton, DasSkeleton::ValueType _type);
            /**
             * Read animation scope value according to the specified value type
             * @param _animation is a reference to DasAnimation instance, where all data is stored
             * @param _type is a type value specifying the current value type
             */
            void _ReadAnimationValue(DasAnimation *_animation, DasAnimation::ValueType _type);
            /**
             * Read animation channel scope value according to the specified value type
             * @param _channel is a reference to DasAnimationChannel instance
             * @param _type is a type value specifying the current value type
             */
            void _ReadAnimationChannelValue(DasAnimationChannel *_channel, DasAnimationChannel::ValueType _type);
            /**
             * Call correct scope value data reader method according to the type specified
             * @param _scope specifies any _scope value that is read
             * @param _type specifies that scope type
             * @param _value_type specifies the scope value type
             * @param _val_str specifies the value string that was read
             */
            void _ReadScopeValueDataCaller(std::any &_scope, DasScopeType _type, std::any &_value_type, const std::string &_val_str);
            /**
             * Retrieve correct initialised scope structure in std::any format
             * @param _type specifies the scope type
             * @return std::any instance specifying the structure value
             */
            inline std::any _GetAnyScopeStructure(DasScopeType _type);

        protected:
            /**
             * Clear all pointers from m_buffer_blobs, so they won't get deleted on destruction
             */
            inline void _ClearBlobs() {
                m_buffer_blobs.clear();
            }

        public:
            DasReaderCore(const std::string &_file_name = "");
            DasReaderCore(DasReaderCore &&_drc) noexcept;
            ~DasReaderCore();
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
            /**
             * Close the file stream and delete heap allocated maps
             */
            void Clear();
    };
}

#endif
