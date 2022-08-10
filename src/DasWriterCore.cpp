/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: DasWriterCore.cpp - DAS writer parent class implementation
/// author: Karl-Mihkel Ott

#define DAS_WRITER_CORE_CPP
#include "das/DasWriterCore.h"

namespace Libdas {

    DasWriterCore::DasWriterCore(const std::string &_file_name) :
        m_file_name(_file_name) 
    {
        // open a file stream if possible
        if(m_file_name != "") {
            _CheckAndAddFileExtension();
            _OpenFileStream();
        }
    }


    DasWriterCore::~DasWriterCore() {
        // close the stream if it is open
        if(m_out_stream.is_open())
            m_out_stream.close();
    }


    void DasWriterCore::_CheckAndAddFileExtension() {
        // search for the nearest dot from the end of string
        int32_t i = static_cast<int32_t>(m_file_name.size() - 1);
        for(; i >= 0; i--) {
            if(m_file_name[i] == '.')
                break;
        }

        // dot character was found and file extension is .das
        if(i > 0 && m_file_name.substr(i, m_file_name.size()) == ".das")
            return;
        else m_file_name += ".das";
    }


    void DasWriterCore::_OpenFileStream() {
        // file stream is currently open, close it
        if(m_out_stream.is_open())
            m_out_stream.close();

        m_out_stream.open(m_file_name, std::ios_base::binary);
        
        // file opening failed, exit the program
        if(m_out_stream.fail()) {
            std::cerr << "Could not open file " + m_file_name << " for writing. Check permissions!" << std::endl;
            EXIT_ON_ERROR(LIBDAS_ERROR_INVALID_FILE);
        }
    }


    void DasWriterCore::_WriteStringValue(const std::string &_value_name, const std::string &_value) {
        LIBDAS_ASSERT(m_out_stream.is_open());

        m_out_stream.write(_value_name.c_str(), _value_name.size());
        m_out_stream.write(": \"", 3);
        m_out_stream.write(_value.c_str(), _value.size());
        m_out_stream.write("\"", 1);
        m_out_stream.write(LIBDAS_DAS_NEWLINE, strlen(LIBDAS_DAS_NEWLINE));
    }


    void DasWriterCore::_WriteGenericDataValue(const char *_data, const size_t _len, bool _append_nl, const std::string &_value_name) {
        LIBDAS_ASSERT(m_out_stream.is_open());

        if(_value_name != "") {
            m_out_stream.write(_value_name.c_str(), _value_name.size());
            m_out_stream.write(": ", 2);
        }

        m_out_stream.write(_data, _len);

        if(_append_nl) m_out_stream.write(LIBDAS_DAS_NEWLINE, strlen(LIBDAS_DAS_NEWLINE));
    }


    void DasWriterCore::_WriteScopeBeginning(const std::string &_scope_name) {
        LIBDAS_ASSERT(m_out_stream.is_open());
        m_out_stream.write(_scope_name.c_str(), _scope_name.size());
        m_out_stream.write(LIBDAS_DAS_NEWLINE, strlen(LIBDAS_DAS_NEWLINE));
    }


    void DasWriterCore::_EndScope() {
        LIBDAS_ASSERT(m_out_stream.is_open());
        const std::string end_decl = "ENDSCOPE";
        m_out_stream.write(end_decl.c_str(), end_decl.size());
        m_out_stream.write(LIBDAS_DAS_NEWLINE, strlen(LIBDAS_DAS_NEWLINE));
    }


    void DasWriterCore::NewFile(const std::string &_file_name) {
        m_file_name = _file_name;
        LIBDAS_ASSERT(m_file_name != "");
        _CheckAndAddFileExtension();
        _OpenFileStream();
    }


    void DasWriterCore::CloseStream() {
        if(m_out_stream.is_open())
            m_out_stream.close();
    }


