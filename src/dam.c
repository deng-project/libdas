/// DENG: dynamic engine - small but powerful 3D game engine
/// licence: Apache, see LICENCE file
/// file: dam.h - deng asset manager header file
/// author: Karl-Mihkel Ott

#define DAS_FILE
#define __DAM_C
#include <dam.h>


/// Find the source file format
static AssetSrcFileFormat findSrcFormat(char *file_name) {
    AssetSrcFileFormat srcf = FORMAT_UNKNOWN;
    size_t flen = strlen(file_name);

    // Find the last period from the file name
    char *per = NULL;
    for(int32_t i = (int32_t) flen - 1; i >= 0; i--) {
        if(file_name[i] == '.') {
            per = file_name + i;
            break;
        }
    }

    if(!per || per - file_name + 1 > flen - 1) {
        fprintf(stderr, "Unknown file format for file %s\n", file_name);
        exit(1);
    }
    per++;

    // Check the file format for the given file
    if(!strcmp(per, "stl"))
        srcf = STANDARD_TRIANGLE_LANG;
    else if(!strcmp(per, "obj"))
        srcf = WAVEFRONT_OBJ;
    else if(!strcmp(per, "gltf"))
        srcf = GLTF;
    else if(!strcmp(per, "flx"))
        srcf = FLUXBOX;
    else {
        fprintf(stderr, "Unknown file format %s for file %s\n", per, file_name);
        exit(1);
    }

    return srcf;
}


/// Parse user required actions
static dam_Options parseInput(int argc, char *argv[]) {
    // Check if any arguments are even given
    if(argc == 1) {
        printf("DENG asset manager v %u.%u.%u\n", MAJOR, MINOR, REVISION);
        printf("%s", __to_help);
        exit(0);
    }
    dam_Options opt = { 0 };
    opt.das_file = argv[argc - 1];
    opt.meta = NULL;

    // DAM has two cases where options are not regarded, when showing helptext and when showing asset information
    if((argc == 2 && !strcmp(argv[1], "-h")) || !strcmp(argv[1], "--help") || argc < 2) {
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
            opt.input = argv[i];
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
                opt.meta = NULL;
                mflag_c++;
            }
            else if(!strcmp(argv[i], "-m") || !strcmp(argv[i], "--meta"))
                opt.meta = (char*) 1;

            // Check if meta flag has been called more than once
            if(mflag_c > 1) {
                printf("Too many meta flags, there must be only one!");
                exit(0);
            }
        }
    }


    return opt;
}


/// Select appropriate group for usage and prompt the user about possibly multiple groups
static uint32_t selectWObjGroup(uint32_t *nz_objs, uint32_t nz_oc, uint32_t *nz_grps, uint32_t nz_gc, char *gt,
                                das_WavefrontObjEntity *ents) {

    // In case there is only one entity select that and quit
    if(nz_oc + nz_gc == 1) {
        if(nz_oc == 1) {
            *gt = 'o';
            return *nz_objs;
        } else {
            *gt = 'g';
            return *nz_grps;
        }
    }

    // Notify the user about multiple objects and groups
    printf("This file contains ");
    if(nz_oc) printf("%u objects", nz_oc);
    if(nz_gc) printf(" and %u groups", nz_gc);
    printf("\nHowever DAS file can contain data for only single object\n"\
           "Please select the object / group you wish to use\n");


    // List all groups
    printf("Groups\n"\
           "======\n");
    for(uint32_t i = 0; i < nz_gc; i++)
        printf("g%u) %s\n", i + 1, ents[nz_grps[i]].data.name);

    // List all objects
    printf("Objects\n"\
           "=======\n");
    for(uint32_t i = 0; i < nz_oc; i++)
        printf("o%u) %s\n", i + 1, ents[nz_objs[i]].data.name);


    char ibuf[BUF_LEN] = { 0 };

    // Wait for correct user input
    do {
        printf("> ");
        fflush(stdout);
        scanf("%256s", ibuf);

        uint32_t out = UINT32_MAX;
        if((ibuf[0] == 'o' || ibuf[1] == 'g') && (out = (uint32_t) atoi(ibuf + 1))) {
            *gt = ibuf[0];
            if((*gt == 'o' && out <= nz_oc) || (*gt == 'g' && out <= nz_gc))
                return out;
        }

        printf("Invalid options, try again\n");
    } while(true);
}


