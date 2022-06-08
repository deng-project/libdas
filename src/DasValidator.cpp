// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: DasValidator.cpp - DAS file validation class implementation
// author: Karl-Mihkel Ott


#define DAS_VALIDATOR_CPP
#include <DasValidator.h>

namespace Libdas {
    
    DasValidator::DasValidator(DasParser &_parser, bool _validate) : m_parser(_parser) {
        if(_validate) Validate();
    }


    DasValidator::DasValidator(DasValidator &&_val) noexcept :
        m_parser(_val.m_parser),
        m_warning_stack(_val.m_warning_stack),
        m_error_stack(_val.m_error_stack) {}


    void DasValidator::_CheckJointProperties(const DasMeshPrimitive &_prim, uint32_t _cur_index, uint32_t _max_index) {
        // check joint set properties
        if(_prim.joint_set_count) {
            for(uint32_t i = 0; i < _prim.joint_set_count; i++) {
                // indices
                if(_prim.joint_index_buffer_ids[i] >= m_parser.GetBufferCount()) {
                    std::string errme = "DAS validation error: Invalid joint index buffer id " + std::to_string(_prim.joint_index_buffer_ids[i]) + " for mesh primitive " + std::to_string(_cur_index);
                    m_error_stack.push(errme);
                } else if(m_parser.AccessBuffer(_prim.joint_index_buffer_ids[i]).data_len < _prim.joint_index_buffer_offsets[i] + _max_index * static_cast<uint32_t>(sizeof(Libdas::Vector4<uint16_t>))) {
                    std::string errme = "DAS validation error: Invalid joint index buffer(" + std::to_string(_prim.joint_index_buffer_ids[i]) +
                                        ") region with offset " + std::to_string(_prim.joint_index_buffer_offsets[i]) + " and size " + 
                                        std::to_string(_max_index * static_cast<uint32_t>(sizeof(Libdas::Vector4<uint16_t>))) + 
                                        " for mesh primitive " + std::to_string(_cur_index);
                    m_error_stack.push(errme);
                }

                // weights
                if(_prim.joint_weight_buffer_ids[i] >= m_parser.GetBufferCount()) {
                    std::string errme = "DAS validation error: Invalid joint weight buffer id " + std::to_string(_prim.joint_weight_buffer_ids[i]) + " for mesh primitive " + std::to_string(_cur_index);
                    m_error_stack.push(errme);
                } else if(m_parser.AccessBuffer(_prim.joint_weight_buffer_ids[i]).data_len < _prim.joint_weight_buffer_offsets[i] + _max_index * static_cast<uint32_t>(sizeof(Libdas::Vector4<float>))) {
                    std::string errme = "DAS validation error: Invalid joint weight buffer(" + std::to_string(_prim.joint_weight_buffer_ids[i]) +
                                        ") region with offset " + std::to_string(_prim.joint_weight_buffer_offsets[i]) + " and size " +
                                        std::to_string(_max_index * static_cast<uint32_t>(sizeof(Libdas::Vector4<float>))) +
                                        " for mesh primitive " + std::to_string(i); 
                    m_error_stack.push(errme);
                }
            }
        }
    }


    void DasValidator::_CheckMorphTargetIndices(const DasMeshPrimitive &_prim, uint32_t _cur_index) {
        // for each morph target in mesh primitive
        for(uint32_t i = 0; i < _prim.morph_target_count; i++) {
            // check if morph target index is valid
            if(_prim.morph_targets[i] >= m_parser.GetMorphTargetCount()) {
                const std::string errme = "DAS validation error: Invalid morph target index " + std::to_string(_prim.morph_targets[i]) +
                                          " for mesh primitive " + std::to_string(_cur_index);
                m_error_stack.push(errme);
                m_critical_bit = true;
            }
        }
    }


