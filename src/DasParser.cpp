// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: DasParser.cpp - DAS format parser implementation
// author: Karl-Mihkel Ott

#define DAS_PARSER_CPP
#include "das/DasParser.h"

namespace Libdas {

#ifdef __DEBUG
    std::vector<std::vector<DasSkeletonJoint>::iterator> _JointCheck(std::vector<DasSkeletonJoint> &_joints) {
        std::vector<std::vector<DasSkeletonJoint>::iterator> overlapping_children;
        overlapping_children.reserve(_joints.size());

        // for each joint
        for(auto it = _joints.begin(); it != _joints.end(); it++) {
            // for each joint child
            for(uint32_t i = 0; i < it->children_count; i++) {
                if(it->children[i] >= _joints.size()) {
                    overlapping_children.push_back(it);
                    break;
                }
            }
        }

        return overlapping_children;
    }


    std::vector<std::vector<DasNode>::iterator> _NodeCheck(std::vector<DasNode> &_nodes) {
        std::vector<std::vector<DasNode>::iterator> overlapping_children;
        overlapping_children.reserve(_nodes.size());

        // for each node
        for(auto it = _nodes.begin(); it != _nodes.end(); it++) {
            // for each node child
            for(uint32_t i = 0; i < it->children_count; i++) {
                if(it->children[i] >= _nodes.size()) {
                    overlapping_children.push_back(it);
                    break;
                }
            }
        }

        return overlapping_children;
    }
#endif


    DasParser::DasParser(const std::string &_file_name) : 
        DasReaderCore(_file_name) 
    {
        // some default reservation values
        m_buffers.reserve(4);
        m_meshes.reserve(10);
        m_mesh_primitives.reserve(40);
        m_morph_targets.reserve(30);
        m_nodes.reserve(10);
        m_scenes.reserve(2);
        m_joints.reserve(32);
        m_skeletons.reserve(4);
        m_animations.reserve(10);
    }


    DasParser::DasParser(DasParser &&_parser) noexcept :
        m_props(std::move(_parser.m_props)),
        m_buffers(std::move(_parser.m_buffers)),
        m_meshes(std::move(_parser.m_meshes)),
        m_mesh_primitives(std::move(_parser.m_mesh_primitives)),
        m_morph_targets(std::move(_parser.m_morph_targets)),
        m_nodes(std::move(_parser.m_nodes)),
        m_scenes(std::move(_parser.m_scenes)),
        m_joints(std::move(_parser.m_joints)),
        m_skeletons(std::move(_parser.m_skeletons)),
        m_channels(std::move(_parser.m_channels)),
        m_animations(std::move(_parser.m_animations)) {}


    void DasParser::_DataCast(std::any &_any_scope, DasScopeType _type) {
        switch(_type) {
            case LIBDAS_DAS_SCOPE_PROPERTIES:
                m_props = std::any_cast<DasProperties&&>(std::move(_any_scope));
                break;

            case LIBDAS_DAS_SCOPE_BUFFER:
                m_buffers.emplace_back(std::any_cast<DasBuffer&&>(std::move(_any_scope)));
                break;

            case LIBDAS_DAS_SCOPE_MESH_PRIMITIVE:
                m_mesh_primitives.emplace_back(std::any_cast<DasMeshPrimitive&&>(std::move(_any_scope)));
                break;

            case LIBDAS_DAS_SCOPE_MORPH_TARGET:
                m_morph_targets.emplace_back(std::any_cast<DasMorphTarget&&>(std::move(_any_scope)));
                break;

            case LIBDAS_DAS_SCOPE_MESH:
                m_meshes.emplace_back(std::any_cast<DasMesh&&>(std::move(_any_scope)));
                break;

            case LIBDAS_DAS_SCOPE_NODE:
                m_nodes.emplace_back(std::any_cast<DasNode&&>(std::move(_any_scope)));
                break;

            case LIBDAS_DAS_SCOPE_SCENE:
                m_scenes.emplace_back(std::any_cast<DasScene&&>(std::move(_any_scope)));
                break;

            case LIBDAS_DAS_SCOPE_SKELETON:
                m_skeletons.emplace_back(std::any_cast<DasSkeleton&&>(std::move(_any_scope)));
                break;

            case LIBDAS_DAS_SCOPE_SKELETON_JOINT:
                m_joints.emplace_back(std::any_cast<DasSkeletonJoint&&>(std::move(_any_scope)));
                break;

            case LIBDAS_DAS_SCOPE_ANIMATION_CHANNEL:
                m_channels.emplace_back(std::any_cast<DasAnimationChannel&&>(std::move(_any_scope)));
                break;

            case LIBDAS_DAS_SCOPE_ANIMATION:
                m_animations.emplace_back(std::any_cast<DasAnimation&&>(std::move(_any_scope)));
                break;

            default:
                break;
        }
    }


    void DasParser::_FindSceneNodeRoots(DasScene &_scene) {
        // allocate memory for scene roots
        _scene.roots = new uint32_t[_scene.node_count];
        _scene.root_count = 0;

        // array for containing boolean values about nodes being used as children
        std::vector<bool> is_child(m_nodes.size(), false);

        // search for child nodes
        for(uint32_t i = 0; i < _scene.node_count; i++) {
            const Libdas::DasNode &node = m_nodes[_scene.nodes[i]];
            for(uint32_t j = 0; j < node.children_count; j++)
                is_child[node.children[j]] = true;
        }

        // check which node is not a child node
        for(uint32_t i = 0; i < _scene.node_count; i++) {
            if(!is_child[_scene.nodes[i]])
                _scene.roots[_scene.root_count++] = _scene.nodes[i];
        }
    }


    void DasParser::Parse(bool _clean_read, const std::string &_file_name) {
        if(_file_name != "")
            NewFile(_file_name);

        ReadSignature();

        // verify file signature
        DasScopeType type = LIBDAS_DAS_SCOPE_END;
        do {
            type = ParseScopeDeclaration();
            if(type == LIBDAS_DAS_SCOPE_END)
                break;
            std::any any_scope = ReadScopeData(type);
            _DataCast(any_scope, type);
        } while(true);

        if(_clean_read) CloseFile();

        m_buffers.shrink_to_fit();
        m_meshes.shrink_to_fit();
        m_mesh_primitives.shrink_to_fit();
        m_morph_targets.shrink_to_fit();
        m_nodes.shrink_to_fit();
        m_scenes.shrink_to_fit();
        m_joints.shrink_to_fit();
        m_skeletons.shrink_to_fit();
        m_animations.shrink_to_fit();

        // search and find root nodes for each given scene
        for(auto it = m_scenes.begin(); it != m_scenes.end(); it++)
            _FindSceneNodeRoots(*it);

        Clear();
    }


    void DasParser::DeleteBuffers() {
        for (auto buf_it = m_buffers.begin(); buf_it != m_buffers.end(); buf_it++) {
            for (auto ptr_it = buf_it->data_ptrs.begin(); ptr_it != buf_it->data_ptrs.end(); ptr_it++) {
                std::free(ptr_it->first);
            }
        }

        _ClearBlobs();
        m_buffers.clear();
    }
}
