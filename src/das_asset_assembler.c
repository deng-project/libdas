/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: das_asset_assembler.c - das binary writer source code
/// author: Karl-Mihkel Ott


#define DAS_FILE
#define __DAS_ASSET_ASSEMBLER_C
#include <das_asset_assembler.h>


/// Assemble an asset file from given das_Asset instance
void das_CreateAssetFile(das_Asset *asset, const char *file_name, char *meta) {
    openFileStreamWO(file_name);
    writeFILE_HDR(file_name);
    writeINFO_HDR(asset, file_name);
    if(meta && *meta != 0) writeMETA_HDR(meta, file_name);
    writeVERT_HDR(asset, file_name);

    // Check the asset mode and decide which vertex attributes to write
    switch(asset->asset_mode) {
        case DAS_ASSET_MODE_2D_UNMAPPED:
            writeGenericVertAttrHDR(asset->vertices.v2d.mul.pos, asset->vertices.v2d.mul.pn,
                                    2, DAS_VPOS_HEADER_SIG, 
                                    "Could not write 2D asset vertex position attributes", 
                                    file_name);
            break;

        case DAS_ASSET_MODE_2D_TEXTURE_MAPPED:
            writeGenericVertAttrHDR(asset->vertices.v2d.mul.pos, asset->vertices.v2d.mul.pn,
                                    2, DAS_VPOS_HEADER_SIG, 
                                    "Could not write 2D asset vertex position attributes", 
                                    file_name);

            writeGenericVertAttrHDR(asset->vertices.v2d.mul.tex, asset->vertices.v2d.mul.tn,
                                    2, DAS_VTEX_HEADER_SIG, 
                                    "Could not write 2D asset vertex texture attributes", 
                                    file_name);
            break;
        
        case __DAS_ASSET_MODE_3D_UNMAPPED_UNOR:
            writeGenericVertAttrHDR(asset->vertices.v3d.mul.pos, asset->vertices.v3d.mul.pn,
                                    3, DAS_VPOS_HEADER_SIG, 
                                    "Could not write 3D asset vertex position attributes", 
                                    file_name);
            break;

        case DAS_ASSET_MODE_3D_UNMAPPED:
            writeGenericVertAttrHDR(asset->vertices.v3d.mul.pos, asset->vertices.v3d.mul.pn,
                                    3, DAS_VPOS_HEADER_SIG, 
                                    "Could not write 3D asset vertex position attributes", 
                                    file_name);

            writeGenericVertAttrHDR(asset->vertices.v3d.mul.norm, asset->vertices.v3d.mul.nn,
                                    3, DAS_VNOR_HEADER_SIG, 
                                    "Could not write 3D asset vertex normal attributes", 
                                    file_name);
            break;

        case __DAS_ASSET_MODE_3D_TEXTURE_MAPPED_UNOR:
            writeGenericVertAttrHDR(asset->vertices.v3d.mul.pos, asset->vertices.v3d.mul.pn,
                                    3, DAS_VPOS_HEADER_SIG, 
                                    "Could not write 3D asset vertex position attributes", 
                                    file_name);

            writeGenericVertAttrHDR(asset->vertices.v3d.mul.tex, asset->vertices.v3d.mul.tn,
                                    2, DAS_VTEX_HEADER_SIG, 
                                    "Could not write 3D asset vertex texture attributes", 
                                    file_name);
            break;
        
        case DAS_ASSET_MODE_3D_TEXTURE_MAPPED:
            writeGenericVertAttrHDR(asset->vertices.v3d.mul.pos, asset->vertices.v3d.mul.pn,
                                    3, DAS_VPOS_HEADER_SIG, 
                                    "Could not write 3D asset vertex position attributes", 
                                    file_name);

            writeGenericVertAttrHDR(asset->vertices.v3d.mul.tex, asset->vertices.v3d.mul.tn,
                                    2, DAS_VTEX_HEADER_SIG, 
                                    "Could not write 3D asset vertex texture attributes", 
                                    file_name);

            writeGenericVertAttrHDR(asset->vertices.v3d.mul.norm, asset->vertices.v3d.mul.nn,
                                    3, DAS_VNOR_HEADER_SIG, 
                                    "Could not write 3D asset vertex normal attributes", 
                                    file_name);
            break;

        default: break;
    }

    writeINDX_HDR(asset, file_name);
    closeFileStream();
}


/***********************************/
/**** Header assembly functions ****/
/***********************************/


