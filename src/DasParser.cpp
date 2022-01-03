// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: DasParser.cpp - DAS format parser implementation
// author: Karl-Mihkel Ott

#define DAS_PARSER_CPP
#include <DasParser.h>

namespace Libdas {

    DasParser::DasParser(const std::string &_file_name) : 
        DasReaderCore(_file_name) {}


    void DasParser::_DataCast(std::any &_any_scope, DasScopeType _type) {
        switch(_type) {
            case LIBDAS_DAS_SCOPE_PROPERTIES:
                m_props = std::move(std::any_cast<DasProperties&>(_any_scope));
                break;

            case LIBDAS_DAS_SCOPE_BUFFER:
                m_buffers.push_back(std::move(std::any_cast<DasBuffer&>(_any_scope)));
                break;

            case LIBDAS_DAS_SCOPE_MESH:
                m_meshes.push_back(std::move(std::any_cast<DasMesh&>(_any_scope)));
                break;

            case LIBDAS_DAS_SCOPE_NODE:
                m_nodes.push_back(std::move(std::any_cast<DasNode&>(_any_scope)));
                std::any_cast<DasNode&>(_any_scope).children = nullptr;
                std::any_cast<DasNode&>(_any_scope).meshes = nullptr;
                std::any_cast<DasNode&>(_any_scope).animations = nullptr;
                std::any_cast<DasNode&>(_any_scope).skeletons = nullptr;
                break;

            case LIBDAS_DAS_SCOPE_SCENE:
                m_scenes.push_back(std::move(std::any_cast<DasScene&>(_any_scope)));
                std::any_cast<DasScene&>(_any_scope).nodes = nullptr;
                break;

            case LIBDAS_DAS_SCOPE_SKELETON:
                m_skeletons.push_back(std::move(std::any_cast<DasSkeleton&>(_any_scope)));
                std::any_cast<DasSkeleton&>(_any_scope).joints = nullptr;
                break;

            case LIBDAS_DAS_SCOPE_SKELETON_JOINT:
                m_joints.push_back(std::move(std::any_cast<DasSkeletonJoint&>(_any_scope)));
                break;

            case LIBDAS_DAS_SCOPE_ANIMATION:
                m_animations.push_back(std::move(std::any_cast<DasAnimation&>(_any_scope)));
                std::any_cast<DasAnimation&>(_any_scope).keyframe_timestamps = nullptr;
                std::any_cast<DasAnimation&>(_any_scope).interpolation_types = nullptr;
                std::any_cast<DasAnimation&>(_any_scope).animation_targets = nullptr;
                break;

            default:
                break;
        }
    }


    void DasParser::Parse(const std::string &_file_name) {
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
    }
}
