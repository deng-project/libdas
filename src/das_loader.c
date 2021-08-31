/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: das_loader.c - das file format reader implementation file
/// author: Karl-Mihkel Ott


#define DAS_FILE
#define __DAS_LOADER_C
#include <das_loader.h>


/// Read .das binary asset file into das_Asset instance
void das_LoadAsset (
    das_Asset *asset, 
    das_AssetMode dst_mode,
    das_ColorData color,
    char **meta,
    id_t *tex_uuid,
    const char *file_name
) {
    asset->tex_uuid = tex_uuid;
    strcpy(asset->src, file_name);
    asset->diffuse = DAS_DEFAULT_DIFFUSE_COLOR;
    asset->ambient = DAS_DEFAULT_AMBIENT_COLOR;
    asset->specular = DAS_DEFAULT_SPECULAR_COLOR;
    asset->is_shown = true;
    asset->ignore_transform = false;
    asset->force_unmap = false;

    // Open file stream for reading
    openFileStreamRO(file_name);

    das_FILE_HDR file_hdr = { 0 };
    das_INFO_HDR info_hdr = { 0 };
    das_META_HDR meta_hdr = { 0 };
    das_VERT_HDR vert_hdr = { 0 };
    das_INDX_HDR indx_hdr = { 0 };

    readFILE_HDR(&file_hdr, file_name);
    readINFO_HDR(&info_hdr, file_name);

    asset->asset_mode = info_hdr.asset_type;
    strcpy(asset->uuid.bytes, info_hdr.uuid);

    // Check if metadata should be read or ignored
    if(meta) {
        tryToReadMeta(&meta_hdr, file_name);
        *meta = meta_hdr.data;
    }
    else skipMetaHeaders(file_name);

    // Read vertices header
    readVERT_HDR(&vert_hdr, file_name);

    das_VertAttribute attr;
    ZB(attr);

    // Read vertex attributes starting from position elements, texture elements and finishing on normal elements
    readVertAttr(&attr, DAS_VPOS_HEADER_SIG, file_name, sizeof(das_PosData));

    das_TextureData **ptex = NULL;
    size_t *ptn = NULL;

    // Check vertex position data element count
    switch(attr.esize) {
        case 2:
            asset->vertices.v2d.mul.pn = attr.vert_c;
            asset->vertices.v2d.mul.pos = (das_PosData2D*) malloc(attr.vert_c * sizeof(das_PosData2D));
            dataRead(asset->vertices.v3d.mul.pos, attr.vert_c * sizeof(das_PosData2D),
                     "Could not read vertex position attributes, possibly corrupted file",
                     file_name);
            ptex = &asset->vertices.v2d.mul.tex;
            ptn = &asset->vertices.v2d.mul.tn;
            break;

        case 3:
            asset->vertices.v3d.mul.pn = attr.vert_c;
            asset->vertices.v3d.mul.pos = (das_PosData*) malloc(attr.vert_c * sizeof(das_PosData));
            dataRead(asset->vertices.v3d.mul.pos, attr.vert_c * sizeof(das_PosData),
                     "Could not read vertex 3D position attributes, possibly corrupted file",
                     file_name);
            ptex = &asset->vertices.v3d.mul.tex;
            ptn = &asset->vertices.v3d.mul.tn;
            break;

        default:
            ptex = &asset->vertices.v3d.mul.tex;
            ptn = &asset->vertices.v3d.mul.tn;
            break;
    }

    // Read texture data if needed
    if(asset->asset_mode == DAS_ASSET_MODE_3D_TEXTURE_MAPPED ||
       asset->asset_mode == __DAS_ASSET_MODE_3D_TEXTURE_MAPPED_UNOR ||
       asset->asset_mode == DAS_ASSET_MODE_2D_TEXTURE_MAPPED) {
        ZB(attr);
        readVertAttr(&attr, DAS_VTEX_HEADER_SIG, file_name, 0);

        *ptn = attr.vert_c;
        *ptex = (das_TextureData*) malloc((*ptn) * sizeof(das_TextureData));
        dataRead(*ptex, (*ptn) * sizeof(das_TextureData), 
                 "Could not read vertex texture position attributes, possibly corrupted file",
                 file_name);
    }

    // Read vertex normals if needed
    if(asset->asset_mode == DAS_ASSET_MODE_3D_TEXTURE_MAPPED ||
       asset->asset_mode == DAS_ASSET_MODE_3D_UNMAPPED) {
        ZB(attr);
        readVertAttr(&attr, DAS_VNOR_HEADER_SIG, file_name, 0);

        asset->vertices.v3d.mul.nn = attr.vert_c;
        asset->vertices.v3d.mul.norm = (das_NormalData*) malloc(attr.vert_c * sizeof(das_NormalData));
        dataRead(asset->vertices.v3d.mul.norm, asset->vertices.v3d.mul.nn * sizeof(das_NormalData),
                 "Could not read vertex normal attributes, possibly corrupted file",
                 file_name);
    }


    // Read and copy index data
    readINDX_HDR(asset->asset_mode, &indx_hdr, file_name, true);
    asset->indices.n = indx_hdr.ind_c;
    asset->indices.pos = indx_hdr.pinds;
    asset->indices.tex = indx_hdr.tinds;
    asset->indices.norm = indx_hdr.ninds;

    closeFileStream();

	#ifdef __DEBUG
		_db_Asset(asset, file_name);
	#endif
}


