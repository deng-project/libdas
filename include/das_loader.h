/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: das_loader.h - das file format reader header file
/// author: Karl-Mihkel Ott


#ifndef __DAS_LOADER_H
#define __DAS_LOADER_H

#ifdef __cplusplus
extern "C" {
#endif


#if defined(__DAS_LOADER_C) || defined(__DAM_C)
    #include <stdlib.h>
    #include <stdbool.h>
    #include <stddef.h>
    #include <stdint.h>
    #include <stdio.h>
    #include <string.h>

    #include <assets.h>
    #include <das_file.h>

    // Reading functions
    void readFILE_HDR(das_FILE_HDR *fhdr, const char *file_name);
    void readINFO_HDR(das_INFO_HDR *ifhdr, const char *file_name);
    bool tryToReadMeta(das_META_HDR *meta, const char *file_name);
    void skipMetaHeaders(const char *file_name);
    void readVERT_HDR(das_VERT_HDR *vhdr, const char *file_name);
    void readGenVertHdr(__das_VertTemplate *thdr, uint64_t exsig, const char *file_name);
    void readINDX_HDR(das_AssetMode amode, das_INDX_HDR *ihdr, const char *file_name);


    /// Data manipulation algoritms
    void vertNormalise(das_Asset *asset);
    void convertAssetMode(das_Asset *asset);


    /// Debugging
	#ifdef __DEBUG
		#define __ASSET_LOG_FILE        "assets.log"
		static void _db_Asset(das_Asset *asset, const char *file_name);
	#else
        #define __ASSET_LOG_FILE
	#endif


    // Global static variables for DAS reader
    static FILE *__sfile = NULL;
    static uint64_t __flen = 0;
    static uint64_t __offset = 0;
#endif


#ifdef DAS_FILE
    #define EMSG_LEN        2048
    #define ZB(t) memset(&t, 0, sizeof(t))

    // File reading headers
    void openFileStreamRO(const char *file_name); 
    void openFileStreamWO(const char *file_name);
    void dataRead(void *buf, size_t s, const char *emsg, const char *file_name);
    void dataWrite(void *buf, size_t s, const char *emsg, const char *file_name);
    //void seekStream();
    void skipStreamRO(size_t len, const char *esmg, const char *file_name);
    void closeFileStream();
#endif


/// API bindings
void das_LoadAsset(
    das_Asset *asset, 
    das_AssetMode dst_mode,
    das_ObjColorData color,
    char **meta,
    const char *tex_uuid,
    const char *file_name
);

#ifdef __cplusplus
}
#endif
#endif