    void DasWriterCore::InitialiseFile(const DasProperties &_properties) {
        DasSignature sig;
        m_out_stream.write(reinterpret_cast<char*>(&sig), sizeof(DasSignature));

        _WriteScopeBeginning("PROPERTIES");

        // write model name if present
        if(_properties.model != "")
            _WriteStringValue("MODEL", _properties.model);

        // write author name if present
        if(_properties.author != "")
            _WriteStringValue("AUTHOR", _properties.author);

        // write copyright if present
        if(_properties.copyright != "")
            _WriteStringValue("COPYRIGHT", _properties.copyright);

        // write modification date
        const auto epoch = std::chrono::system_clock::now().time_since_epoch();
        const auto s = std::chrono::duration_cast<std::chrono::seconds>(epoch);
        _WriteNumericalValue<uint64_t>("MODDATE", static_cast<uint64_t>(s.count()));

        // write default scene value
        _WriteNumericalValue<uint32_t>("DEFAULTSCENE", _properties.default_scene);

        _EndScope();
    }


    void DasWriterCore::WriteBuffer(const DasBuffer &_buffer) {
        _WriteScopeBeginning("BUFFER");
        _WriteNumericalValue<BufferType>("BUFFERTYPE", _buffer.type);
        _WriteNumericalValue<uint32_t>("DATALEN", _buffer.data_len);

        m_out_stream.write("DATA: ", 6);
        for(auto it = _buffer.data_ptrs.begin(); it != _buffer.data_ptrs.end(); it++)
            m_out_stream.write(it->first, it->second);
        m_out_stream.write(LIBDAS_DAS_NEWLINE, strlen(LIBDAS_DAS_NEWLINE));

        _EndScope();
    }


    void DasWriterCore::WriteTextureBuffer(const std::vector<std::string> &_textures) {
        for(const std::string &file_name : _textures) {
            _WriteScopeBeginning("BUFFER");
            TextureReader rd = TextureReader(file_name);
            BufferType type = rd.GetImageBufferType();
            _WriteNumericalValue<BufferType>("BUFFERTYPE", type);
            
            size_t len = 0;
            const char *data = rd.GetBuffer(len);
            _WriteNumericalValue<uint32_t>("DATALEN", static_cast<uint32_t>(len));
            _WriteGenericDataValue(data, len, true, "DATA");
        }
    }


