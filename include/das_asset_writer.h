/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: das_asset_writer.h - das binary writer header
/// author: Karl-Mihkel Ott


#ifndef __DAS_ASSET_ASSEMBLER_H
#define __DAS_ASSET_ASSEMBLER_H

#ifdef __DAS_ASSET_WRITER_CPP
    #include <string>
    #include <cstring>
    #include <iostream>
    #include <fstream>

    #include <uuid.h>
    #include <assets.h>
    #include <das_file.h>
    #include <msg_assert.h>
#endif


namespace libdas {

    class AssetWriter {
        private:
            std::fstream m_file;

        private:
            void writeFILE_HDR();
            void writeINFO_HDR(const Asset &asset);
            void writeMETA_HDR(const char *meta);
            void writeVERT_HDR(const Asset &asset);
            void writeGenericVertAttrHDR(void *vd, uint32_t vc, uint32_t esize, uint64_t sig,
                                         const char *emsg, uint64_t pos_size);
            void writeINDX_HDR(const Asset &asset);

        public:
            AssetWriter();
            AssetWriter(const Asset &asset, const std::string &file_name, const char *meta);

            /// Explicitly create a new asset file from given data
            void createAssetFile(const Asset &asset, const std::string &file_name, const char *meta);
    };

}

#endif
