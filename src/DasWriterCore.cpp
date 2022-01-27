/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: DasWriterCore.cpp - DAS writer parent class implementation
/// author: Karl-Mihkel Ott

#define DAS_WRITER_CORE_CPP
#include <DasWriterCore.h>

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
            std::exit(LIBDAS_ERROR_INVALID_FILE);
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

        for(size_t i = 0; i < _buffer.data_ptrs.size(); i++) {
            if(!i && i != _buffer.data_ptrs.size() - 1) 
                _WriteGenericDataValue(_buffer.data_ptrs[i].first, _buffer.data_ptrs[i].second, false, "DATA");
            else if(!i)
                _WriteGenericDataValue(_buffer.data_ptrs[i].first, _buffer.data_ptrs[i].second, true, "DATA");
            else if(i != _buffer.data_ptrs.size() - 1)
                _WriteGenericDataValue(_buffer.data_ptrs[i].first, _buffer.data_ptrs[i].second, false);
            else _WriteGenericDataValue(_buffer.data_ptrs[i].first, _buffer.data_ptrs[i].second);
        }

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


    void DasWriterCore::WriteMesh(const DasMesh &_mesh) {
        _WriteScopeBeginning("MESH");
        
        // check if name should be written
        if(_mesh.name != "")
            _WriteStringValue("NAME", _mesh.name);

        // write numerical buffer related values
        _WriteNumericalValue<uint32_t>("INDEXBUFFERID", _mesh.index_buffer_id);
        _WriteNumericalValue<uint32_t>("INDEXBUFFEROFFSET", _mesh.index_buffer_offset);
        _WriteNumericalValue<uint32_t>("INDICESCOUNT", _mesh.indices_count);
        _WriteNumericalValue<uint32_t>("VERTEXBUFFERID", _mesh.vertex_buffer_id);
        _WriteNumericalValue<uint32_t>("VERTEXBUFFEROFFSET", _mesh.vertex_buffer_offset);

        // write texture buffer id if available
        if(_mesh.texture_id != UINT32_MAX)
            _WriteNumericalValue<uint32_t>("TEXTUREID", _mesh.texture_id);

        // write texture map buffer id if available
        if(_mesh.texture_map_buffer_id != UINT32_MAX) {
            _WriteNumericalValue<uint32_t>("TEXTUREMAPBUFFERID", _mesh.texture_map_buffer_id);
            _WriteNumericalValue<uint32_t>("TEXTUREMAPBUFFEROFFSET", _mesh.texture_map_buffer_offset);
        }

        // write vertex normal buffer id if available
        if(_mesh.vertex_normal_buffer_id != UINT32_MAX) {
            _WriteNumericalValue<uint32_t>("VERTEXNORMALBUFFERID", _mesh.vertex_normal_buffer_id);
            _WriteNumericalValue<uint32_t>("VERTEXNORMALBUFFEROFFSET", _mesh.vertex_normal_buffer_offset);
        }

        _EndScope();
    }


    void DasWriterCore::WriteNode(const DasNode &_node) {
        _WriteScopeBeginning("NODE");

        if(_node.name != "") _WriteStringValue("NAME", _node.name);
        if(_node.children_count) {
            _WriteNumericalValue<uint32_t>("CHILDRENCOUNT", _node.children_count);
            _WriteArrayValue<uint32_t>("CHILDREN", _node.children_count, _node.children);
        }
        if(_node.mesh_count) {
            _WriteNumericalValue<uint32_t>("MESHCOUNT", _node.mesh_count);
            _WriteArrayValue<uint32_t>("MESHES", _node.mesh_count, _node.meshes);
        }
        if(_node.animation_count) {
            _WriteNumericalValue<uint32_t>("ANIMATIONCOUNT", _node.animation_count);
            _WriteArrayValue<uint32_t>("ANIMATIONS", _node.animation_count, _node.animations);
        }
        if(_node.skeleton_count) {
            _WriteNumericalValue<uint32_t>("SKELETONCOUNT", _node.skeleton_count);
            _WriteArrayValue<uint32_t>("SKELETONS", _node.skeleton_count, _node.skeletons);
        }
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
        _WriteNumericalValue<uint32_t>("JOINTCOUNT", _skeleton.joint_count);
        _WriteArrayValue<uint32_t>("JOINTS", _skeleton.joint_count, _skeleton.joints);

        _EndScope();
    }


    void DasWriterCore::WriteSkeletonJoint(const DasSkeletonJoint &_joint) {
        _WriteScopeBeginning("JOINT");
        _WriteMatrixValue<float>("INVERSEBINDPOS", _joint.inverse_bind_pos);

        if(_joint.name != "") _WriteStringValue("NAME", _joint.name);
        _WriteNumericalValue<uint32_t>("PARENT", _joint.parent);
        _WriteNumericalValue<float>("SCALE", _joint.scale);
        _WriteGenericDataValue(reinterpret_cast<const char*>(&_joint.rotation), sizeof(Quaternion), true, "ROTATION");
        _WriteGenericDataValue(reinterpret_cast<const char*>(&_joint.translation), sizeof(Vector3<float>), true, "TRANSLATION");

        _EndScope();
    }


    void DasWriterCore::WriteAnimation(const DasAnimation &_animation) {
        _WriteScopeBeginning("ANIMATION");
        if(_animation.name != "") _WriteStringValue("NAME", _animation.name);
        _WriteNumericalValue<uint32_t>("NODEID", _animation.node_id);
        _WriteNumericalValue<uint32_t>("DURATION", _animation.duration);
        _WriteNumericalValue<uint32_t>("KEYFRAMECOUNT", _animation.keyframe_count);
        _WriteArrayValue<float>("KEYFRAMETIMESTAMPS", _animation.keyframe_count, _animation.keyframe_timestamps);
        _WriteArrayValue<InterpolationType>("INTERPOLATIONTYPES", _animation.keyframe_count, _animation.interpolation_types);
        _WriteArrayValue<AnimationTarget>("ANIMATIONTYPES", _animation.keyframe_count, _animation.animation_targets);
        _WriteNumericalValue<uint32_t>("KEYFRAMEBUFFERID", _animation.keyframe_buffer_id);
        _WriteNumericalValue<uint32_t>("KEYFRAMEBUFFEROFFSET", _animation.keyframe_buffer_offset);

        _EndScope();
    }


    void DasWriterCore::AppendTextures(std::vector<DasBuffer> &_buffers, const std::vector<std::string> &_embedded_textures, bool _use_raw) {
        m_texture_readers.reserve(_embedded_textures.size());
        for(const std::string &file_name : _embedded_textures) {
            m_texture_readers.push_back(std::move(TextureReader(file_name, _use_raw)));
            
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
