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


    void DasWriterCore::_WriteGenericDataValue(const std::string &_value_name, const char *_data, const size_t _len) {
        LIBDAS_ASSERT(m_out_stream.is_open());

        m_out_stream.write(_value_name.c_str(), _value_name.size());
        m_out_stream.write(": ", 2);
        m_out_stream.write(_data, _len);
        m_out_stream.write(LIBDAS_DAS_NEWLINE, strlen(LIBDAS_DAS_NEWLINE));
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


    void DasWriterCore::InitialiseFile(const DasProperties &_properties) {
        DasSignature sig;
        m_use_compression = _properties.compression;
        m_out_stream.write(reinterpret_cast<char*>(&sig), sizeof(DasSignature));

        _WriteScopeBeginning("PROPERTIES");

        // write model name if present
        if(_properties.model != "")
            _WriteStringValue("MODEL", _properties.model);

        // write author name if present
        if(_properties.author != "")
            _WriteStringValue("AUTHOR", _properties.author);

        // write modification date
        const auto epoch = std::chrono::system_clock::now().time_since_epoch();
        const auto s = std::chrono::duration_cast<std::chrono::seconds>(epoch);
        _WriteNumericalValue<uint64_t>("MODDATE", static_cast<uint64_t>(s.count()));
        
        // write compression value
        _WriteNumericalValue<bool>("COMPRESSION", _properties.compression);

        _EndScope();
    }


    void DasWriterCore::WriteBuffer(const DasBuffer &_buffer) {
        _WriteScopeBeginning("BUFFER");
        _WriteNumericalValue<BufferType>("BUFFERTYPE", _buffer.type);
        _WriteNumericalValue<uint32_t>("DATALEN", _buffer.data_len);
        _WriteGenericDataValue("DATA", _buffer.data, _buffer.data_len);
        _EndScope();
    }


    void DasWriterCore::WriteModel(const DasModel &_model) {
        _WriteScopeBeginning("MODEL");
        
        // check if name should be written
        if(_model.name != "")
            _WriteStringValue("NAME", _model.name);

        // write numerical buffer related values
        _WriteNumericalValue<uint32_t>("INDEXBUFFERID", _model.index_buffer_id);
        _WriteNumericalValue<uint32_t>("INDEXBUFFEROFFSET", _model.index_buffer_offset);
        _WriteNumericalValue<uint32_t>("INDICESCOUNT", _model.indices_count);
        _WriteNumericalValue<uint32_t>("VERTEXBUFFERID", _model.vertex_buffer_id);
        _WriteNumericalValue<uint32_t>("VERTEXBUFFEROFFSET", _model.vertex_buffer_offset);
        _WriteNumericalValue<uint32_t>("TEXTUREID", _model.texture_id);
        _WriteNumericalValue<uint32_t>("TEXTUREMAPBUFFERID", _model.texture_map_buffer_id);
        _WriteNumericalValue<uint32_t>("TEXTUREMAPBUFFEROFFSET", _model.texture_map_buffer_offset);
        _WriteNumericalValue<uint32_t>("VERTEXNORMALBUFFERID", _model.vertex_normal_buffer_id);
        _WriteNumericalValue<uint32_t>("VERTEXNORMALBUFFEROFFSET", _model.vertex_normal_buffer_offset);
        _WriteMatrixValue("TRANSFORM", _model.transform);

        _EndScope();
    }
}