    void DasWriterCore::WriteMeshPrimitive(const DasMeshPrimitive &_primitive) {
        _WriteScopeBeginning("MESHPRIMITIVE");

        _WriteNumericalValue<uint32_t>("INDEXBUFFERID", _primitive.index_buffer_id);
        if(_primitive.index_buffer_offset)
            _WriteNumericalValue<uint32_t>("INDEXBUFFEROFFSET", _primitive.index_buffer_offset);
        _WriteNumericalValue<uint32_t>("INDICESCOUNT", _primitive.indices_count);
        _WriteNumericalValue<uint32_t>("VERTEXBUFFERID", _primitive.vertex_buffer_id);
        if(_primitive.vertex_buffer_offset)
            _WriteNumericalValue<uint32_t>("VERTEXBUFFEROFFSET", _primitive.vertex_buffer_offset);

        if(_primitive.vertex_normal_buffer_id != UINT32_MAX) {
            _WriteNumericalValue<uint32_t>("VERTEXNORMALBUFFERID", _primitive.vertex_normal_buffer_id);
            if(_primitive.vertex_normal_buffer_offset)
                _WriteNumericalValue<uint32_t>("VERTEXNORMALBUFFEROFFSET", _primitive.vertex_normal_buffer_offset);
        }

        if(_primitive.vertex_tangent_buffer_id != UINT32_MAX) {
            _WriteNumericalValue<uint32_t>("VERTEXTANGENTBUFFERID", _primitive.vertex_tangent_buffer_id);
            if(_primitive.vertex_tangent_buffer_offset)
                _WriteNumericalValue<uint32_t>("VERTEXTANGENTBUFFEROFFSET", _primitive.vertex_tangent_buffer_offset);
        }

        if(_primitive.texture_count) {
            _WriteNumericalValue<uint32_t>("TEXTURECOUNT", _primitive.texture_count);
            _WriteArrayValue<uint32_t>("UVBUFFERIDS", _primitive.texture_count, _primitive.uv_buffer_ids);
            _WriteArrayValue<uint32_t>("UVBUFFEROFFSETS", _primitive.texture_count, _primitive.uv_buffer_offsets);

            if(_primitive.texture_ids)
                _WriteArrayValue<uint32_t>("TEXTUREIDS", _primitive.texture_count, _primitive.texture_ids);
        }
        
        if(_primitive.color_mul_count) {
            _WriteNumericalValue<uint32_t>("COLORMULCOUNT", _primitive.texture_count);
            _WriteArrayValue<uint32_t>("COLORMULBUFFERIDS", _primitive.texture_count, _primitive.uv_buffer_ids);
            _WriteArrayValue<uint32_t>("COLORMULBUFFEROFFSETS", _primitive.texture_count, _primitive.uv_buffer_offsets);
        }

        if(_primitive.joint_set_count) {
            _WriteNumericalValue<uint32_t>("JOINTSETCOUNT", _primitive.joint_set_count);
            _WriteArrayValue<uint32_t>("JOINTINDEXBUFFERIDS", _primitive.joint_set_count, _primitive.joint_index_buffer_ids);
            _WriteArrayValue<uint32_t>("JOINTINDEXBUFFEROFFSETS", _primitive.joint_set_count, _primitive.joint_index_buffer_offsets);
            _WriteArrayValue<uint32_t>("JOINTWEIGHTBUFFERIDS", _primitive.joint_set_count, _primitive.joint_weight_buffer_ids);
            _WriteArrayValue<uint32_t>("JOINTWEIGHTBUFFEROFFSETS", _primitive.joint_set_count, _primitive.joint_weight_buffer_offsets);
        }
        

        if(_primitive.morph_target_count) {
            _WriteNumericalValue<uint32_t>("MORPHTARGETCOUNT", _primitive.morph_target_count);
            _WriteArrayValue<uint32_t>("MORPHTARGETS", _primitive.morph_target_count, _primitive.morph_targets);
            _WriteArrayValue<float>("MORPHWEIGHTS", _primitive.morph_target_count, _primitive.morph_weights);
        }

        _EndScope();
    }


    void DasWriterCore::WriteMorphTarget(const DasMorphTarget &_morph_target) {
        _WriteScopeBeginning("MORPHTARGET");

        if(_morph_target.vertex_buffer_id != UINT32_MAX) {
            _WriteNumericalValue<uint32_t>("VERTEXBUFFERID", _morph_target.vertex_buffer_id);
            if(_morph_target.vertex_buffer_offset)
                _WriteNumericalValue<uint32_t>("VERTEXBUFFEROFFSET", _morph_target.vertex_buffer_offset);
        }

        if(_morph_target.texture_count) {
            _WriteNumericalValue<uint32_t>("TEXTURECOUNT", _morph_target.texture_count);
            _WriteArrayValue<uint32_t>("UVBUFFERIDS", _morph_target.texture_count, _morph_target.uv_buffer_ids);
            _WriteArrayValue<uint32_t>("UVBUFFEROFFSETS", _morph_target.texture_count, _morph_target.uv_buffer_offsets);
        }

        if(_morph_target.color_mul_count) {
            _WriteNumericalValue<uint32_t>("COLORMULCOUNT", _morph_target.color_mul_count);
            _WriteArrayValue<uint32_t>("COLORMULBUFFERIDS", _morph_target.color_mul_count, _morph_target.color_mul_buffer_ids);
            _WriteArrayValue<uint32_t>("COLORMULBUFFEROFFSETS", _morph_target.color_mul_count, _morph_target.color_mul_buffer_offsets);
        }

        if(_morph_target.vertex_normal_buffer_id != UINT32_MAX) {
            _WriteNumericalValue<uint32_t>("VERTEXNORMALBUFFERID", _morph_target.vertex_normal_buffer_id);
            if(_morph_target.vertex_normal_buffer_offset)
                _WriteNumericalValue<uint32_t>("VERTEXNORMALBUFFEROFFSET", _morph_target.vertex_normal_buffer_offset);
        }

        if(_morph_target.vertex_tangent_buffer_id != UINT32_MAX) {
            _WriteNumericalValue<uint32_t>("VERTEXTANGENTBUFFERID", _morph_target.vertex_tangent_buffer_id);
            if(_morph_target.vertex_normal_buffer_offset)
                _WriteNumericalValue<uint32_t>("VERTEXTANGENTBUFFEROFFSET", _morph_target.vertex_tangent_buffer_offset);
        }

        _EndScope();
    }