    void DasValidator::_CheckMeshPrimitiveIndicesContinuity(const DasMeshPrimitive &_prim, uint32_t _cur_index) {
        const DasBuffer &ibuffer = m_parser.AccessBuffer(_prim.index_buffer_id);
        const uint32_t *ptr = reinterpret_cast<const uint32_t*>(ibuffer.data_ptrs.back().first + _prim.index_buffer_offset);

        std::vector<uint32_t> sorted_indices;
        sorted_indices.reserve(_prim.indices_count);
        sorted_indices.insert(sorted_indices.begin(), ptr, ptr + _prim.indices_count);
        std::sort(sorted_indices.begin(), sorted_indices.end(), std::less());

        // continuity check
        bool is_continuous = true;
        for(size_t i = 0; i < sorted_indices.size() - 2; i++) {
            if(sorted_indices[i + 1] - sorted_indices[i] > 1) {
                is_continuous = false;
                break;
            }
        }

        if(!is_continuous) {
            const std::string warnme = "DAS validation warning: Indices are not continuous for mesh primitive " + std::to_string(_cur_index);
            m_warning_stack.push(warnme);
        }
    }


    void DasValidator::_VerifyProperties() {
        const DasProperties &props = m_parser.GetProperties();
        if(props.default_scene >= m_parser.GetSceneCount()) {
            std::string errme = "DAS validation error: Invalid default scene index " + std::to_string(props.default_scene);
            m_error_stack.push(errme);
            m_critical_bit = true;
        }
    }


    void DasValidator::_VerifyMeshes() {
        // check if mesh primitive indices are correct (2.1)
        for(uint32_t i = 0; i < m_parser.GetMeshCount(); i++) {
            const DasMesh &mesh = m_parser.AccessMesh(i);
            for(uint32_t j = 0; j < mesh.primitive_count; j++) {
                if(mesh.primitives[j] >= m_parser.GetMeshPrimitiveCount()) {
                    const std::string errme = "DAS validation error: Invalid mesh primitive id " + std::to_string(mesh.primitives[j]) + 
                                              " for mesh " + std::to_string(i);
                    m_error_stack.push(errme);
                    m_critical_bit = true;
                }
            }
        }

        m_max_index_table.resize(m_parser.GetMeshPrimitiveCount());
        for(uint32_t i = 0; i < m_parser.GetMeshPrimitiveCount(); i++) {
            const DasMeshPrimitive &prim = m_parser.AccessMeshPrimitive(i);
            // check if indices data region is correct
            // 2.2
            if(prim.index_buffer_id >= m_parser.GetBufferCount()) {
                const std::string errme = "DAS validation error: Invalid index buffer id " + std::to_string(prim.index_buffer_id) + " for mesh primitive " + std::to_string(i);
                m_error_stack.push(errme);
            } else if(m_parser.AccessBuffer(prim.index_buffer_id).data_len < prim.index_buffer_offset + prim.indices_count * static_cast<uint32_t>(sizeof(uint32_t))) {
                const std::string errme = "DAS validation error: Invalid index buffer(" + std::to_string(prim.index_buffer_id) + 
                                          ") region with offset " + std::to_string(prim.index_buffer_offset) + "and size " + 
                                          std::to_string(prim.indices_count * static_cast<uint32_t>(sizeof(uint32_t))) + 
                                          " for mesh primitive " + std::to_string(i);
                m_error_stack.push(errme);
            } else {
                // get the max index
                const DasBuffer &index_buffer = m_parser.AccessBuffer(prim.index_buffer_id);
                const uint32_t max_index = *std::max_element(index_buffer.data_ptrs.front().first + prim.index_buffer_offset, 
                                                             index_buffer.data_ptrs.front().first + prim.index_buffer_offset + prim.indices_count * static_cast<uint32_t>(sizeof(uint32_t))) + 1;
                m_max_index_table[i] = max_index;

                // 2.2
                _CheckPositionVertices(prim, i, max_index);
                _CheckVertexNormal(prim, i, max_index);
                _CheckVertexTangent(prim, i, max_index);
                _CheckTextureProperties(prim, i, max_index);
                _CheckColorMulProperties(prim, i, max_index);
                _CheckJointProperties(prim, i, max_index);

                // check morph targets (2.3)
                _CheckMorphTargetIndices(prim, i);

                // check indices continuity (2.4)
                _CheckMeshPrimitiveIndicesContinuity(prim, i);
            }
        }

        // check if all mesh primitives are used (2.5)
        std::vector<bool> used_table(m_parser.GetMeshPrimitiveCount());
        for(uint32_t i = 0; i < m_parser.GetMeshCount(); i++) {
            const DasMesh &mesh = m_parser.AccessMesh(i);
            for(uint32_t j = 0; j < mesh.primitive_count; j++) {
                used_table[mesh.primitives[j]] = true;
            }
        }

        // check for usage
        for(auto it = used_table.begin(); it != used_table.end(); it++) {
            if(!*it) {
                const size_t index = it - used_table.begin();
                const std::string warnme = "DAS validation warning: Unused mesh primitive " + std::to_string(index);
                m_warning_stack.push(warnme);
            }
        }
    }


