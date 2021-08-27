/// libdas: DENG asset loader and assembler library
/// licence: Apache, see LICENCE file
/// file: das_asset_assembler.h - assembler source code for das binary files
/// author: Karl-Mihkel Ott


#define __DAS_ASSET_ASSEMBLER_C
#include <data/das_asset_assembler.h>


/***********************************/
/**** Header assembly functions ****/
/***********************************/

/// Assemble the info header of the asset
/// NOTE: This function call assumes that __wfile is a valid pointer to a stream
static void __das_AssembleINFO_HDR(das_AssetMode dst_mode) {
    char *uuid = uuid_Generate();
    das_INFO_HDR inf_hdr = { 
        .magic_number = __DAS_ASSET_MAGIC_NUMBER,
        .hdr_name = __DAS_INFO_HEADER_NAME_NN, 
        .hdr_size = __DAS_INFO_HEADER_SIZE,
        .uuid = { 0 },
        .time_st = (deng_ui64_t) time(NULL),
        .asset_type = dst_mode,

        // Temporary
        .cmpr = 0
    };

    strncpy(inf_hdr.uuid, uuid, __DAS_UUID_LEN);
    inf_hdr.asset_type = dst_mode;

    fwrite(&inf_hdr, sizeof(das_INFO_HDR), 1, __wfile);
}


/// Assemble vertex header of the asset
/// NOTE: This function call assumes that __wfile is a valid pointer to a stream
static void __das_AssembleVERT_HDR(das_VertDynamic vert, das_AssetMode dst_mode) {
    das_VERT_HDR vert_hdr = {
        .hdr_name = __DAS_VERTICES_HEADER_NAME_NN
    };

    // Check the vertices' destination mode for calculating the header size
    switch(dst_mode) {
    case __DAS_ASSET_MODE_3D_UNMAPPED_UNOR:
        vert_hdr.hdr_size = sizeof(__das_VertTemplate) + vert.v3d.mul.pn * sizeof(das_ObjPosData) + 28;
        break;

    case DAS_ASSET_MODE_3D_UNMAPPED:
        // Store only vertex position data
        vert_hdr.hdr_size = 2 * sizeof(__das_VertTemplate) + vert.v3d.mul.pn * sizeof(das_ObjPosData) + 
            vert.v3d.mul.nn + sizeof(das_ObjNormalData) + 28;
        break;

    case __DAS_ASSET_MODE_3D_TEXTURE_MAPPED_UNOR:
        vert_hdr.hdr_size = 2 * sizeof(__das_VertTemplate) + vert.v3d.mul.pn * (sizeof(das_ObjPosData) + 
            vert.v3d.mul.tn * sizeof(das_ObjTextureData)) + 28;
        break;

    case DAS_ASSET_MODE_3D_TEXTURE_MAPPED:
        vert_hdr.hdr_size = 3 * sizeof(__das_VertTemplate) + vert.v3d.mul.pn * sizeof(das_ObjPosData) + 
            vert.v3d.mul.tn * sizeof(das_ObjTextureData) + vert.v3d.mul.nn * sizeof(das_ObjNormalData) + 28;
        break;

    default:
        break;
    }

    // Write vertices header data into stream
    fwrite(&vert_hdr, sizeof(das_VERT_HDR), 1, __wfile);

    // Check which subheaders to build
    __das_AssembleVPOS_HDR(&vert);
    switch(dst_mode) {
    case DAS_ASSET_MODE_3D_UNMAPPED:
        __das_AssembleVNOR_HDR(&vert);
        break;

    case __DAS_ASSET_MODE_3D_TEXTURE_MAPPED_UNOR:
        __das_AssembleVTEX_HDR(&vert);
        break;

    case DAS_ASSET_MODE_3D_TEXTURE_MAPPED:
        __das_AssembleVTEX_HDR(&vert);
        __das_AssembleVNOR_HDR(&vert);
        break;

    default:
        break;
    }
}


/// Assemble vertex position subheader
/// NOTE: This function call assumes that __wfile is a valid pointer to a stream
static void __das_AssembleVPOS_HDR(das_VertDynamic *p_vert) {
    das_VPOS_HDR vpos_hdr = {
        .hdr_name = __DAS_VERT_POSITION_HEADER_NAME_NN,
        .vert_c = p_vert->v3d.mul.pn,
        .hdr_size = p_vert->v3d.mul.pn * sizeof(das_ObjPosData) + 16
    };

    // Write the header beginning to stream
    fwrite(&vpos_hdr, sizeof(das_VPOS_HDR), 1, __wfile);

    // Write vertices data to the stream
    fwrite(p_vert->v3d.mul.pos, sizeof(das_ObjPosData), vpos_hdr.vert_c, __wfile);
}


