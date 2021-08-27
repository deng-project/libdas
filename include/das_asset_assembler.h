

#ifndef __DAS_ASSET_ASSEMBLER_H
#define __DAS_ASSET_ASSEMBLER_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __DAS_ASSET_ASSEMBLER_C
    #include <stdlib.h>
    #include <stdio.h>
    #include <string.h>
    #include <time.h>

    #include <common/base_types.h>
    #include <common/uuid.h>
    #include <common/cerr_def.h>
    #include <data/assets.h>
    #include <data/das_file.h>

    /***********************************/
    /**** Header assembly functions ****/
    /***********************************/

    /*
     * Assemble the info header of the asset
     * This function call assumes that __wfile is a valid pointer to a stream
     */
    static void __das_AssembleINFO_HDR(das_AssetMode dst_mode);


    /*
     * Assemble vertex header of the asset
     * This function call assumes that __wfile is a valid pointer to a stream
     */
    static void __das_AssembleVERT_HDR(das_VertDynamic vert, das_AssetMode dst_mode);


    /*
     * Assemble vertex position subheader
     * This function call assumes that __wfile is a valid pointer to a stream
     */
    static void __das_AssembleVPOS_HDR(das_VertDynamic *p_vert);


    /*
     * Assemble texture position subheader
     * This function call assumes that __wfile is a valid pointer to a stream
     */
    static void __das_AssembleVTEX_HDR(das_VertDynamic *p_vert);


    /*
     * Assemble normal position subheader
     * This function call assumes that __wfile is a valid pointer to a stream
     */
    static void __das_AssembleVNOR_HDR(das_VertDynamic *p_vert);

        
    /*
     * Assemble META_HDR with additional meta data 
     * This function call assumes that __wfile is a valid pointer to a stream
     */
    static void __das_AssembleMETA_HDR(char *meta_data);


    /*
     * Assemble indices header of the asset
     * This function call assumes that __wfile is a valid pointer to a stream
     */
    static void __das_AssembleINDX_HDR(das_IndicesDynamic inds, das_AssetMode am);


    /*
     * Open new file stream for writing 
     */
    static void __das_OpenFileStream(char *file_name);
    

    /*
     * Close the current file stream
     */
    static void __das_CloseFileStream();

    FILE *__wfile = NULL;
#endif


/*
 * Assemble static DENG asset file from das_Asset instance
 */
void das_StaticAssemble (
    das_Asset *p_asset,
    char *file_name,
    char **meta_data,
    size_t meta_c
);

#ifdef __cplusplus
}
#endif
#endif
