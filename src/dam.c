/// DENG: dynamic engine - small but powerful 3D game engine
/// licence: Apache, see LICENCE file
/// file: dam.h - deng asset manager header file
/// author: Karl-Mihkel Ott

#define DAS_FILE
#define __DAM_C
#include <dam.h>


/*
 * Read information about the asset
 * Information is following:
 *  - UUID of the asset
 *  - Date and time of the creation
 *  - Type of vertices
 *  - Total count of unique vertices
 *  - Total count of drawn vertices
 */
void __dam_ListAsset(const char *file_name) {
    das_INFO_HDR inf_hdr = { 0 };
    das_VERT_HDR vert_hdr = { 0 };
    das_INDX_HDR indx_hdr = { 0 };

    // Specific vertex header reading
    das_VPOS_HDR vpos_hdr = { 0 };
    das_VTEX_HDR vtex_hdr = { 0 };
    das_VNOR_HDR vnor_hdr = { 0 };

    __das_ReadAssetFile(file_name);
    __das_ReadINFO_HDR(&inf_hdr, file_name);
 
    char date[MAX_STR] = { 0 };
    time_t t = (time_t) inf_hdr.time_st;
    struct tm *tmp = localtime(&t);
    strftime(date, MAX_STR, "%F %H:%S");

    // Read metadata
    __das_ReadMeta(file_name);

    // Create padded uuid strings
    char pad_uuid[37] = { 0 };
    strncpy(pad_uuid, inf_hdr.uuid, 36);

    // Read the initial part of the VERT_HDR
    __das_ReadVERT_HDR(&vert_hdr, file_name);
    __das_ReadGenVertHdr(&pos_hdr, (char*) "VPOS_HDR", li.file_name);
    __das_IncrementOffset(vpos_hdr.vert_c * sizeof(das_ObjPosData));

    // Increment reading offset for file buffer
    switch(inf_hdr.asset_type) {
    case DAS_ASSET_MODE_3D_UNMAPPED:
        // Read vertex normals header
        __das_ReadGenVertHdr(&vnor_hdr, (char*) __DAS_NORM_POSITION_HEADER_NAME, li.file_name);
        __das_IncrementOffset(vnor_hdr.vert_c * sizeof(das_ObjNormalData));
        break;

    case DAS_ASSET_MODE_3D_TEXTURE_MAPPED:
        // Read texture vertex header
        __das_ReadGenVertHdr(&vtex_hdr, (char*) __DAS_TEX_POSITION_HEADER_NAME, li.file_name);
        __das_IncrementOffset(vtex_hdr.vert_c * sizeof(das_ObjTextureData));

        // Read vertex normals header
        __das_ReadGenVertHdr(&vnor_hdr, (char*) __DAS_NORM_POSITION_HEADER_NAME, li.file_name);
        __das_IncrementOffset(vnor_hdr.vert_c * sizeof(das_ObjNormalData));
        break;

    case __DAS_ASSET_MODE_3D_TEXTURE_MAPPED_UNOR:
        // Read texture data
        __das_ReadGenVertHdr(&vtex_hdr, (char*) __DAS_NORM_POSITION_HEADER_NAME, li.file_name);
        __das_IncrementOffset(vtex_hdr.vert_c * sizeof(das_ObjTextureData));
        break;

    default:
        break;
    }

    // Read indices
    __das_ReadINDX_HDR(&indx_hdr, li.file_name);

    cm_FormatTimeFromEpoch (
        DATE_FORMAT_YMD_SP_HYPHEN,
        TIME_FORMAT_24_HOUR_SP_COLON,
        inf_hdr.time_st,
        &date,
        &time
    );

    printf("Magic number: 0x%16lx\n", inf_hdr.magic_number);
    printf("UUID: %s\n", pad_uuid);
    printf("Date and time of creation: %s(UTC)\n", date);

    switch(inf_hdr.asset_type) {
    case DAS_ASSET_MODE_3D_UNMAPPED:
        printf("Vertex type: unmapped vertices\n");
        printf("Unique position vertices count: %d\n", vpos_hdr.vert_c);
        printf("Unique vertex normals count: %d\n", vnor_hdr.vert_c);
        break;

    case __DAS_ASSET_MODE_3D_UNMAPPED_UNOR:
        printf("Vertex type: unnormalised unmapped vertices\n");
        printf("Unique position vertices count: %d\n", vpos_hdr.vert_c);
        break;

    case DAS_ASSET_MODE_3D_TEXTURE_MAPPED:
        printf("Vertex type: textured vertices\n");
        printf("Unique position vertices count: %d\n", vpos_hdr.vert_c);
        printf("Unique texture vertices count: %d\n", vtex_hdr.vert_c);
        printf("Unique vertex normals count: %d\n", vnor_hdr.vert_c);
        break;
    
    case __DAS_ASSET_MODE_3D_TEXTURE_MAPPED_UNOR:
        printf("Vertex type: unnormalised textured vertices\n");
        printf("Unique texture vertices count: %d\n", vtex_hdr.vert_c);
        break;

    default:
        break;
    }

    printf("Total vertices count: %d\n", indx_hdr.ind_c);

    // Print metadata
    size_t meta_c = 0;
    char **meta_data = das_GetMeta(&meta_c);

    if(meta_c && li.meta_enable) {
        printf("Additional metadata: \n");
        for(size_t i = 0; i < meta_c; i++) {
            // Check if newline is present in metadata
            if(meta_data[i][strlen(meta_data[i]) - 1] != '\n')
                printf("%s\n", meta_data[i]);
            else printf("%s", meta_data[i]);
        }
    }
    __das_ReadCleanup();
}


