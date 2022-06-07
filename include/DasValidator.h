// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: DasValidator.h - DAS file validation class header
// author: Karl-Mihkel Ott

#ifndef DAS_VALIDATOR_H
#define DAS_VALIDATOR_H

#ifdef DAS_VALIDATOR_CPP
    #include <cstring>
    #include <cmath>
#ifdef _DEBUG
    #include <iostream>
#endif
    #include <string>
    #include <stack>
    #include <fstream>
    #include <vector>
    #include <unordered_map>
    #include <algorithm>
    #include <any>

    #include <Points.h>
    #include <Vector.h>
    #include <Matrix.h>
    #include <Quaternion.h>

    #include <Api.h>
    #include <ErrorHandlers.h>
    #include <AsciiStreamReader.h>
    #include <AsciiLineReader.h>
    #include <DasStructures.h>
    #include <DasReaderCore.h>
    #include <DasParser.h>
#endif

namespace Libdas {

    /**
     * DasValidator class performs following checks on the file:
     *   1. Check if properties contain valid default scene (error stack, critical_bit)
     *   2. Check meshes and mesh primitives
     *     2.1 Check if mesh primitive indices are correct (error stack, critical_bit)
     *     2.2 Check if buffer regions are correct (error stack)
     *     2.3 Check if morph target indices are correct (error stack, critical_bit)
     *     2.4 Check if indices form a continuos array when sorted (warning stack)
     *     2.5 Check if all mesh primitives are used by meshes (warning stack)
     *   3. Check morph targets
     *     3.1 Check if all morph targets are used (warning stack)
     *     3.2 Check if buffer regions are correct (error stack)
     *   4. Check scenes and nodes  
     *     4.1 Check if scene node ids are correct (error stack, critical_bit)
     *     4.2 Check if there are matching scene node ids in the node array (warning stack)
     *     4.2 Check if all nodes are consumed as scene roots or as child nodes (warning stack)
     *   5. Check skeletons and joints
     *     5.1 Check if skeleton joint ids are correct (error stack, critical_bit)
     *     5.2 Check if there are matching skeleton joint ids in the joint array (warning stack)
     *     5.2 Check if all joints are consumed by skeletons (warning stack)
     */
    class LIBDAS_API DasValidator {
        private:
            DasParser &m_parser;
            std::stack<std::string> m_warning_stack;
            std::stack<std::string> m_error_stack;
            std::vector<uint32_t> m_max_index_table;
            bool m_critical_bit = false;

        private:
            // templated checking methods
            template<typename T>
            void _CheckPositionVertices(const T &_prim, uint32_t _cur_index, uint32_t _max_index) {
                if(_prim.vertex_buffer_id >= m_parser.GetBufferCount()) {
                    std::string errme;
                    if constexpr(std::is_same_v<T, DasMeshPrimitive>) {
                        errme = "DAS validation error: Invalid position vertex buffer id " + std::to_string(_prim.vertex_buffer_id) + " for mesh primitive " + std::to_string(_cur_index);
                    } else {
                        errme = "DAS validation error: Invalid position vertex buffer id " + std::to_string(_prim.vertex_buffer_id) + " for morph target " + std::to_string(_cur_index);
                    }
                    m_error_stack.push(errme);
                } else if(m_parser.AccessBuffer(_prim.vertex_buffer_id).data_len < _prim.vertex_buffer_offset + _max_index * sizeof(Libdas::Vector3<float>)) {
                    std::string errme;
                    if constexpr(std::is_same_v<T, DasMeshPrimitive>) {
                        errme = "DAS validation error: Invalid position vertex buffer(" + std::to_string(_prim.vertex_buffer_id) + 
                                ") region with offset " + std::to_string(_prim.vertex_buffer_offset) + " and size " + 
                                std::to_string(_max_index * static_cast<uint32_t>(sizeof(Libdas::Vector3<float>))) +
                                " for mesh primitive " + std::to_string(_cur_index); 
                    } else {
                        errme = "DAS validation error: Invalid position vertex buffer(" + std::to_string(_prim.vertex_buffer_id) + 
                                ") region with offset " + std::to_string(_prim.vertex_buffer_offset) + " and size " + 
                                std::to_string(_max_index * static_cast<uint32_t>(sizeof(Libdas::Vector3<float>))) +
                                " for morph target" + std::to_string(_cur_index); 
                    }
                    m_error_stack.push(errme);
                }
            }

