/// libdas: DENG asset loader and assembler library
/// licence: Apache, see LICENCE file
/// file: dam.h - deng asset manager header file
/// author: Karl-Mihkel Ott


#ifndef __DAM_H
#define __DAM_H


/// DAM has different versioning than DENG
#define MAJOR       0
#define MINOR       1
#define REVISION    "1r1"

#define META_LEN        2048
#define BUF_LEN         256

#ifdef __DAM_C
    #define __DAM_PREVIEW_TITLE     "DAM"
    #include <iostream>
    #include <fstream>
    #include <vector>
    #include <cstring>
    #include <strings.h>
    #include <unordered_map>

    #include <uuid.h>
    #include <assets.h>
    #include <das_file.h>
    #include <das_loader.h>
    #include <msg_assert.h>
    #include <das_asset_writer.h>
    #include <wobj.h>
#endif

#define MAX_STR    4096
/// Structure for containing all info about assembling a new asset
#define VUU     0x76757500
#define VUN     0x76756e00
#define VMU     0x766d7500
#define VMN     0x766d6e00


namespace libdas {

    struct dam_Options {
        char *input = nullptr;
        char *out_file = nullptr;
        char *meta = nullptr;
        uint32_t vmode = VMN;
        uint8_t cmp = 0;
        bool show_finfo = false;
    };


    enum AssetSrcFileFormat {
        FORMAT_UNKNOWN          =       -1,
        WAVEFRONT_OBJ           =       0,
        STANDARD_TRIANGLE_LANG  =       1, // reserved for future use
        GLTF                    =       2, // reserved for future use
        FLUXBOX                 =       3 // reserved for future use
    };


    class dam : das_loader {
        private:
            const char *m_to_help = "For more information use -h or --help flag\n";
            const char *m_help_text = 
                "Basic usage of DAM: dam [OPTIONS] <DAS_FILE>\n" \
                "-h / --help -- view help text\n" \
                "-i / --input <INPUT> -- input file name\n" \
                "-v / --vert <vuu, vun, vmu, vmn> -- specify the new asset vertices mode\n" \
                "-nm / --no-meta -- display information about asset without any metadata (DEFAULT)\n" \
                "-m / --meta -- read metadata from stdin\n" \
                "-c / --compression <LEVEL> -- compress the file with 0 (no compression) or 1 (huffman coding)\n";

        public:
            AssetSrcFileFormat findSrcFormat(const std::string &file_name);
            dam_Options parseInput(int argc, char *argv[]);
            uint32_t selectWObjGroup(uint32_t *nz_objs, uint32_t nz_oc, uint32_t *nz_grps, uint32_t nz_gc, WavefrontObjEntityType &t, std::vector<WavefrontObjEntity> &ents);
            void listAssetData(const std::string &file_name);
            void mkAssetFromWObj(Asset &asset, std::vector<WavefrontObjEntity> &ents);
            void quadTriangulate(Asset &asset);
    };
}
#endif