/// Write the initial FILE_HDR data into file
static void writeFILE_HDR(const char *file_name) {
    das_FILE_HDR fhdr = { 0 };
    fhdr.hdr_sig = DAS_FILE_HEADER_SIG;
    dataWrite(&fhdr, sizeof(das_FILE_HDR), "Could not write FILE_HDR", file_name);
}


/// Write INFO_HDR data into file
static void writeINFO_HDR(const das_Asset *asset, const char *file_name) {
    das_INFO_HDR ihdr = { 0 };

    ihdr.hdr_sig = DAS_INFO_HEADER_SIG;
    ihdr.hdr_size = sizeof(das_INFO_HDR);
    ihdr.asset_type = (das_AssetMode) asset->asset_mode;
    // reserved for future compression algorithms
    ihdr.cmpr = 0;
    ihdr.time_st = time(NULL);
    strcpy(ihdr.uuid, asset->uuid.bytes);

    dataWrite(&ihdr, sizeof(das_INFO_HDR), 
              "Could not write INFO_HDR",
              file_name);
}


/// Write metadata into its corresponding header
static void writeMETA_HDR(char *meta, const char *file_name) {
    das_META_HDR mhdr = { 0 };

    const size_t ndsize = 16;
    mhdr.hdr_sig = DAS_META_HEADER_SIG;
    mhdr.data_size = strlen(meta) + 1;
    mhdr.hdr_size = ndsize + mhdr.data_size;

    dataWrite(&mhdr, ndsize, 
              "Could not write initial META_HDR data",
              file_name);

    dataWrite(meta, mhdr.data_size,
              "Could not write metadata",
              file_name);
}


/// Write the initial VERT_HDR data to the file
static void writeVERT_HDR(const das_Asset *asset, const char *file_name) {
    das_VERT_HDR vhdr = { 0 };
    vhdr.hdr_sig = DAS_VERT_HEADER_SIG;
    vhdr.hdr_size = sizeof(das_VERT_HDR);

    // Find out the header size according to assets asset mode
    const size_t gnd_size = 17;
    switch(asset->asset_mode) {
        case DAS_ASSET_MODE_2D_UNMAPPED:
            vhdr.hdr_size += gnd_size;
            vhdr.hdr_size += asset->vertices.v2d.mul.pn * sizeof(das_PosData2D);
            break;

        case DAS_ASSET_MODE_2D_TEXTURE_MAPPED:
            vhdr.hdr_size += 2 * gnd_size;
            vhdr.hdr_size += asset->vertices.v2d.mul.pn * sizeof(das_PosData2D);
            vhdr.hdr_size += asset->vertices.v2d.mul.tn * sizeof(das_TextureData);
            break;

        case __DAS_ASSET_MODE_3D_UNMAPPED_UNOR:
            vhdr.hdr_size += gnd_size;
            vhdr.hdr_size += asset->vertices.v3d.mul.pn * sizeof(das_PosData);
            break;

        case DAS_ASSET_MODE_3D_UNMAPPED:
            vhdr.hdr_size += 2 * gnd_size;
            vhdr.hdr_size += asset->vertices.v3d.mul.pn * sizeof(das_PosData);
            vhdr.hdr_size += asset->vertices.v3d.mul.nn * sizeof(das_NormalData);
            break;

        case __DAS_ASSET_MODE_3D_TEXTURE_MAPPED_UNOR:
            vhdr.hdr_size += 2 * gnd_size;
            vhdr.hdr_size += asset->vertices.v3d.mul.pn * sizeof(das_PosData);
            vhdr.hdr_size += asset->vertices.v3d.mul.tn * sizeof(das_TextureData);
            break;

        case DAS_ASSET_MODE_3D_TEXTURE_MAPPED:
            vhdr.hdr_size += 3 * gnd_size;
            vhdr.hdr_size += asset->vertices.v3d.mul.pn * sizeof(das_PosData);
            vhdr.hdr_size += asset->vertices.v3d.mul.tn * sizeof(das_TextureData);
            vhdr.hdr_size += asset->vertices.v3d.mul.nn * sizeof(das_NormalData);
            break;

        default:
            DAS_FWOASSERT(NULL, "Invalid asset mode specified", file_name);
            break;
    }

    dataWrite(&vhdr, sizeof(das_VERT_HDR), "Could not write VERT_HDR to file", file_name);
}