/// Assemble texture position subheader
/// NOTE: This function call assumes that __wfile is a valid pointer to a stream
static void __das_AssembleVTEX_HDR(das_VertDynamic *p_vert) { 
    das_VTEX_HDR vtex_hdr = {
        .hdr_name = __DAS_TEX_POSITION_HEADER_NAME_NN,
        .vert_c = p_vert->v3d.mul.tn,
        .hdr_size = p_vert->v3d.mul.tn * sizeof(das_ObjTextureData) + 16
    };
    
    // Write the header beginning to stream
    fwrite(&vtex_hdr, sizeof(das_VTEX_HDR), 1, __wfile);

    // Write vertices data to the stream
    fwrite(p_vert->v3d.mul.tex, sizeof(das_ObjTextureData), vtex_hdr.vert_c, __wfile);
}


/// Assemble texture position subheader
/// NOTE: This function call assumes that __wfile is a valid pointer to a stream
static void __das_AssembleVNOR_HDR(das_VertDynamic *p_vert) { 
    das_VTEX_HDR vnor_hdr = {
        .hdr_name = __DAS_NORM_POSITION_HEADER_NAME_NN,
        .vert_c = p_vert->v3d.mul.nn,
        .hdr_size = p_vert->v3d.mul.nn * sizeof(das_ObjNormalData) + 16
    };
    
    // Write the header beginning to stream
    fwrite(&vnor_hdr, sizeof(das_VNOR_HDR), 1, __wfile);

    // Write vertices data to the stream
    fwrite(p_vert->v3d.mul.norm, sizeof(das_ObjNormalData), vnor_hdr.vert_c, __wfile);
}

    
/// Assemble META_HDR with additional meta data 
/// NOTE: This function call assumes that __wfile is a valid pointer to a stream
static void __das_AssembleMETA_HDR(char *meta_data) {
    das_META_HDR meta_hdr = {
        .beg = __DAS_META_HEADER_NAME,
        .hdr_size = 16 + strlen(meta_data),
        .data_size = strlen(meta_data),
        .data = meta_data,
    };

    
    // Write the first part of the header into the stream
    fwrite(&meta_hdr, 8, 2, __wfile);

    // Write metadata into the stream
    fwrite(meta_hdr.data, sizeof(char), strlen(meta_hdr.data), __wfile);
}


/// Assemble indices header of the asset
/// NOTE: This function call assumes that __wfile is a valid pointer to a stream
static void __das_AssembleINDX_HDR(das_IndicesDynamic inds, das_AssetMode am) {
    // 
    das_INDX_HDR indx_hdr = {
        .hdr_name = __DAS_INDICES_HEADER_NAME,
        .hdr_size = (deng_ui32_t) (16 + inds.n * sizeof(deng_ui32_t)),
        .ind_c = (deng_ui32_t) inds.n
    };

    // Write the header into the stream
    fwrite(&indx_hdr, sizeof(das_INDX_HDR), 1, __wfile);

    // Write position indices array into the stream
    fwrite(inds.pos, sizeof(deng_ui32_t), inds.n, __wfile);

    // Write position indices array into the stream
    if(am == __DAS_ASSET_MODE_3D_TEXTURE_MAPPED_UNOR || 
       am == DAS_ASSET_MODE_3D_TEXTURE_MAPPED)
        fwrite(inds.tex, sizeof(deng_ui32_t), inds.n, __wfile);

    // Write position indices array into the stream
    if(am == DAS_ASSET_MODE_3D_UNMAPPED ||
       am == DAS_ASSET_MODE_3D_TEXTURE_MAPPED)
        fwrite(inds.norm, sizeof(deng_ui32_t), inds.n, __wfile);
}


/// Open new file stream for writing 
static void __das_OpenFileStream(char *file_name) {
    __wfile = fopen(file_name, "wb");
    if(!__wfile)
        FILE_ERR(file_name);
}


/// Close the current file stream
static void __das_CloseFileStream() {
    if(__wfile) {
        fclose(__wfile);
        __wfile = NULL;
    }
}


/// Assemble static DENG asset from das_Asset instance
void das_StaticAssemble (
    das_Asset *p_asset,
    char *file_name,
    char **meta_data,
    size_t meta_c
) {
    __das_OpenFileStream(file_name);

    // Assemble INFO_HDR
    __das_AssembleINFO_HDR(p_asset->asset_mode);

    // Check for any metadata and write it to the stream if it exists
    for(size_t i = 0; i < meta_c; i++)
        __das_AssembleMETA_HDR(meta_data[i]);

    // Assemble VERT_HDR
    __das_AssembleVERT_HDR(p_asset->vertices, p_asset->asset_mode);

    // Assemble INDX_HDR
    __das_AssembleINDX_HDR(p_asset->indices, p_asset->asset_mode);
    __das_CloseFileStream();
}