            template<typename T>
            void _CheckVertexNormal(const T &_prim, uint32_t _cur_index, uint32_t _max_index) {
                if(_prim.vertex_normal_buffer_id != UINT32_MAX && _prim.vertex_normal_buffer_id >= m_parser.GetBufferCount()) {
                    std::string errme;
                    if constexpr(std::is_same_v<T, DasMeshPrimitive>) {
                        errme = "DAS validation error: Invalid vertex normal buffer id " + std::to_string(_prim.vertex_normal_buffer_id) + " for mesh primitive " + std::to_string(_cur_index);
                    } else {
                        errme = "DAS validation error: Invalid vertex normal buffer id " + std::to_string(_prim.vertex_normal_buffer_id) + " for morph target " + std::to_string(_cur_index);
                    }
                    m_error_stack.push(errme);
                } else if(_prim.vertex_normal_buffer_id != UINT32_MAX && m_parser.AccessBuffer(_prim.vertex_normal_buffer_id).data_len < _prim.vertex_normal_buffer_offset + _max_index * sizeof(Libdas::Vector3<float>)) {
                    std::string errme;
                    if constexpr(std::is_same_v<T, DasMeshPrimitive>) {
                        errme = "DAS validation error: Invalid vertex normal buffer(" + std::to_string(_prim.vertex_normal_buffer_id) + 
                                ") region with offset " + std::to_string(_prim.vertex_normal_buffer_offset) + " and size " + 
                                std::to_string(_max_index * static_cast<uint32_t>(sizeof(Libdas::Vector3<float>))) +
                                " for mesh primitive " + std::to_string(_cur_index); 
                    } else {
                        errme = "DAS validation error: Invalid vertex normal buffer(" + std::to_string(_prim.vertex_normal_buffer_id) + 
                                ") region with offset " + std::to_string(_prim.vertex_normal_buffer_offset) + " and size " + 
                                std::to_string(_max_index * static_cast<uint32_t>(sizeof(Libdas::Vector3<float>))) +
                                " for morph target" + std::to_string(_cur_index); 
                    }
                    m_error_stack.push(errme);
                }
            }

            template<typename T>
            void _CheckVertexTangent(const T &_prim, uint32_t _cur_index, uint32_t _max_index) {
                if(_prim.vertex_tangent_buffer_id != UINT32_MAX && _prim.vertex_tangent_buffer_id >= m_parser.GetBufferCount()) {
                    std::string errme;
                    if constexpr(std::is_same_v<T, DasMeshPrimitive>) {
                        errme = "DAS validation error: Invalid vertex tangent buffer id " + std::to_string(_prim.vertex_tangent_buffer_id) + " for mesh primitive " + std::to_string(_cur_index);
                    } else {
                        errme = "DAS validation error: Invalid vertex tangent buffer id " + std::to_string(_prim.vertex_tangent_buffer_id) + " for morph target " + std::to_string(_cur_index);
                    }
                    m_error_stack.push(errme);
                } else if(_prim.vertex_tangent_buffer_id != UINT32_MAX && m_parser.AccessBuffer(_prim.vertex_tangent_buffer_id).data_len < _prim.vertex_tangent_buffer_offset + _max_index * sizeof(Libdas::Vector4<float>)) {
                    std::string errme;
                    if constexpr(std::is_same_v<T, DasMeshPrimitive>) {
                        errme = "DAS validation error: Invalid vertex tangent buffer(" + std::to_string(_prim.vertex_tangent_buffer_id) + 
                                ") region with offset " + std::to_string(_prim.vertex_tangent_buffer_offset) + " and size " + 
                                std::to_string(_max_index * static_cast<uint32_t>(sizeof(Libdas::Vector4<float>))) +
                                " for mesh primitive " + std::to_string(_cur_index); 
                    } else {
                        errme = "DAS validation error: Invalid vertex tangent buffer(" + std::to_string(_prim.vertex_tangent_buffer_id) + 
                                ") region with offset " + std::to_string(_prim.vertex_tangent_buffer_offset) + " and size " + 
                                std::to_string(_max_index * static_cast<uint32_t>(sizeof(Libdas::Vector4<float>))) +
                                " for morph target" + std::to_string(_cur_index); 
                    }
                    m_error_stack.push(errme);
                }
            }

