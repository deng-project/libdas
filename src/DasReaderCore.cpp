/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: DasReaderCore.h - DAS file format reader class header
/// author: Karl-Mihkel Ott

#define DAS_READER_CORE_CPP
#include <DasReaderCore.h>

namespace Libdas {

    DasReaderCore::DasReaderCore(const std::string &_file_name) : 
        AsciiLineReader(_file_name), m_file_name(_file_name), m_error(MODEL_FORMAT_DAS) 
    {
        // open file stream if file name was specified
        if(m_file_name != "")
            m_in_stream.open(m_file_name, std::ios_base::binary);
    }


    void DasReaderCore::NewFile(const std::string &_file_name) {
        LIBDAS_ASSERT(_file_name != "");

        // close the previous stream if it was opened
        CloseStream();

        // open the file stream
        m_in_stream.open(_file_name, std::ios_base::binary);
    }


    void DasReaderCore::CloseStream() {
        if(m_in_stream.is_open())
            m_in_stream.close();
    }


    void DasReaderCore::ReadSignature() {
        DasSignature exp_sig;
        DasSignature sig;

        m_in_stream.read(reinterpret_cast<char*>(&sig), sizeof(DasSignature));
        bool is_pad = false;
        
        // verify signature integrity
        if(sig.magic == exp_sig.magic) {
            // verify padding values, zero byte padding
            if(strncmp(sig.padding, exp_sig.padding, sizeof(sig.padding))) {
                // check newline byte padding
                memset(exp_sig.padding, 0x0a, 12);
                if(strncmp(sig.padding, exp_sig.padding, 12)) {
                    // check whitespace byte padding
                    memset(exp_sig.padding, 0x20, 12);
                    if(!strncmp(sig.padding, exp_sig.padding, 12))
                        is_pad = true;
                }

                else is_pad = true;
            }

            else is_pad = true;
        }

        if(is_pad != exp_sig.magic) m_error.Error(LIBDAS_ERROR_INVALID_SIGNATURE);
    }


    DasProperties DasReaderCore::ReadProperties() {
        //const std::string exp_prop_scope = "PROPERTIES";

        //SkipSkippableCharacters();
        //char *beg = GetReadPtr();
        //char *end = ExtractWord();

        //std::string prop_str = std::string(beg, end);
        //if(prop_str != exp_prop_scope)
            //m_error.Error(LIBDAS_ERROR_INVALID_DATA, prop_str);

        
        return DasProperties();
    }
}