/// Open file for reading and set the buffer length
void openFileStreamRO(const char *file_name) {
    DAS_FROASSERT(__sfile = fopen(file_name, "rb"),
               "Could not open file for reading",
               file_name);

    // Find the length of the file
    fseek(__sfile, 0, SEEK_END);
    __flen = (uint64_t) ftell(__sfile);
    fseek(__sfile, 0, SEEK_SET);
}



/// Open file for writing and set the buffer length as infinite
void openFileStreamWO(const char *file_name) {
    DAS_FROASSERT(__sfile = fopen(file_name, "wb"),
              "Could not open file for writing",
              file_name);

    __flen = UINT64_MAX;
}


/// Read generic chunk of data from the current file stream
void dataRead(void *buf, size_t s, const char *emsg, const char *file_name) {
    DAS_FROASSERT(__offset + s <= __flen, emsg, file_name);
    fread(buf, s, 1, __sfile);
    __offset += s;
}

/// Write generic chunk of data into current file stream
void dataWrite(void *buf, size_t s, const char *emsg, const char *file_name) {
    DAS_FWOASSERT(fwrite(buf, s, 1, __sfile),
                  emsg, file_name);
}


/// Skip read only file stream from current offset
void skipStreamRO(size_t len, const char *emsg, const char *file_name) {
    DAS_FROASSERT(__offset + len <= __flen, emsg, file_name);
    __offset += len;
    fseek(__sfile, __offset, SEEK_SET);
}


/// Close generic file stream
void closeFileStream() {
    fclose(__sfile);
    __sfile = NULL;
    __offset = 0;
    __flen = 0;
}


/// Get the length of the current FILE stream
uint64_t getBufferLen() {
    return __flen;
}


/// Get the current reading offset of the FILE stream
uint64_t getOffset() {
    return __offset;
}


/**********************************************/
/********** Header reading functions **********/
/**********************************************/


/// Read file header containing the file signature and padding
void readFILE_HDR(das_FILE_HDR *fhdr, const char *file_name) {
    char emsg[EMSG_LEN] = { 0 };
    sprintf(emsg, "Could not read FILE_HDR, potentially corrupt file or stream");
    dataRead(fhdr, sizeof(das_FILE_HDR), emsg, file_name);

    DAS_FROASSERT(fhdr->hdr_sig == DAS_FILE_HEADER_SIG,
                  "Could not verify file signature",
                  file_name);
}


/// Read asset information from INFO_HDR
void readINFO_HDR(das_INFO_HDR *ihdr, const char *file_name) {
    char emsg[EMSG_LEN] = { 0 };
    sprintf(emsg, "Could not read INFO_HDR, potentially corrupt file or stream");

    dataRead(ihdr, sizeof(das_INFO_HDR), emsg, file_name);

    // Verify that the header signature is correct
    DAS_FROASSERT(ihdr->hdr_sig == DAS_INFO_HEADER_SIG,
                "Could not verify header signature for INFO_HDR",
                file_name);
}


/// Verify that all custom headers are skipped from reading
void skipMetaHeaders(const char *file_name) {
    das_META_HDR mhdr = { 0 };

    // Because sizeof(das_META_HDR) - sizeof(char*) == 16
    const size_t rsize = 16;
    dataRead(&mhdr, rsize,
             "Could not attempt to read meta header, potentially corrup file or stream",
             file_name);

    // Could not verify the signature, exit the subroutine
    if(mhdr.hdr_sig != DAS_META_HEADER_SIG) {
        __offset -= rsize;
        fseek(__sfile, __offset, SEEK_SET);
        return;
    }

    DAS_FROASSERT(mhdr.hdr_size + rsize != mhdr.data_size,
                "Could not get correct metadata header size",
                file_name);

    DAS_FROASSERT(mhdr.data_size + __offset >= __flen,
                "Corrupt metadata size",
                file_name);

    __offset += mhdr.data_size;
    fseek(__sfile, __offset, SEEK_SET);
}