/// Clear all data from given asset
static void clearAssetData(das_Asset *asset) {
    // Check the asset mode to verify, which memory areas to free
    switch(asset->asset_mode) {
        case DAS_ASSET_MODE_2D_UNMAPPED:
            free(asset->vertices.v2d.mul.pos);
            free(asset->indices.pos);
            break;

        case DAS_ASSET_MODE_2D_TEXTURE_MAPPED:
            free(asset->vertices.v2d.mul.pos);
            free(asset->vertices.v2d.mul.tex);
            free(asset->indices.pos);
            free(asset->indices.tex);
            break;

        case __DAS_ASSET_MODE_3D_UNMAPPED_UNOR:
            free(asset->vertices.v3d.mul.pos);
            free(asset->indices.pos);
            break;

        case DAS_ASSET_MODE_3D_UNMAPPED:
            free(asset->vertices.v3d.mul.pos);
            free(asset->vertices.v3d.mul.norm);
            free(asset->indices.pos);
            free(asset->indices.norm);
            break;

        case __DAS_ASSET_MODE_3D_TEXTURE_MAPPED_UNOR:
            free(asset->vertices.v3d.mul.pos);
            free(asset->vertices.v3d.mul.tex);
            free(asset->indices.pos);
            free(asset->indices.tex);
            break;

        case DAS_ASSET_MODE_3D_TEXTURE_MAPPED:
            free(asset->vertices.v3d.mul.pos);
            free(asset->vertices.v3d.mul.tex);
            free(asset->vertices.v3d.mul.norm);

            free(asset->indices.pos);
            free(asset->indices.tex);
            free(asset->indices.norm);
            break;

        default:
            break;
    }
}


/*
 * Read information about the asset
 * Information is following:
 *  - UUID of the asset
 *  - Date and time of the creation
 *  - Type of vertices
 *  - Total count of unique vertices
 *  - Total count of drawn vertices
 */
static void listAssetData(const char *file_name) {
    das_FILE_HDR fhdr = { 0 };
    das_INFO_HDR inf_hdr = { 0 };
    das_META_HDR meta_hdr = { 0 };
    das_VERT_HDR vert_hdr = { 0 };
    das_INDX_HDR indx_hdr = { 0 };

    // Specific vertex header reading
    das_VertAttribute vpos_hdr = { 0 };
    das_VertAttribute vtex_hdr = { 0 };
    das_VertAttribute vnor_hdr = { 0 };

    openFileStreamRO(file_name);
    readFILE_HDR(&fhdr, file_name);
    readINFO_HDR(&inf_hdr, file_name);

    char date[MAX_STR] = { 0 };
    time_t t = (time_t) inf_hdr.time_st;
    struct tm *tmpt = localtime(&t);
    strftime(date, MAX_STR, "%F %H:%S", tmpt);

    // Read metadata
    bool is_meta = tryToReadMeta(&meta_hdr, file_name);

    // Read the initial part of the VERT_HDR
    readVERT_HDR(&vert_hdr, file_name);

    // Read position vertex header and skip its vertices
    readVertAttr(&vpos_hdr, DAS_VPOS_HEADER_SIG, file_name);
    skipStreamRO(vpos_hdr.vert_c * sizeof(das_PosData),
                 "Could not skip position vertices", file_name);

    // Increment reading offset for file buffer
    switch(inf_hdr.asset_type) {
    case DAS_ASSET_MODE_2D_TEXTURE_MAPPED:
        break;

    case DAS_ASSET_MODE_3D_UNMAPPED:
        readVertAttr(&vnor_hdr, DAS_VNOR_HEADER_SIG, file_name);
        skipStreamRO(vnor_hdr.vert_c * sizeof(das_NormalData), 
                     "Could not skip vertex normals", file_name);
        break;

    case DAS_ASSET_MODE_3D_TEXTURE_MAPPED:
        readVertAttr(&vtex_hdr, DAS_VTEX_HEADER_SIG, file_name);
        skipStreamRO(vtex_hdr.vert_c * sizeof(das_TextureData),
                     "Could not skip texture vertices", file_name);
        
        readVertAttr(&vnor_hdr, DAS_VNOR_HEADER_SIG, file_name);
        skipStreamRO(vnor_hdr.vert_c * sizeof(das_NormalData), 
                     "Could not skip vertex normals", file_name);
        break;

    case __DAS_ASSET_MODE_3D_TEXTURE_MAPPED_UNOR:
        readVertAttr(&vtex_hdr, DAS_VTEX_HEADER_SIG, file_name);
        skipStreamRO(vtex_hdr.vert_c * sizeof(das_TextureData),
                     "Could not skip texture vertices", file_name);
        break;

    default:
        break;
    }

    printf("File: %s\n", file_name);
    printf("UUID: %s\n", inf_hdr.uuid);
    printf("Date and time of creation: %s(UTC)\n", date);

    switch(inf_hdr.asset_type) {
    case DAS_ASSET_MODE_3D_UNMAPPED:
        printf("Asset type: unmapped vertices\n");
        printf("Unique position vertices count: %d\n", vpos_hdr.vert_c);
        printf("Unique vertex normals count: %d\n", vnor_hdr.vert_c);
        break;

    case __DAS_ASSET_MODE_3D_UNMAPPED_UNOR:
        printf("Asset type: unnormalised unmapped vertices\n");
        printf("Unique position vertices count: %d\n", vpos_hdr.vert_c);
        break;

    case DAS_ASSET_MODE_3D_TEXTURE_MAPPED:
        printf("Asset type: textured vertices\n");
        printf("Unique position vertices count: %d\n", vpos_hdr.vert_c);
        printf("Unique texture vertices count: %d\n", vtex_hdr.vert_c);
        printf("Unique vertex normals count: %d\n", vnor_hdr.vert_c);
        break;
    
    case __DAS_ASSET_MODE_3D_TEXTURE_MAPPED_UNOR:
        printf("Asset type: unnormalised textured vertices\n");
        printf("Unique texture vertices count: %d\n", vtex_hdr.vert_c);
        break;

    default:
        break;
    }

    printf("Total indices count: %d\n", indx_hdr.ind_c);

    // Check if metadata is available and display it
    if(is_meta) {
        printf("Metadata: %s\n", meta_hdr.data);
        free(meta_hdr.data);
    }

    closeFileStream();
}