            template<typename T>
            void _CheckTextureProperties(const T &_prim, uint32_t _cur_index, uint32_t _max_index) {
                if(_prim.texture_count) {
                    // for each texture check it's buffer regions
                    for(uint32_t i = 0; i < _prim.texture_count; i++) {
                        if(_prim.uv_buffer_ids[i] >= m_parser.GetBufferCount()) {
                            std::string errme;
                            if constexpr(std::is_same_v<T, DasMeshPrimitive>) {
                                errme = "DAS validation error: Invalid uv buffer id " + std::to_string(_prim.uv_buffer_ids[i]) + " for mesh primitive " + std::to_string(_cur_index);
                            } else {
                                errme = "DAS validation error: Invalid uv buffer id " + std::to_string(_prim.uv_buffer_ids[i]) + " for morph target " + std::to_string(_cur_index);
                            }
                            m_error_stack.push(errme);
                        } else if(m_parser.AccessBuffer(_prim.uv_buffer_ids[i]).data_len < _prim.uv_buffer_offsets[i] + _max_index * sizeof(Libdas::Vector2<float>)) {
                            std::string errme;
                            if constexpr(std::is_same_v<T, DasMeshPrimitive>) {
                                errme = "DAS validation error: Invalid uv vertex buffer(" + std::to_string(_prim.uv_buffer_ids[i]) +
                                        ") region with offset " + std::to_string(_prim.uv_buffer_offsets[i]) + " and size " +
                                        std::to_string(_max_index * static_cast<uint32_t>(sizeof(Libdas::Vector2<float>))) +
                                        " for mesh primitive " + std::to_string(_cur_index);
                            } else {
                                errme = "DAS validation error: Invalid uv vertex buffer(" + std::to_string(_prim.uv_buffer_ids[i]) +
                                        ") region with offset " + std::to_string(_prim.uv_buffer_offsets[i]) + " and size " +
                                        std::to_string(_max_index * static_cast<uint32_t>(sizeof(Libdas::Vector2<float>))) +
                                        " for morph target " + std::to_string(_cur_index);
                            }
                            m_error_stack.push(errme);
                        }
                    }
                }
            }

            template<typename T>
            void _CheckColorMulProperties(const T &_prim, uint32_t _cur_index, uint32_t _max_index) {
                if(_prim.color_mul_count) {
                    // for each color multiplier check it's buffer regions
                    for(uint32_t i = 0; i < _prim.color_mul_count; i++) {
                        if(_prim.color_mul_buffer_ids[i] >= m_parser.GetBufferCount()) {
                            std::string errme;
                            if constexpr(std::is_same_v<T, DasMeshPrimitive>) {
                                errme = "DAS validation error: Invalid color multiplier buffer id " + std::to_string(_prim.color_mul_buffer_ids[i]) + " for mesh primitive " + std::to_string(_cur_index);
                            } else {
                                errme = "DAS validation error: Invalid color multiplier buffer id " + std::to_string(_prim.color_mul_buffer_ids[i]) + " for morph target " + std::to_string(_cur_index);
                            }
                            m_error_stack.push(errme);
                        } else if(m_parser.AccessBuffer(_prim.color_mul_buffer_ids[i]).data_len < _prim.color_mul_buffer_offsets[i] + _max_index * sizeof(Libdas::Vector4<float>)) {
                            std::string errme;
                            if constexpr(std::is_same_v<T, DasMeshPrimitive>) {
                                errme = "DAS validation error: Invalid color multiplier buffer(" + std::to_string(_prim.color_mul_buffer_ids[i]) +
                                        ") region with offset " + std::to_string(_prim.uv_buffer_offsets[i]) + " and size " +
                                        std::to_string(_max_index * static_cast<uint32_t>(sizeof(Libdas::Vector4<float>))) +
                                        " for mesh primitive " + std::to_string(_cur_index);
                            } else {
                                errme = "DAS validation error: Invalid color multiplier buffer(" + std::to_string(_prim.color_mul_buffer_ids[i]) +
                                        ") region with offset " + std::to_string(_prim.uv_buffer_offsets[i]) + " and size " +
                                        std::to_string(_max_index * static_cast<uint32_t>(sizeof(Libdas::Vector4<float>))) +
                                        " for morph target " + std::to_string(_cur_index);
                            }
                            m_error_stack.push(errme);
                        }
                    }
                }
            }
            void _CheckJointProperties(const DasMeshPrimitive &_prim, uint32_t _cur_index, uint32_t _max_index);
            void _CheckMorphTargetIndices(const DasMeshPrimitive &_prim, uint32_t _cur_index);
            void _CheckMeshPrimitiveIndicesContinuity(const DasMeshPrimitive &_prim, uint32_t _cur_index);

            void _VerifyProperties();
            void _VerifyMeshes();
            void _VerifyMorphTargets();
            void _VerifyScenes();
            void _VerifySkeletons();

        public:
            DasValidator(DasParser &_parser, bool _validate = true);
            DasValidator(DasValidator &&_val) noexcept;

            void Validate();

            inline std::string PopWarningStack() {
                std::string warnme = m_warning_stack.top();
                m_warning_stack.pop();
                return warnme;
            }

            inline bool IsWarningStackEmpty() {
                return m_warning_stack.empty();
            }

            inline std::string PopErrorStack() {
                std::string errme = m_error_stack.top();
                m_error_stack.pop();
                return errme;
            }

            inline bool IsErrorStackEmpty() {
                return m_error_stack.empty();
            }

            inline bool IsCriticalBit() {
                return m_critical_bit;
            }
    };
}

#endif
