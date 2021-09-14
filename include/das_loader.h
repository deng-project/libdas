/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: das_loader.h - das file format reader header file
/// author: Karl-Mihkel Ott


#ifndef __DAS_LOADER_H
#define __DAS_LOADER_H

#ifdef __DAS_LOADER_CPP
    #include <iostream>
    #include <fstream>
    #include <cstring>
    #include <cassert>

    #include <uuid.h>
    #include <assets.h>
    #include <das_file.h>
    #include <msg_assert.h>
#endif


/// API bindings
//void das_LoadAsset(das_Asset *asset, das_AssetMode dst_mode, das_ColorData color,
                   //char **meta, uuid_t *tex_uuid, const char *file_name);

namespace libdas {

    class das_loader {
        private:
            Asset m_asset;

        protected:
            std::fstream m_file;
            uint64_t m_file_size;
            uint64_t m_offset;

        protected:
            void readFILE_HDR(FILE_HDR *fhdr, const std::string &file_name);
            void readINFO_HDR(INFO_HDR *ifhdr, const std::string &file_name);
            bool tryToReadMeta(META_HDR *meta, const std::string &file_name);
            void skipMetaHeaders(const std::string &file_name);
            void readVERT_HDR(VERT_HDR *vhdr, const std::string &file_name);
            void readVertAttr(VertAttribute *ahdr, uint64_t exsig, const std::string &file_name, uint64_t pos_size);
            void readINDX_HDR(AssetMode amode, INDX_HDR *ihdr, const std::string &file_name, bool read_indices);

            /// Data manipulation algoritms
            void vertNormalise(Asset *asset);
            void convertAssetMode(Asset *asset);

        public:
            das_loader();
            das_loader(AssetMode dst_mode, const ColorData &color, char **meta, uuid_t *tex_uuid, const std::string &file_name);

            void loadAsset(AssetMode dst_mode, const ColorData &color, char **meta, uuid_t *tex_uuid, const std::string &file_name);
            static void cleanupAsset(const Asset &asset);
            Asset &getAsset();
    };
}
#endif