/// Create a valid asset instance with sorted indices
void __dam_AssembleAsset(dam_AssemblyInfo *p_info) {
    char *dst_ext = cm_ExtractFileExtName(p_info->dst_file);
    char *model_ext = cm_ExtractFileExtName(p_info->src_file);
    char ext_file[128] = {0};
    
    // Check if file extension needs to be added
    if(!dst_ext || (dst_ext && strcmp(dst_ext, "das")))
        sprintf(ext_file, "%s.das", p_info->dst_file);
    else
        sprintf(ext_file, "%s", p_info->dst_file);


    das_Asset asset = {0};
    asset.asset_mode = p_info->vert_mode;
    // Check for supported 3D model formats and parse them
    if(model_ext && !strcmp(model_ext, "obj")) {
        size_t entity_c = 0;
        das_WavefrontObjEntity *entities;
        
        // Parse the Wavefront OBJ file into entities
        das_ParseWavefrontOBJ(&entities, &entity_c, p_info->src_file);

        // Prompt the user about the object he would like to use
        das_WavefrontObjEntityWritePrompt(&asset, entities,
            entity_c, p_info->src_file);

        // Perform final cleanup of the entities
        das_WavefrontObjDestroyEntities(entities, entity_c);
    }

    else __DAM_INVALID_MODEL_FORMAT(model_ext);


    // Check if the asset file should contain any meta data
    if(p_info->meta_c) {
        char **meta = NULL;
        size_t meta_c = 0;
        __dam_ReadMetaData(&meta, &meta_c, p_info->meta_files, p_info->meta_c); 
        das_StaticAssemble(&asset, ext_file, meta, meta_c);

        // Perform metadata cleanup
        for(size_t i = 0; i < meta_c; i++)
            free(meta[i]);
        free(meta);
    }

    else das_StaticAssemble(&asset, ext_file, NULL, 0);

    // Free position vertices and indices
    free(asset.vertices.v3d.mul.pos);
    free(asset.indices.pos);

    // Asset mode specific cleanup
    switch(asset.asset_mode) {
    case DAS_ASSET_MODE_3D_UNMAPPED:
        // Free vertex normals
        free(asset.vertices.v3d.mul.norm);
        free(asset.indices.norm);
        break;

    case DAS_ASSET_MODE_3D_TEXTURE_MAPPED:
        free(asset.vertices.v3d.mul.tex);
        free(asset.indices.tex);
        free(asset.vertices.v3d.mul.norm);
        free(asset.indices.norm);
        break;

    case __DAS_ASSET_MODE_3D_TEXTURE_MAPPED_UNOR:
        free(asset.vertices.v3d.mul.tex);
        free(asset.indices.tex);
        break;

    default:
        break;
    }
}


/// Parse user required actions
dam_Options parseInput(int argc, char *argv[]) {
    dam_Options opt = { 0 };
    opt.das_file = argv[argc - 1];

    // DAM has two cases where options are not regarded, when showing helptext and when showing asset information
    if((argc == 2 && (!strcmp(argv[1], "-h")) || !strcmp(argv[1], "--help")) || argc < 2) {
        printf("%s", __help_text);
        exit(0);
    }
    else if(argc == 2) {
        opt.show_finfo = true;
        return opt;
    }

    // When those two cases are avoided, dam assumes that a new asset needs to be assembled
    // Start iterating through options
    bool wait_input = false;
    bool wait_mode = false;
    uint32_t mflag_c = 0;
    for(int i = 1; i < argc - 1; i++) {
        if(wait_input) {
            opt.das_file = argv[i];
            wait_input = false;
        }

        else if(wait_mode) {
            // Check if vertices mode is invalid
            if(strcmp(argv[i], "vuu") && strcmp(argv[i], "vun") && 
               strcmp(argv[i], "vmu") && strcmp(argv[i], "vmn")) {
                printf("Invalid vertices mode specified\n"\
                       "Vertices mode must be any of the following: vuu (vertex unmapped unnormalised),"\
                       "vun (vertex unmapped normalised), vmu (vertex texture mapped unnormalised) or vmn (vertex texture mapped normalised)");
                exit(0);
            }

            opt.vmode = *((uint32_t*) argv[i]);
            wait_mode = false;
        }

        // Parse command arguments normally
        else {
            if(!strcmp(argv[i], "-i") || !strcmp(argv[i], "--input"))
                wait_input = true;
            else if(!strcmp(argv[i], "-v") || !strcmp(argv[i], "--vert"))
                wait_mode = true;
            else if(!strcmp(argv[i], "-nm") || !strcmp(argv[i], "--no-meta")) {
                opt.is_meta = false;
                meta_c++;
            }
            else if(!strcmp(argv[i], "-m") || !strcmp(argv[i], "--meta")) {
                opt.is_meta = true;
                meta_c++;
            }

            // Check if meta flag has been called more than once
            if(meta_c > 1) {
                printf("Too many meta flags, there must be only one!");
                exit(0);
            }
        }
    }


    return opt;
}



int main(int argc, char *argv[]) {
    // Parse user input into input actions
    dam_Options opt = parseInput(argc, argv);

    // Check if info should be listed or assembly performed
    if(opt.show_finfo) listAsset();
    else {
        if(opt.meta) {
            static char meta[META_LEN] = { 0 };
            fgets(meta, META_LEN, stdin);
            opt.meta = meta;
        }

        assembleAsset(opt);
    }
    return 0;
}
