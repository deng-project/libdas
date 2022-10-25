// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: DasValidator.cpp - DAS file validation class implementation
// author: Karl-Mihkel Ott


#define DAS_VALIDATOR_CPP
#include "das/DasValidator.h"

namespace Libdas {
    
    DasValidator::DasValidator(DasModel &_model, bool _validate) : 
        m_model(_model) 
    {
        if(_validate) Validate();
    }


    DasValidator::DasValidator(DasValidator &&_val) noexcept :
        m_model(std::move(_val.m_model)),
        m_warning_stack(std::move(_val.m_warning_stack)),
        m_error_stack(std::move(_val.m_error_stack)) {}


    void DasValidator::_CheckJointProperties(const DasMeshPrimitive &_prim, uint32_t _cur_index, uint32_t _max_index) {
        // check joint set properties
        if(_prim.joint_set_count) {
            for(uint32_t i = 0; i < _prim.joint_set_count; i++) {
                // indices
                if(_prim.joint_index_buffer_ids[i] >= (uint32_t) m_model.buffers.size()) {
                    std::string errme = "DAS validation error: Invalid joint index buffer id " + std::to_string(_prim.joint_index_buffer_ids[i]) + " for mesh primitive " + std::to_string(_cur_index);
                    m_error_stack.push(errme);
                } else if(m_model.buffers[_prim.joint_index_buffer_ids[i]].data_len < _prim.joint_index_buffer_offsets[i] + _max_index * static_cast<uint32_t>(sizeof(TRS::Vector4<uint16_t>))) {
                    std::string errme = "DAS validation error: Invalid joint index buffer(" + std::to_string(_prim.joint_index_buffer_ids[i]) +
                                        ") region with offset " + std::to_string(_prim.joint_index_buffer_offsets[i]) + " and size " + 
                                        std::to_string(_max_index * static_cast<uint32_t>(sizeof(TRS::Vector4<uint16_t>))) + 
                                        " for mesh primitive " + std::to_string(_cur_index);
                    m_error_stack.push(errme);
                }

                // weights
                if(_prim.joint_weight_buffer_ids[i] >= (uint32_t) m_model.buffers.size()) {
                    std::string errme = "DAS validation error: Invalid joint weight buffer id " + std::to_string(_prim.joint_weight_buffer_ids[i]) + " for mesh primitive " + std::to_string(_cur_index);
                    m_error_stack.push(errme);
                } else if(m_model.buffers[_prim.joint_weight_buffer_ids[i]].data_len < _prim.joint_weight_buffer_offsets[i] + _max_index * static_cast<uint32_t>(sizeof(TRS::Vector4<float>))) {
                    std::string errme = "DAS validation error: Invalid joint weight buffer(" + std::to_string(_prim.joint_weight_buffer_ids[i]) +
                                        ") region with offset " + std::to_string(_prim.joint_weight_buffer_offsets[i]) + " and size " +
                                        std::to_string(_max_index * static_cast<uint32_t>(sizeof(TRS::Vector4<float>))) +
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
            if(_prim.morph_targets[i] >= (uint32_t) m_model.morph_targets.size()) {
                const std::string errme = "DAS validation error: Invalid morph target index " + std::to_string(_prim.morph_targets[i]) +
                                          " for mesh primitive " + std::to_string(_cur_index);
                m_error_stack.push(errme);
                m_critical_bit = true;
            }
        }
    }


    void DasValidator::_CheckMeshPrimitiveIndicesContinuity(const DasMeshPrimitive &_prim, uint32_t _cur_index) {
        const DasBuffer& ibuffer = m_model.buffers[_prim.index_buffer_id];
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
        const DasProperties &props = m_model.props;
        if(props.default_scene >= (uint32_t)m_model.scenes.size()) {
            std::string errme = "DAS validation error: Invalid default scene index " + std::to_string(props.default_scene);
            m_error_stack.push(errme);
            m_critical_bit = true;
        }
    }


    void DasValidator::_VerifyMeshes() {
        // check if mesh primitive indices are correct (2.1)
        for (auto it = m_model.meshes.begin(); it != m_model.meshes.end(); it++) {
            for(uint32_t j = 0; j < it->primitive_count; j++) {
                if(it->primitives[j] >= (uint32_t)m_model.mesh_primitives.size()) {
                    const std::string errme = "DAS validation error: Invalid mesh primitive id " + std::to_string(it->primitives[j]) + 
                                              " for mesh " + std::to_string(it - m_model.meshes.begin());
                    m_error_stack.push(errme);
                    m_critical_bit = true;
                }
            }
        }

        m_max_index_table.resize(m_model.mesh_primitives.size());
        auto& prims = m_model.mesh_primitives;
        for (auto it = prims.begin(); it != prims.end(); it++) {
            const uint32_t index = static_cast<uint32_t>(it - prims.begin());
            // check if indices data region is correct
            // 2.2
            if (it->index_buffer_id >= (uint32_t)m_model.buffers.size()) {
                const std::string errme = "DAS validation error: Invalid index buffer id " + std::to_string(it->index_buffer_id) + " for mesh primitive " + std::to_string(index);
                m_error_stack.push(errme);
            } else if(m_model.buffers[it->index_buffer_id].data_len < it->index_buffer_offset + it->indices_count * static_cast<uint32_t>(sizeof(uint32_t))) {
                const std::string errme = "DAS validation error: Invalid index buffer(" + std::to_string(it->index_buffer_id) + 
                                          ") region with offset " + std::to_string(it->index_buffer_offset) + " and size " +
                                          std::to_string(it->indices_count * static_cast<uint32_t>(sizeof(uint32_t))) +
                                          " for mesh primitive " + std::to_string(index);
                m_error_stack.push(errme);
            } else {
                // get the max index
                const DasBuffer &index_buffer = m_model.buffers[it->index_buffer_id];
                const uint32_t max_index = *std::max_element(index_buffer.data_ptrs.front().first + it->index_buffer_offset, 
                                                             index_buffer.data_ptrs.front().first + it->index_buffer_offset + it->indices_count * static_cast<uint32_t>(sizeof(uint32_t))) + 1;
                m_max_index_table[index] = max_index;

                // 2.2
                _CheckPositionVertices(*it, index, max_index);
                _CheckVertexNormal(*it, index, max_index);
                _CheckVertexTangent(*it, index, max_index);
                _CheckTextureProperties(*it, index, max_index);
                _CheckColorMulProperties(*it, index, max_index);
                _CheckJointProperties(*it, index, max_index);

                // check morph targets (2.3)
                _CheckMorphTargetIndices(*it, index);

                // check indices continuity (2.4)
                _CheckMeshPrimitiveIndicesContinuity(*it, index);
            }
        }

        // check if all mesh primitives are used (2.5)
        std::vector<bool> used_table(m_model.mesh_primitives.size());
        for (auto it = m_model.meshes.begin(); it != m_model.meshes.end(); it++) {
            for(uint32_t j = 0; j < it->primitive_count; j++) {
                used_table[it->primitives[j]] = true;
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
        std::vector<uint32_t> max_idx_table(m_model.morph_targets.size());
        std::fill(max_idx_table.begin(), max_idx_table.end(), UINT32_MAX);      // fill it with invalid value of UINT32_MAX

        for (auto it = m_model.mesh_primitives.begin(); it != m_model.mesh_primitives.end(); it++) {
            for(uint32_t j = 0; j < it->morph_target_count; j++) {
                if(max_idx_table[it->morph_targets[j]] == UINT32_MAX || max_idx_table[it->morph_targets[j]] < m_max_index_table[it - m_model.mesh_primitives.begin()])
                    max_idx_table[it->morph_targets[j]] = m_max_index_table[it - m_model.mesh_primitives.begin()];
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
        for (auto it = m_model.morph_targets.begin(); it != m_model.morph_targets.end(); it++) {
            const uint32_t index = static_cast<uint32_t>(it - m_model.morph_targets.begin());
            if(max_idx_table[index] == UINT32_MAX)
                continue;

            _CheckPositionVertices(*it, index, max_idx_table[index]);
            _CheckVertexNormal(*it, index, max_idx_table[index]);
            _CheckVertexTangent(*it, index, max_idx_table[index]);
            _CheckTextureProperties(*it, index, max_idx_table[index]);
            _CheckColorMulProperties(*it, index, max_idx_table[index]);
        }
    }


    void DasValidator::_VerifyScenes() {
        std::vector<bool> consumed_table(m_model.nodes.size());

        // check if scene node ids are correct (4.1)
        for (auto it = m_model.scenes.begin(); it != m_model.scenes.end(); it++) {
            std::vector<uint32_t> node_nr_table(m_model.nodes.size());

            for(uint32_t j = 0; j < it->node_count; j++) {
                if(it->nodes[j] >= (uint32_t)m_model.nodes.size()) {
                    const std::string errme = "DAS validation error: Invalid node id " + std::to_string(it->nodes[j]) +
                                              " for scene " + std::to_string(it - m_model.scenes.begin());
                    m_error_stack.push(errme);
                    m_critical_bit = true;
                } else {
                    node_nr_table[it->nodes[j]]++;
                    consumed_table[it->nodes[j]] = true;
                }
            }

            // check if there are matching nodes in the nodes array (4.2)
            for(auto ndtbl_it = node_nr_table.begin(); ndtbl_it != node_nr_table.end(); ndtbl_it++) {
                if(*ndtbl_it > 1) {
                    const size_t index = ndtbl_it - node_nr_table.begin();
                    const std::string warnme = "DAS validation warning: Scene node " + std::to_string(index) +
                                               " is used multiple times in scene " + std::to_string(it - m_model.scenes.begin());
                    m_warning_stack.push(warnme);
                }
            }
        }

        // check if children are correct for each node (4.1)
        for (auto it = m_model.nodes.begin(); it != m_model.nodes.end(); it++) {
            for(uint32_t j = 0; j < it->children_count; j++) {
                if(it->children[j] >= (uint32_t)m_model.nodes.size()) {
                    const std::string errme = "DAS validation error: Invalid child node id " + std::to_string(it->children[j]) +
                                              " for node " + std::to_string(it - m_model.nodes.begin());
                    m_error_stack.push(errme);
                    m_critical_bit = true;
                } else {
                    consumed_table[it->children[j]] = true;
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
        std::vector<bool> consumed_joint_table(m_model.joints.size());
        std::vector<uint32_t> joint_nr_table(m_model.joints.size());

        for (auto it = m_model.skeletons.begin(); it != m_model.skeletons.end(); it++) {
            std::fill(joint_nr_table.begin(), joint_nr_table.end(), 0);

            // check nr 5.1
            if(it->parent >= (uint32_t)m_model.joints.size()) {
                const std::string errme = "DAS validation error: Invalid parent joint id " + std::to_string(it->parent) +
                                          " for skeleton " + std::to_string(it - m_model.skeletons.begin());
                m_error_stack.push(errme);
                m_critical_bit = true;
            }

            for(uint32_t j = 0; j < it->joint_count; j++) {
                if(it->joints[j] >= (uint32_t)m_model.joints.size()) {
                    const std::string errme = "DAS validation error: Invalid joint id " + std::to_string(it->joints[j]) +
                                              " for skeleton " + std::to_string(it - m_model.skeletons.begin());
                    m_error_stack.push(errme);
                    m_critical_bit = true;
                } else {
                    joint_nr_table[it->joints[j]]++;
                    consumed_joint_table[it->joints[j]] = true;
                }
            }

            // check if there are matching skeleton joint ids in the joints array (5.2)
            for(auto jntnr_it = joint_nr_table.begin(); jntnr_it != joint_nr_table.end(); jntnr_it++) {
                if(*jntnr_it > 1) {
                    const size_t index = jntnr_it - joint_nr_table.begin();
                    const std::string warnme = "DAS validation error: Joint node " + std::to_string(index) + 
                                               " is used multiple times by skeleton " + std::to_string(it - m_model.skeletons.begin());
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
