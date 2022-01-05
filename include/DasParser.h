// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: DasParser.h - DAS format parser header
// author: Karl-Mihkel Ott

#ifndef DAS_PARSER_H
#define DAS_PARSER_H

#ifdef DAS_PARSER_CPP
    #include <any>
    #include <fstream>
    #include <vector>
    #include <string>
    #include <cstring>
    #include <cmath>
    #include <iostream>
    #include <unordered_map>

    #include <Api.h>
    #include <Vector.h>
    #include <Matrix.h>
    #include <Quaternion.h>
    #include <ErrorHandlers.h>
    #include <AsciiStreamReader.h>
    #include <AsciiLineReader.h>
    #include <DasStructures.h>
    #include <DasReaderCore.h>
#endif

namespace Libdas {

    class LIBDAS_API DasParser : private DasReaderCore {
        private:
            DasProperties m_props;
            std::vector<DasBuffer> m_buffers;
            std::vector<DasMesh> m_meshes;
            std::vector<DasNode> m_nodes;
            std::vector<DasScene> m_scenes;
            std::vector<DasSkeletonJoint> m_joints;
            std::vector<DasSkeleton> m_skeletons;
            std::vector<DasAnimation> m_animations;

        private:
            /**
             * Cast scope stored in std::any object into real scope value
             * @param _any_scope is any reference object that will be casted into correct data type
             * @param _type is specified scope type 
             */
            void _DataCast(std::any &_any_scope, DasScopeType _type);

        public:
            DasParser(const std::string &_file_name = "");
            /**
             * Parse contents from provided DAS file into scene array.
             * If the file contains no scenes, a default scene will be created that should be considered as a
             * object library.
             * @param _file_name is an optional argument that specifies new file to use
             */
            void Parse(const std::string &_file_name = "");

            ////////////////////////////////
            // ***** Getter methods ***** //
            ////////////////////////////////
            /**
             * Get all specified file properties
             * @return reference to DasProperties object that specifies all parsed properties
             */
            inline DasProperties &GetProperties() { 
                return m_props; 
            }

            /**
             * Access buffer reference by id
             * @param _id specifies the buffer id to use for accessing
             * @return reference to DasBuffer object, accessed by the id
             */
            inline DasBuffer &AccessBuffer(uint32_t _id) { 
                return m_buffers[_id % static_cast<uint32_t>(m_buffers.size())];
            }

            /**
             * Access mesh reference by id
             * @param _id specifies the model id to use for accessing
             * @return reference to DasMesh, accessed by the id
             */
            inline DasMesh &AccessMesh(uint32_t _id) { 
                return m_meshes[_id % static_cast<uint32_t>(m_meshes.size())]; 
            }

            /**
             * Access animation reference by id
             * @param _id specifies the animation id to use for accessing
             * @return reference to DasAnimation, accessed by the id
             */
            inline DasAnimation &AccessAnimation(uint32_t _id) { 
                return m_animations[_id % static_cast<uint32_t>(m_animations.size())]; 
            }

            /**
             * Access a scene node by id
             * @param _id specifies the scene node id to use for accessing
             * @return reference to DasNode object, accessed by the id
             */
            inline DasNode &AccessNode(uint32_t _id) {
                return m_nodes[_id % static_cast<uint32_t>(m_nodes.size())];
            }

            /**
             * Access a skeleton by id
             * @param _id specifies the skeleton id to use for accessing
             * @return reference to DasSkeleton object, accessed by the id
             */
            inline DasSkeleton &AccessSkeleton(uint32_t _id) {
                return m_skeletons[_id % static_cast<uint32_t>(m_skeletons.size())];
            }

            /**
             * Access a skeleton joint by id
             * @param _id specifies the skeleton joint id to use for accessing
             * @return reference to DasSkeletonJoint object, accessed by the id
             */
            inline DasSkeletonJoint &AccessSkeletonJoint(uint32_t _id) {
                return m_joints[_id % static_cast<uint32_t>(m_joints.size())];
            }

            /**
             * Get all scenes specified in the file
             * @return reference to std::vector object containing all scene details
             */
            inline std::vector<DasScene> &GetScenes() {
                return m_scenes;
            }

            /**
             * Get the amount of buffers that were parsed 
             * @return uint32_t value specifying the buffer count
             */
            inline uint32_t GetBufferCount() {
                return static_cast<uint32_t>(m_buffers.size());
            }

            /**
             * Get the amount of models that were parsed
             * @return uint32_t value specifying the model count
             */
            inline uint32_t GetMeshCount() {
                return static_cast<uint32_t>(m_meshes.size());
            }

            /**
             * Get the amount of nodes that were parsed
             * @return uint32_t value specifying the node count
             */
            inline uint32_t GetNodeCount() {
                return static_cast<uint32_t>(m_nodes.size());
            }
            
            /**
             * Get the amount of skeletons that were parsed
             * @param uint32_t value specifying the skeleton count
             */
            inline uint32_t GetSkeletonCount() {
                return static_cast<uint32_t>(m_skeletons.size());
            }

            /**
             * Get the amount of skeleton joints that were parsed
             * @param uint32_t value specifying the skeleton joint count
             */
            inline uint32_t GetSkeletonJointCount() {
                return static_cast<uint32_t>(m_joints.size());
            }

            /**
             * Get the amount of animations that were parsed
             * @return uint32_t value specifying the animation count
             */
            inline uint32_t GetAnimationCount() {
                return static_cast<uint32_t>(m_animations.size());
            }

    };
}

#endif