    void DasValidator::_VerifyMorphTargets() {
        // check if all morph targets are used (3.1)
        std::vector<uint32_t> max_idx_table(m_parser.GetMorphTargetCount());
        std::fill(max_idx_table.begin(), max_idx_table.end(), UINT32_MAX);      // fill it with invalid value of UINT32_MAX

        for(uint32_t i = 0; i < m_parser.GetMeshPrimitiveCount(); i++) {
            const DasMeshPrimitive &prim = m_parser.AccessMeshPrimitive(i);
            for(uint32_t j = 0; j < prim.morph_target_count; j++) {
                if(max_idx_table[prim.morph_targets[j]] == UINT32_MAX || max_idx_table[prim.morph_targets[j]] < m_max_index_table[i]) 
                    max_idx_table[prim.morph_targets[j]] = m_max_index_table[i];
            }
        }

        for(auto it = max_idx_table.begin(); it != max_idx_table.end(); it++) {
            if(*it == UINT32_MAX) {
                const size_t index = it - max_idx_table.begin();
                const std::string warnme = "DAS validation warning: Unused morph target " + std::to_string(index);
                m_warning_stack.push(warnme);
            }
        }

        // check if morph target buffer regions are correct (3.2)
        for(uint32_t i = 0; i < m_parser.GetMorphTargetCount(); i++) {
            if(max_idx_table[i] == UINT32_MAX) 
                continue;

            const DasMorphTarget &morph = m_parser.AccessMorphTarget(i);
            _CheckPositionVertices(morph, i, max_idx_table[i]);
            _CheckVertexNormal(morph, i, max_idx_table[i]);
            _CheckVertexTangent(morph, i, max_idx_table[i]);
            _CheckTextureProperties(morph, i, max_idx_table[i]);
            _CheckColorMulProperties(morph, i, max_idx_table[i]);
        }
    }