/// Write generic vertex attribute header and its data
static void writeGenericVertAttrHDR(void *vt, uint32_t vc, uint32_t esize, uint64_t sig,
                                    const char *emsg, const char *file_name) {
    das_VertAttribute vthdr = { 0 };
    const uint32_t ndsize = 17;
    const uint64_t dsize = vc * esize * sizeof(float);
    vthdr.hdr_sig = sig;
    vthdr.esize = esize;
    vthdr.vert_c = vc;
    vthdr.hdr_size = ndsize + dsize;

    // Write header without its data
    dataWrite(&vthdr, ndsize, emsg, file_name);

    // Write header data
    dataWrite(vt, dsize, emsg, file_name);
}


/// Write INDX_HDR and all indicies data associated with it
static void writeINDX_HDR(const das_Asset *asset, const char *file_name) {
    const uint32_t ndsize = 16;
    das_INDX_HDR ihdr = { 0 };
    ihdr.hdr_sig = DAS_INDX_HEADER_SIG;
    ihdr.ind_c = asset->indices.n;

    // Check the asset mode and thus find the header size
    switch(asset->asset_mode) {
        case DAS_ASSET_MODE_2D_UNMAPPED:
        case __DAS_ASSET_MODE_3D_UNMAPPED_UNOR:
            ihdr.hdr_size = ndsize + asset->indices.n * sizeof(uint32_t);
            break;

        case DAS_ASSET_MODE_2D_TEXTURE_MAPPED:
        case __DAS_ASSET_MODE_3D_TEXTURE_MAPPED_UNOR:
        case DAS_ASSET_MODE_3D_UNMAPPED:
            ihdr.hdr_size = ndsize + 2 * asset->indices.n * sizeof(uint32_t);
            break;

        case DAS_ASSET_MODE_3D_TEXTURE_MAPPED:
            ihdr.hdr_size = ndsize + 3 * asset->indices.n * sizeof(uint32_t);
            break;

        default:
            break;
    }


    // Write header data without actual indices
    dataWrite(&ihdr, ndsize, "Could not write initial INDX_HDR data", file_name);

    // Check the asset type and write correct indices data to file
    switch(asset->asset_mode) {
        case DAS_ASSET_MODE_2D_UNMAPPED:
            dataWrite(asset->indices.pos, ihdr.ind_c * sizeof(uint32_t), 
                      "Could not write 2D asset position indices",
                      file_name);
            break;

        case DAS_ASSET_MODE_2D_TEXTURE_MAPPED:
            dataWrite(asset->indices.pos, ihdr.ind_c * sizeof(uint32_t),
                      "Could not write 2D asset position indices",
                      file_name);

            dataWrite(asset->indices.tex, ihdr.ind_c * sizeof(uint32_t),
                      "Could not write 2D asset texture indices",
                      file_name);
            break;

        case __DAS_ASSET_MODE_3D_UNMAPPED_UNOR:
            dataWrite(asset->indices.pos, ihdr.ind_c * sizeof(uint32_t), 
                      "Could not write 3D asset position indices",
                      file_name);
            break;

        case DAS_ASSET_MODE_3D_UNMAPPED:
            dataWrite(asset->indices.pos, ihdr.ind_c * sizeof(uint32_t), 
                      "Could not write 3D asset position indices",
                      file_name);

            dataWrite(asset->indices.norm, ihdr.ind_c * sizeof(uint32_t), 
                      "Could not write 3D asset vertex normal indices",
                      file_name);
            break;

        case __DAS_ASSET_MODE_3D_TEXTURE_MAPPED_UNOR:
            dataWrite(asset->indices.pos, ihdr.ind_c * sizeof(uint32_t), 
                      "Could not write 3D asset position indices",
                      file_name);

            dataWrite(asset->indices.tex, ihdr.ind_c * sizeof(uint32_t), 
                      "Could not write 3D asset texture indices",
                      file_name);
            break;

        case DAS_ASSET_MODE_3D_TEXTURE_MAPPED:
            dataWrite(asset->indices.pos, ihdr.ind_c * sizeof(uint32_t), 
                      "Could not write 3D asset position indices",
                      file_name);

            dataWrite(asset->indices.tex, ihdr.ind_c * sizeof(uint32_t), 
                      "Could not write 3D asset texture indices",
                      file_name);

            dataWrite(asset->indices.norm, ihdr.ind_c * sizeof(uint32_t), 
                      "Could not write 3D asset vertex normal indices",
                      file_name);
            break;

        default:
            DAS_FWOASSERT(NULL, "Invalid asset mode specified for writing INDX_HDR", file_name);
            break;
    }
}
