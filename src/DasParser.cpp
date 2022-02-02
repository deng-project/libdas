// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: DasParser.cpp - DAS format parser implementation
// author: Karl-Mihkel Ott

#define DAS_PARSER_CPP
#include <DasParser.h>

namespace Libdas {

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
    }
}
