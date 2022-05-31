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
    #include <Points.h>
    #include <Quaternion.h>
    #include <LibdasAssert.h>
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
            std::vector<DasMeshPrimitive> m_mesh_primitives;
            std::vector<DasMorphTarget> m_morph_targets;
            std::vector<DasNode> m_nodes;
            std::vector<DasScene> m_scenes;
            std::vector<DasSkeletonJoint> m_joints;
            std::vector<DasSkeleton> m_skeletons;
            std::vector<DasAnimationChannel> m_channels;
            std::vector<DasAnimation> m_animations;

        private:
            /**
             * Cast scope stored in std::any object into real scope value
             * @param _any_scope is any reference object that will be casted into correct data type
             * @param _type is specified scope type 
             */
            void _DataCast(std::any &_any_scope, DasScopeType _type);

            /**
             * Find root nodes from given scene
             * @param _id is a reference to the DasScene object whose root nodes will be calculated
             */
            void _FindSceneNodeRoots(DasScene &_scene);

        public:
            DasParser(const std::string &_file_name = "");
            DasParser(DasParser &&_parser) noexcept;

            /**
             * Parse contents from provided DAS file into scene array.
             * If the file contains no scenes, a default scene will be created that should be considered as a
             * object library.
             * @param _clean_read is an optional argument when set to true, closes the file stream currently used
             * @param _file_name is an optional argument that specifies new file to use
             */
            void Parse(bool _clean_read = false, const std::string &_file_name = "");

            ////////////////////////////////
            // ***** Getter methods ***** //
            ////////////////////////////////
            /**
             * Get all specified file properties
             * @return reference to DasProperties object that specifies all parsed properties
             */
            inline const DasProperties &GetProperties() const { 
                return m_props; 
            }

            /**
             * Access buffer reference by id
             * @param _id specifies the buffer id to use for accessing
             * @return reference to DasBuffer object, accessed by the id
             */
            inline const DasBuffer &AccessBuffer(uint32_t _id) const { 
                LIBDAS_ASSERT(_id < static_cast<uint32_t>(m_buffers.size()));
                return m_buffers[_id];
            }

            /**
             * Access mesh primitive by id
             * @param _id specifies the mesh primitive id to use for accessing
             * @return reference to DasMeshPrimitive instance, accessed by the id
             */
            inline const DasMeshPrimitive &AccessMeshPrimitive(uint32_t _id) const {
                LIBDAS_ASSERT(_id < static_cast<uint32_t>(m_mesh_primitives.size()));
                return m_mesh_primitives[_id];
            }

            /**
             * Access morph target by id
             * @param _id specifies the morph target id to use for accessing
             * @return reference to DasMorphTarget instance, accessed by the id
             */
            inline const DasMorphTarget &AccessMorphTarget(uint32_t _id) const {
                LIBDAS_ASSERT(_id < static_cast<uint32_t>(m_morph_targets.size()));
                return m_morph_targets[_id];
            }

            /**
             * Access mesh reference by id
             * @param _id specifies the mesh id to use for accessing
             * @return reference to DasMesh, accessed by the id
             */
            inline const DasMesh &AccessMesh(uint32_t _id) const { 
                LIBDAS_ASSERT(_id < static_cast<uint32_t>(m_meshes.size()));
                return m_meshes[_id]; 
            }

            /**
             * Access animation channel reference by id
             * @param _id specifyies the animation channel id to use for accessing
             * @return reference to DasAnimationChannel instance, accessed by the id
             */
            inline const DasAnimationChannel &AccessAnimationChannel(uint32_t _id) const {
                LIBDAS_ASSERT(_id < static_cast<uint32_t>(m_channels.size()));
                return m_channels[_id];
            }

            /**
             * Access animation reference by id
             * @param _id specifies the animation id to use for accessing
             * @return reference to DasAnimation, accessed by the id
             */
            inline const DasAnimation &AccessAnimation(uint32_t _id) const { 
                LIBDAS_ASSERT(_id < static_cast<uint32_t>(m_animations.size()));
                return m_animations[_id]; 
            }

            /**
             * Access a scene node by id
             * @param _id specifies the scene node id to use for accessing
             * @return reference to DasNode object, accessed by the id
             */
            inline const DasNode &AccessNode(uint32_t _id) const {
                LIBDAS_ASSERT(_id < static_cast<uint32_t>(m_nodes.size()));
                return m_nodes[_id];
            }

            /**
             * Access a skeleton by id
             * @param _id specifies the skeleton id to use for accessing
             * @return reference to DasSkeleton object, accessed by the id
             */
            inline const DasSkeleton &AccessSkeleton(uint32_t _id) const {
                LIBDAS_ASSERT(_id < static_cast<uint32_t>(m_skeletons.size()));
                return m_skeletons[_id];
            }

            /**
             * Access a scene by id
             * @param _id specifies the scene id to use for accessing
             * @return reference to DasScene object, accessed by the id
             */
            inline const DasScene &AccessScene(uint32_t _id) const {
                LIBDAS_ASSERT(_id < static_cast<uint32_t>(m_scenes.size()));
                return m_scenes[_id];
            }

            /**
             * Access a skeleton joint by id
             * @param _id specifies the skeleton joint id to use for accessing
             * @return reference to DasSkeletonJoint object, accessed by the id
             */
            inline const DasSkeletonJoint &AccessSkeletonJoint(uint32_t _id) const {
                LIBDAS_ASSERT(_id < static_cast<uint32_t>(m_joints.size()));
                return m_joints[_id];
            }

            /**
             * Get the amount of scenes that were parsed
             * @return uint32_t value specifying the scene count
             */
            inline uint32_t GetSceneCount() const {
                return static_cast<uint32_t>(m_scenes.size());
            }

            /**
             * Get the amount of buffers that were parsed 
             * @return uint32_t value specifying the buffer count
             */
            inline uint32_t GetBufferCount() const {
                return static_cast<uint32_t>(m_buffers.size());
            }

            /**
             * Get the amount of mesh primitives that were parsed
             * @return uint32_t value specifying the mesh primitive count
             */
            inline uint32_t GetMeshPrimitiveCount() const {
                return static_cast<uint32_t>(m_mesh_primitives.size());
            }

            /**
             * Get the amount of morph targets that were parsed
             * @return uint32_t value specifying the morph target count
             */
            inline uint32_t GetMorphTargetCount() const {
                return static_cast<uint32_t>(m_morph_targets.size());
            }

            /**
             * Get the amount of meshes that were parsed
             * @return uint32_t value specifying the model count
             */
            inline uint32_t GetMeshCount() const {
                return static_cast<uint32_t>(m_meshes.size());
            }

            /**
             * Get the amount of nodes that were parsed
             * @return uint32_t value specifying the node count
             */
            inline uint32_t GetNodeCount() const {
                return static_cast<uint32_t>(m_nodes.size());
            }
            
            /**
             * Get the amount of skeletons that were parsed
             * @return uint32_t value specifying the skeleton count
             */
            inline uint32_t GetSkeletonCount() const {
                return static_cast<uint32_t>(m_skeletons.size());
            }

            /**
             * Get the amount of skeleton joints that were parsed
             * @return uint32_t value specifying the skeleton joint count
             */
            inline uint32_t GetSkeletonJointCount() const {
                return static_cast<uint32_t>(m_joints.size());
            }

            /**
             * Get the amount of animation channels that were parsed
             * @return uint32_t value specifying the animation channel count
             */
            inline uint32_t GetAnimationChannelCount() const {
                return static_cast<uint32_t>(m_channels.size());
            }

            /**
             * Get the amount of animations that were parsed
             * @return uint32_t value specifying the animation count
             */
            inline uint32_t GetAnimationCount() const {
                return static_cast<uint32_t>(m_animations.size());
            }
    };
}

#endif