/// Create an asset from obj file 
/// NOTE: Contains some boilerplate code for dam.c
static void mkAssetFromWObj(das_Asset *asset, das_WavefrontObjEntity *ents, uint32_t ent_c) {
    if(!ent_c) return;
    uint32_t *nz_objs = (uint32_t*) malloc(ent_c * sizeof(uint32_t));
    uint32_t *nz_grps = (uint32_t*) malloc(ent_c * sizeof(uint32_t));
    uint32_t nz_oc = 0;
    uint32_t nz_gc = 0;

    // Find all non-zero objects and grous
    for(uint32_t i = 0; i < ent_c; i++) {
        // Check if no data is present in obj entity and thus ignore this entity
        // NOTE: no data is present if no position vertices of any kind are available. 
        if(!ents[i].data.vert_data.v3d.mul.pn && !ents[i].data.vert_data.v2d.mul.pn)
            continue;

        switch(ents->type) {
            case DAS_ENTITY_TYPE_OBJECT:
                nz_objs[nz_oc] = i;
                nz_oc++;
                break;

            case DAS_ENTITY_TYPE_GROUP:
                nz_grps[nz_gc] = i;
                nz_gc++;
                break;

            default:
                break;
        }
    }

    /// LOGGING
    for(uint32_t i = 0; i < nz_oc; i++)
        printf("Object with vertices: %s\n", ents[nz_objs[i]].data.name);
    for(uint32_t i = 0; i < nz_gc; i++)
        printf("Object with vertices: %s\n", ents[nz_grps[i]].data.name);

    // In case there are no objects or groups with appropriate vertices, quit
    if(!(nz_oc + nz_gc)) {
        free(nz_objs);
        free(nz_grps);
        fprintf(stderr, "Invalid wavefront OBJ file, no vertices available");
        exit(-1);
    }

    // Check how many groups and objects were found and thus decide whose data to use
    char etype = 0x00;
    uint32_t eind = selectWObjGroup(nz_objs, nz_oc, nz_grps, nz_gc, &etype, ents);
    asset->vertices = ents[eind].data.vert_data;
    asset->indices = ents[eind].data.ind_data;

    // For test purposes assume that 3D vmn mode is used
    asset->asset_mode = DAS_ASSET_MODE_3D_TEXTURE_MAPPED;
    asset->uuid = uuid_Generate();

    // Clean all other entities data
    for(uint32_t i = 0; i < ent_c; i++) {
        if(i != eind) {
            free(ents[i].data.vert_data.v3d.mul.pos);
            free(ents[i].data.vert_data.v3d.mul.tex);
            free(ents[i].data.vert_data.v3d.mul.norm);

            free(ents[i].data.ind_data.pos);
            free(ents[i].data.ind_data.tex);
            free(ents[i].data.ind_data.norm);
        }
    }

    free(nz_objs);
    free(nz_grps);
}



int main(int argc, char *argv[]) {
    // Parse user input into input actions
    dam_Options opt = parseInput(argc, argv);

    // Check if info should be listed or assembly performed
    if(opt.show_finfo) listAssetData(opt.das_file);
    else {
        char meta[META_LEN] = { 0 };
        if(opt.meta) {
            fgets(meta, META_LEN, stdin);
            opt.meta = meta;
        }

        // Check the asset file format
        das_Asset asset = { 0 };
        AssetSrcFileFormat srcf = findSrcFormat(opt.input);
        switch(srcf) {
            case WAVEFRONT_OBJ:
                {
                    das_WavefrontObjEntity *ents = NULL;
                    uint32_t ent_c = 0;
                    das_ParseWavefrontOBJ(&ents, &ent_c, opt.input);
                    mkAssetFromWObj(&asset, ents, ent_c);
                    if(ent_c) free(ents);
                }
                break;

            case STANDARD_TRIANGLE_LANG:
                perror("STL files are not yet supported in DAM :(\n");
                exit(1);
                break;

            case GLTF:
                perror("GLTF files are not yet supported in DAM :(\n");
                exit(1);
                break;

            case FLUXBOX:
                perror("FLX file are not yet supported in DAM :(\n");
                exit(1);
                break;

            default:
                break;
        }

        das_CreateAssetFile(&asset, opt.das_file, opt.meta);
        clearAssetData(&asset);
    }
    return 0;
}
