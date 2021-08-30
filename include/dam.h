/// libdas: DENG asset loader and assembler library
/// licence: Apache, see LICENCE file
/// file: dam.h - deng asset manager header file
/// author: Karl-Mihkel Ott


#ifndef __DAM_H
#define __DAM_H


/// DAM has different versioning than DENG
#define __DAM_VERSION "0.1.1"
#define MAJOR       0
#define MINOR       1
#define REVISION    0

#define META_LEN        2048
#define BUF_LEN         256

#ifdef __DAM_C
    #define __DAM_PREVIEW_TITLE     "DAM"
    #include <stdlib.h>
    #include <stdio.h>
    #include <stdint.h>
    #include <stdbool.h>
    #include <time.h>
    #include <string.h>

    #include <uuid.h>
    #include <assets.h>
    #include <das_file.h>
    #include <das_loader.h>
    #include <das_asset_assembler.h>
    #include <wobj.h>
#endif


#ifdef __DAM_C

#define MAX_STR    4096
/// Structure for containing all info about assembling a new asset
#define VUU     0x76757500
#define VUN     0x76756e00
#define VMU     0x766d7500
#define VMN     0x766d6e00

typedef struct dam_Options {
    char *input;
    char *das_file;
    char *meta;
    uint32_t vmode;
    bool show_finfo;
} dam_Options;


typedef enum AssetSrcFileFormat {
    FORMAT_UNKNOWN          =       -1,
    WAVEFRONT_OBJ           =       0,
    STANDARD_TRIANGLE_LANG  =       1, // reserved for future use
    GLTF                    =       2, // reserved for future use
    FLUXBOX                 =       3 // reserved for future use
} AssetSrcFileFormat;

static const char *__to_help = "For more information use -h or --help flag\n";
static const char *__help_text = 
    "Basic usage of DAM: dam [OPTIONS] <DAS_FILE>\n" \
    "-h / --help -- view help text\n" \
    "-i / --input <INPUT> -- input file name\n" \
    "-v / --vert <vuu, vun, vmu, vmn> -- specify the new asset vertices mode\n" \
    "-nm / --no-meta -- display information about asset without any metadata (DEFAULT)\n" \
    "-m / --meta -- read metadata from stdin\n";


static AssetSrcFileFormat findSrcFormat(char *file_name);
static dam_Options parseInput(int argc, char *argv[]);
static uint32_t selectWObjGroup(uint32_t *nz_objs, uint32_t nz_oc, uint32_t *nz_grps, uint32_t nz_gc, char *gt,
                                das_WavefrontObjEntity *ents);
static void clearAssetData(das_Asset *asset);
static void listAssetData(const char *file_name);
static void mkAssetFromWObj(das_Asset *asset, das_WavefrontObjEntity *ents, uint32_t ent_c);
#endif

#endif

