#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <unordered_map>

#include <uuid.h>
#include <assets.h>
#include <das_asset_writer.h>
#include <wobj.h>

#define BUF_LEN     256

// static function declarations
static void clearAssetData(Asset &asset);
static uint32_t selectGroup(uint32_t *nz_objs, uint32_t nz_oc, uint32_t *nz_grps, uint32_t nz_gc, char *gt,
                            std::vector<libdas::WavefrontObjEntity> &ents);
static void mkAssetFromWOBJ(Asset &asset, std::vector<libdas::WavefrontObjEntity> &ents);


static void clearAssetData(Asset &asset) {
    // Check the asset mode to verify, which data to free
    switch(asset.asset_mode) {
        case DAS_ASSET_MODE_2D_UNMAPPED:
            free(asset.vertices.v2d.mul.pos);
            free(asset.indices.pos);
            break;

        case DAS_ASSET_MODE_2D_TEXTURE_MAPPED:
            free(asset.vertices.v2d.mul.pos);
            free(asset.vertices.v2d.mul.tex);
            free(asset.indices.pos);
            free(asset.indices.tex);
            break;

        case __DAS_ASSET_MODE_3D_UNMAPPED_UNOR:
            free(asset.vertices.v3d.mul.pos);
            free(asset.indices.pos);
            break;

        case DAS_ASSET_MODE_3D_UNMAPPED:
            free(asset.vertices.v3d.mul.pos);
            free(asset.vertices.v3d.mul.norm);
            free(asset.indices.pos);
            free(asset.indices.norm);
            break;

        case __DAS_ASSET_MODE_3D_TEXTURE_MAPPED_UNOR:
            free(asset.vertices.v3d.mul.pos);
            free(asset.vertices.v3d.mul.tex);
            free(asset.indices.pos);
            free(asset.indices.tex);
            break;

        case DAS_ASSET_MODE_3D_TEXTURE_MAPPED:
            free(asset.vertices.v3d.mul.pos);
            free(asset.vertices.v3d.mul.tex);
            free(asset.vertices.v3d.mul.norm);

            free(asset.indices.pos);
            free(asset.indices.tex);
            free(asset.indices.norm);
            break;

        default:
            break;
    }
}


/// Select appropriate group for usage and prompt the user about possibly multiple groups
static uint32_t selectGroup(uint32_t *nz_objs, uint32_t nz_oc, uint32_t *nz_grps, uint32_t nz_gc, char *gt, std::vector<libdas::WavefrontObjEntity> &ents) {

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


/// Create an asset from obj file 
/// NOTE: Contains some boilerplate code for dam.c
static void mkAssetFromWOBJ(Asset &asset, std::vector<libdas::WavefrontObjEntity> &ents) {
    if(!ents.size()) return;
    uint32_t *nz_objs = (uint32_t*) malloc(ents.size() * sizeof(uint32_t));
    uint32_t *nz_grps = (uint32_t*) malloc(ents.size() * sizeof(uint32_t));
    uint32_t nz_oc = 0;
    uint32_t nz_gc = 0;

    // Find all non-zero objects and grous
    for(uint32_t i = 0; i < static_cast<uint32_t>(ents.size()); i++) {
        // Check if no data is present in obj entity and thus ignore this entity
        // NOTE: no data is present if no position vertices of any kind are available. 
        if(!ents[i].data.vert_data.v3d.mul.pn && !ents[i].data.vert_data.v2d.mul.pn)
            continue;

        switch(ents[i].type) {
            case libdas::DAS_WAVEFRONT_ENTITY_TYPE_OBJECT:
                nz_objs[nz_oc] = i;
                nz_oc++;
                break;

            case libdas::DAS_WAVEFRONT_ENTITY_TYPE_GROUP:
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
        fprintf(stderr, "Invalid wavefront OBJ file, no vertices available\n");
        exit(-1);
    }

    // Check how many groups and objects were found and thus decide whose data to use
    char etype = 0x00;
    uint32_t eind = selectGroup(nz_objs, nz_oc, nz_grps, nz_gc, &etype, ents);
    asset.vertices = ents[eind].data.vert_data;
    asset.indices = ents[eind].data.ind_data;

    // For test purposes assume that 3D vmn mode is used
    asset.asset_mode = DAS_ASSET_MODE_3D_TEXTURE_MAPPED;
    libdas::id_generator idg;
    asset.uuid = idg.getUUID();

    // Clean all other entities data
    for(uint32_t i = 0; i < static_cast<uint32_t>(ents.size()); i++) {
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




int main(int argv, char *argc[]) {
    if(argv < 2) {
        fprintf(stderr, "Please provide file name as an argument\n");
        exit(-1);
    }

    std::vector<libdas::WavefrontObjEntity> ents;
    libdas::WavefrontObjParser parser = libdas::WavefrontObjParser();
    parser.parseWavefrontObj(ents);

    // For each object check its name
    //for(uint32_t i = 0; i < static_cast<uint32_t>(ents.size()); i++) {
        //if(ents[i].type == DAS_ENTITY_TYPE_GROUP)
            //printf("Group: %s\n", ents[i].data.name);
        //else if(ents[i].type == DAS_ENTITY_TYPE_OBJECT)
            //printf("Object: %s\n", ents[i].data.name);

        //printf("Position vertices count: %lu\n", ents[i].data.vert_data.v3d.mul.pn);
        //printf("Texture vertices count: %lu\n", ents[i].data.vert_data.v3d.mul.tn);
        //printf("Vertex normals count: %lu\n", ents[i].data.vert_data.v3d.mul.nn);
    //}

    Asset asset = { { 0 } };
    mkAssetFromWOBJ(asset, ents);

    // Attempt to write the current asset data into DAS binary
    libdas::AssetWriter writer(asset, "test.das", NULL);

    // Free asset data when possible
    clearAssetData(asset);
    return 0;
}
