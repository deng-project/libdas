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
                m_props = std::move(std::any_cast<DasProperties>(_any_scope));
                break;

            case LIBDAS_DAS_SCOPE_BUFFER:
                m_buffers.push_back(std::any_cast<DasBuffer>(_any_scope));
                break;

            case LIBDAS_DAS_SCOPE_MODEL:
                m_models.push_back(std::any_cast<DasModel>(_any_scope));
                break;

            case LIBDAS_DAS_SCOPE_ANIMATION:
                m_animations.push_back(std::any_cast<DasAnimation>(_any_scope));
                break;

            // inner scope of animation
            case LIBDAS_DAS_SCOPE_KEYFRAME:
                m_animations.back().keyframes.push_back(std::any_cast<DasKeyframe>(_any_scope));
                break;

            case LIBDAS_DAS_SCOPE_SCENE:
                m_scenes.push_back(std::any_cast<DasScene>(_any_scope));
                break;

            case LIBDAS_DAS_SCOPE_SCENE_NODE:
                m_scenes.back().nodes.push_back(std::any_cast<DasSceneNode>(_any_scope));
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


    DasProperties &DasParser::GetProperties() {
        return m_props;
    }

    
    std::vector<DasScene> &DasParser::GetScenes() {
        return m_scenes;
    }


    DasBuffer &DasParser::AccessBuffer(uint32_t _id) {
        return m_buffers.at(_id);
    }


    DasModel &DasParser::AccessModel(uint32_t _id) {
        return m_models.at(_id);
    }


    DasAnimation &DasParser::AccessAnimation(uint32_t _id) {
        return m_animations.at(_id);
    }


    uint32_t DasParser::GetModelCount() {
        return static_cast<uint32_t>(m_models.size());
    }


    uint32_t DasParser::GetAnimationCount() {
        return static_cast<uint32_t>(m_animations.size());
    }
}