    void DasWriterCore::WriteMesh(const DasMesh &_mesh) {
        _WriteScopeBeginning("MESH");
        
        // check if name should be written
        if(_mesh.name != "")
            _WriteStringValue("NAME", _mesh.name);

        _WriteNumericalValue<uint32_t>("PRIMITIVECOUNT", _mesh.primitive_count);
        _WriteArrayValue<uint32_t>("PRIMITIVES", _mesh.primitive_count, _mesh.primitives);

        _EndScope();
    }


    void DasWriterCore::WriteNode(const DasNode &_node) {
        _WriteScopeBeginning("NODE");

        if(_node.name != "") _WriteStringValue("NAME", _node.name);
        if(_node.children_count) {
            _WriteNumericalValue<uint32_t>("CHILDRENCOUNT", _node.children_count);
            _WriteArrayValue<uint32_t>("CHILDREN", _node.children_count, _node.children);
        }
        if(_node.mesh != UINT32_MAX)
            _WriteNumericalValue<uint32_t>("MESH", _node.mesh);
        if(_node.skeleton != UINT32_MAX)
            _WriteNumericalValue<uint32_t>("SKELETON", _node.skeleton);

        _WriteMatrixValue<float>("TRANSFORM", _node.transform);

        _EndScope();
    }


    void DasWriterCore::WriteScene(const DasScene &_scene) {
        _WriteScopeBeginning("SCENE");
        if(_scene.name != "") _WriteStringValue("NAME", _scene.name);
        _WriteNumericalValue<uint32_t>("NODECOUNT", _scene.node_count);
        _WriteArrayValue<uint32_t>("NODES", _scene.node_count, _scene.nodes);

        _EndScope();
    }


    void DasWriterCore::WriteSkeleton(const DasSkeleton &_skeleton) {
        _WriteScopeBeginning("SKELETON");
        if(_skeleton.name != "") _WriteStringValue("NAME", _skeleton.name);
        _WriteNumericalValue<uint32_t>("PARENT", _skeleton.parent);
        _WriteNumericalValue<uint32_t>("JOINTCOUNT", _skeleton.joint_count);
        _WriteArrayValue<uint32_t>("JOINTS", _skeleton.joint_count, _skeleton.joints);

        _EndScope();
    }


    void DasWriterCore::WriteSkeletonJoint(const DasSkeletonJoint &_joint) {
        _WriteScopeBeginning("JOINT");
        _WriteMatrixValue<float>("INVERSEBINDPOS", _joint.inverse_bind_pos);

        if(_joint.name != "") _WriteStringValue("NAME", _joint.name);
        _WriteNumericalValue<uint32_t>("CHILDRENCOUNT", _joint.children_count);
        _WriteArrayValue<uint32_t>("CHILDREN", _joint.children_count, _joint.children);
        _WriteNumericalValue<float>("SCALE", _joint.scale);
        _WriteGenericDataValue(reinterpret_cast<const char*>(&_joint.rotation), sizeof(TRS::Quaternion), true, "ROTATION");
        _WriteGenericDataValue(reinterpret_cast<const char*>(&_joint.translation), sizeof(TRS::Point3D<float>), true, "TRANSLATION");

        _EndScope();
    }


