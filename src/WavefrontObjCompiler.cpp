/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: WavefrontObjCompiler.cpp - Wavefront OBJ to DAS compiler class implementation
/// author: Karl-Mihkel Ott

#define WAVEFRONT_OBJ_COMPILER_CPP
#include <WavefrontObjCompiler.h>


namespace Libdas {

    WavefrontObjCompiler::WavefrontObjCompiler(const std::string &_out_file) :
        m_out_file(_out_file) 
    {
        // open a output stream if possible
        if(m_out_file != "") {
            _CheckAndAddFileExtension();
            _OpenFileStream();
        }
    }


    WavefrontObjCompiler::WavefrontObjCompiler(std::queue<WavefrontObjGroup> &_groups, const std::string &_out_file) :
        m_out_file(_out_file)
    {
        LIBDAS_ASSERT(m_out_file != "");

        // open an output stream
        _CheckAndAddFileExtension();
        _OpenFileStream();

        Compile(_groups);
    }


    WavefrontObjCompiler::~WavefrontObjCompiler() {
        if(m_out_stream.is_open())
            m_out_stream.close();
    }




    void WavefrontObjCompiler::Compile(std::queue<WavefrontObjGroup> &_groups, const std::string &_out_file) {
        // new file name was supplied, open the stream
        if(_out_file != "") {
            m_out_file = _out_file;
            _CheckAndAddFileExtension();
            _OpenFileStream();
        }
    }
}