    void DasValidator::_VerifyScenes() {
        std::vector<bool> consumed_table(m_parser.GetNodeCount());

        // check if scene node ids are correct (4.1)
        for(uint32_t i = 0; i < m_parser.GetSceneCount(); i++) {
            const DasScene &scene = m_parser.AccessScene(i);
            std::vector<uint32_t> node_nr_table(m_parser.GetNodeCount());

            for(uint32_t j = 0; j < scene.node_count; j++) {
                if(scene.nodes[j] >= m_parser.GetNodeCount()) {
                    const std::string errme = "DAS validation error: Invalid node id " + std::to_string(scene.nodes[j]) +
                                              " for scene " + std::to_string(i);
                    m_error_stack.push(errme);
                    m_critical_bit = true;
                } else {
                    node_nr_table[scene.nodes[j]]++;
                    consumed_table[scene.nodes[j]] = true;
                }
            }

            // check if there are matching nodes in the nodes array (4.2)
            for(auto it = node_nr_table.begin(); it != node_nr_table.end(); it++) {
                if(*it > 1) {
                    const size_t index = it - node_nr_table.begin();
                    const std::string warnme = "DAS validation warning: Scene node " + std::to_string(index) +
                                               " is used multiple times in scene " + std::to_string(i);
                    m_warning_stack.push(warnme);
                }
            }
        }

        // check if children are correct for each node (4.1)
        for(uint32_t i = 0; i < m_parser.GetNodeCount(); i++) {
            const DasNode &node = m_parser.AccessNode(i);
            for(uint32_t j = 0; j < node.children_count; j++) {
                if(node.children[j] >= m_parser.GetNodeCount()) {
                    const std::string errme = "DAS validation error: Invalid child node id " + std::to_string(node.children[j]) +
                                              " for node " + std::to_string(j);
                    m_error_stack.push(errme);
                    m_critical_bit = true;
                } else {
                    consumed_table[node.children[j]] = true;
                }
            }
        }

        if(m_critical_bit) 
            return;

        // check if all nodes are consumed as scene nodes or child nodes (4.3)
        for(auto it = consumed_table.begin(); it != consumed_table.end(); it++) {
            if(!*it) {
                const size_t index = it - consumed_table.begin();
                const std::string warnme = "DAS validation warning: Unused scene node " + std::to_string(index);
                m_warning_stack.push(warnme);
            }
        }
    }


    void DasValidator::_VerifySkeletons() {
        std::vector<bool> consumed_joint_table(m_parser.GetSkeletonJointCount());
        std::vector<uint32_t> joint_nr_table(m_parser.GetSkeletonJointCount());

        for(uint32_t i = 0; i < m_parser.GetSkeletonCount(); i++) {
            const DasSkeleton &skeleton = m_parser.AccessSkeleton(i);
            std::fill(joint_nr_table.begin(), joint_nr_table.end(), 0);

            // check nr 5.1
            if(skeleton.parent >= m_parser.GetSkeletonJointCount()) {
                const std::string errme = "DAS validation error: Invalid parent joint id " + std::to_string(skeleton.parent) +
                                          " for skeleton " + std::to_string(i);
                m_error_stack.push(errme);
                m_critical_bit = true;
            } else {
                joint_nr_table[skeleton.parent]++;
                consumed_joint_table[skeleton.parent] = true;
            }

            for(uint32_t j = 0; j < skeleton.joint_count; j++) {
                if(skeleton.joints[j] >= m_parser.GetSkeletonJointCount()) {
                    const std::string errme = "DAS validation error: Invalid joint id " + std::to_string(skeleton.joints[j]) +
                                              " for skeleton " + std::to_string(i);
                    m_error_stack.push(errme);
                    m_critical_bit = true;
                } else {
                    joint_nr_table[skeleton.joints[j]]++;
                    consumed_joint_table[skeleton.joints[j]] = true;
                }
            }

            // check if there are matching skeleton joint ids in the joints array (5.2)
            for(auto it = joint_nr_table.begin(); it != joint_nr_table.end(); it++) {
                if(*it > 1) {
                    const size_t index = it - joint_nr_table.begin();
                    const std::string warnme = "DAS validation error: Joint node " + std::to_string(index) + 
                                               " is used multiple times by skeleton " + std::to_string(i);
                    m_warning_stack.push(warnme);
                }
            }
        }

        // check if all joints are consumed by skeletons (5.3)
        for(auto it = consumed_joint_table.begin(); it != consumed_joint_table.end(); it++) {
            if(!*it) {
                const size_t index = it - consumed_joint_table.begin();
                const std::string warnme = "DAS validation warning: Unused skeleton joint " + std::to_string(index);
                m_warning_stack.push(warnme);
            }
        }
    }


    void DasValidator::Validate() {
        _VerifyProperties();
        if(m_critical_bit)
            return;
        _VerifyMeshes();
        if(m_critical_bit) 
            return;
        _VerifyMorphTargets();
        _VerifyScenes();
        if(m_critical_bit) 
            return;
        _VerifySkeletons();
    }
}
