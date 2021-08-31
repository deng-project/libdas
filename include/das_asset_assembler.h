/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: das_asset_assembler.h - das binary writer header
/// author: Karl-Mihkel Ott


#ifndef __DAS_ASSET_ASSEMBLER_H
#define __DAS_ASSET_ASSEMBLER_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __DAS_ASSET_ASSEMBLER_C
    #include <stdlib.h>
    #include <stdint.h>
    #include <stdbool.h>
    #include <stdio.h>
    #include <string.h>
    #include <time.h>

    #include <uuid.h>
    #include <assets.h>
    #include <das_file.h>
    #include <das_loader.h>

    /**********************************/
    /**** Static asset data writer ****/
    /**********************************/

    static void writeFILE_HDR(const char *file_name);
    static void writeINFO_HDR(const das_Asset *asset, const char *file_name);
    static void writeMETA_HDR(char *meta, const char *file_name);
    static void writeVERT_HDR(const das_Asset *asset, const char *file_name);
    static void writeGenericVertAttrHDR(void *vd, uint32_t vc, uint32_t esize, uint64_t sig,
                                        const char *emsg, const char *file_name, uint64_t pos_size);
    static void writeINDX_HDR(const das_Asset *asset, const char *file_name);
#endif


void das_CreateAssetFile(das_Asset *asset, const char *file_name, char *meta);

#ifdef __cplusplus
}
#endif
#endif