    void DasWriterCore::WriteAnimationChannel(const DasAnimationChannel &_channel) {
        _WriteScopeBeginning("ANIMATIONCHANNEL");
        if(_channel.node_id != UINT32_MAX)
            _WriteNumericalValue<uint32_t>("NODEID", _channel.node_id);
        else if(_channel.joint_id != UINT32_MAX)
            _WriteNumericalValue<uint32_t>("JOINTID", _channel.joint_id);
        _WriteNumericalValue<AnimationTarget>("TARGET", _channel.target);
        _WriteNumericalValue<InterpolationType>("INTERPOLATION", _channel.interpolation);
        _WriteNumericalValue<uint32_t>("KEYFRAMECOUNT", _channel.keyframe_count);
        if(_channel.weight_count)
            _WriteNumericalValue<uint32_t>("WEIGHTCOUNT", _channel.weight_count);
        _WriteArrayValue<float>("KEYFRAMES", _channel.keyframe_count, _channel.keyframes);

        uint32_t type_stride = 0;
        switch(_channel.target) {
            case LIBDAS_ANIMATION_TARGET_WEIGHTS:
                type_stride = static_cast<uint32_t>(sizeof(float)) * _channel.weight_count;
                break;

            case LIBDAS_ANIMATION_TARGET_TRANSLATION:
                type_stride = static_cast<uint32_t>(sizeof(TRS::Vector3<float>));
                break;

            case LIBDAS_ANIMATION_TARGET_ROTATION:
                type_stride = static_cast<uint32_t>(sizeof(TRS::Quaternion));
                break;

            case LIBDAS_ANIMATION_TARGET_SCALE:
                type_stride = static_cast<uint32_t>(sizeof(float));
                break;

            default:
                LIBDAS_ASSERT(false);
                break;
        }

        if(_channel.interpolation == LIBDAS_INTERPOLATION_VALUE_CUBICSPLINE)
            _WriteArrayValue<char>("TANGENTS", type_stride * 2 * _channel.keyframe_count, _channel.tangents);
        _WriteArrayValue<char>("TARGETVALUES", type_stride * _channel.keyframe_count, _channel.target_values);
        
        _EndScope();
    }


    void DasWriterCore::WriteAnimation(const DasAnimation &_animation) {
        _WriteScopeBeginning("ANIMATION");
        if(_animation.name != "") _WriteStringValue("NAME", _animation.name);
        _WriteNumericalValue<uint32_t>("CHANNELCOUNT", _animation.channel_count);
        _WriteArrayValue<uint32_t>("CHANNELS", _animation.channel_count, _animation.channels);

        _EndScope();
    }


    void DasWriterCore::AppendTextures(std::vector<DasBuffer> &_buffers, const std::vector<std::string> &_embedded_textures, bool _use_raw) {
        m_texture_readers.reserve(_embedded_textures.size());
        for(const std::string &file_name : _embedded_textures) {
            m_texture_readers.push_back(TextureReader(file_name, _use_raw));
            
            DasBuffer buf;
            buf.type = m_texture_readers.back().GetImageBufferType();
            size_t len;

            if(!_use_raw) {
                buf.data_ptrs.push_back(std::make_pair(m_texture_readers.back().GetBuffer(len), len));
                buf.data_len = static_cast<uint32_t>(len);
                buf.type = m_texture_readers.back().GetImageBufferType();
            } 
            else {
                int x, y;
                const char *raw_data = m_texture_readers.back().GetRawBuffer(x, y, len);
                m_raw_img_header.width = static_cast<uint32_t>(x);
                m_raw_img_header.height = static_cast<uint32_t>(y);
                m_raw_img_header.bit_depth = 4;

                buf.data_ptrs.push_back(std::make_pair(reinterpret_cast<const char*>(&m_raw_img_header), sizeof(RawImageDataHeader)));
                buf.data_len += static_cast<uint32_t>(sizeof(RawImageDataHeader) + len);
                buf.data_ptrs.push_back(std::make_pair(raw_data, len));
                buf.type = LIBDAS_BUFFER_TYPE_TEXTURE_RAW;
            }

            _buffers.push_back(buf);
        }
    }
}