/// Attempt to read metadata and return true if possible, if not return false
bool tryToReadMeta(das_META_HDR *meta, const char *file_name) {
    const size_t rsize = 16;
    dataRead(meta, rsize, 
             "Could not read META_HDR, potentially corrupt file", 
             file_name);
    
    if(meta->hdr_sig != DAS_META_HEADER_SIG) {
        __offset -= rsize;
        fseek(__sfile, __offset, SEEK_SET);
        return false;
    }

    DAS_FROASSERT(meta->hdr_size + rsize != meta->data_size,
                "Could not get correct metadata header size",
                file_name);

    // Allocate memory for metadata
    meta->data = (char*) malloc(meta->data_size * sizeof(char));
    
    // Copy metadata to its appropriate memory area
    // BUG: Whenever an error condition is triggered in dataRead() subroutine, 
    //      you can expect segmentation fault from trying to print NULL
    dataRead(meta->data, meta->data_size, "Could not get correct metadata size", file_name);
    meta->data[meta->data_size - 1] = 0;

    return true;
}


/// Read asset information from VERT_HDR
void readVERT_HDR(das_VERT_HDR *vhdr, const char *file_name) {
    dataRead(vhdr, sizeof(das_VERT_HDR), 
             "Could not read VERT_HDR, potentially corrupt file",
             file_name);

    DAS_FROASSERT(vhdr->hdr_sig == DAS_VERT_HEADER_SIG,
                  "Could not verify signature for VERT_HDR",
                  file_name);
}


/// Read information about one vertex element header type
void readVertAttr(das_VertAttribute *ahdr, uint64_t exsig, const char *file_name, uint64_t pos_size) {
    const size_t tsize = 17;
    dataRead(ahdr, tsize, 
            "Could not read generic vertex attribute header, potentially corrupt file",
            file_name);

    DAS_FROASSERT(ahdr->hdr_sig == exsig, 
                "Could not verify signature for VERT_HDR attribute",
                file_name);

    if(!pos_size) {
        DAS_FROASSERT(ahdr->hdr_size == ahdr->vert_c * ahdr->esize * sizeof(float) + tsize,
                    "Could not get correct vert attribute header size",
                    file_name);
    } else {
        DAS_FROASSERT(ahdr->hdr_size == ahdr->vert_c * pos_size + tsize,
                    "Could not get correct vert attribute header size",
                    file_name);
    }
}


/// Read asset information from INDX_HDR
void readINDX_HDR(das_AssetMode mode, das_INDX_HDR *ihdr, const char *file_name, bool read_indices) {
    const size_t isize = 16;
    dataRead(ihdr, isize,
             "Could not read INDX_HDR, potentially corrupt file",
             file_name);

    DAS_FROASSERT(ihdr->hdr_sig == DAS_INDX_HEADER_SIG,
                "Could not verify signature for INDX_HDR",
                file_name);

    if(read_indices) {
        uint32_t bfc = 0;
        bool is_tex = false;
        bool is_norm = false;
        switch(mode) {
            case DAS_ASSET_MODE_2D_UNMAPPED:
                bfc = 1;
                break;

            case DAS_ASSET_MODE_2D_TEXTURE_MAPPED:
                bfc = 2;
                is_tex = true;
                break;

            case __DAS_ASSET_MODE_3D_UNMAPPED_UNOR:
                bfc = 1;
                break;

            case DAS_ASSET_MODE_3D_UNMAPPED:
                bfc = 2;
                is_norm = true;
                break;

            case __DAS_ASSET_MODE_3D_TEXTURE_MAPPED_UNOR:
                bfc = 2;
                is_tex = true;
                break;

            case DAS_ASSET_MODE_3D_TEXTURE_MAPPED:
                bfc = 3;
                is_tex = true;
                is_norm = true;
                break;

            case DAS_ASSET_MODE_UNDEFINED:
                DAS_FROASSERT(NULL, "Invalid asset mode specified on reading INDX_HDR",
                           file_name);
                break;
        }

        DAS_FROASSERT(ihdr->hdr_size == ihdr->ind_c * bfc * sizeof(uint32_t) + isize,
                    "Could not get correct INDX_HDR size",
                    file_name);

        const size_t dsize = ihdr->ind_c * sizeof(uint32_t);

        // Allocate memory and read position indices
        ihdr->pinds = (uint32_t*) malloc(dsize);
        dataRead(ihdr->pinds, dsize,
                 "Could not read position indices, possibly corrupt file",
                 file_name);

        // If needed read texture indices
        if(is_tex) {
            ihdr->tinds = (uint32_t*) malloc(dsize);
            dataRead(ihdr->tinds, dsize,
                     "Could not read texture indices, possibly corrupt file",
                     file_name);
        }

        // If needed read vertex normal indices
        if(is_norm) {
            ihdr->ninds = (uint32_t*) malloc(dsize);
            dataRead(ihdr->ninds, dsize,
                     "Could not read vertex normal indices, possibly corrupt file",
                     file_name);
        }
    }
}
