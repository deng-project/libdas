/// DENG: dynamic engine - small but powerful 3D game engine
/// licence: Apache, see LICENCE file
/// file: das_runtime.c - DENG asset structure runtime handling functionality header file
/// author: Karl-Mihkel Ott


#define __DAS_RUNTIME_C
#include <data/das_runtime.h>


/// Attempt to create vertex normals for unnormalised vertices
/// This function works for 3D assets only
void das_MkAssetNormals(das_Asset *p_asset) {
    // Small error check to make sure that the indices count is divisible by 3
    if(p_asset->indices.n % 3) 
        RUN_ERR("__das_AssetNormalise()", "Asset contains invalid count of indices");

    // Create a new hashmap with bucket size of indices count
    Hashmap hm = { 0 }; 
    newHashmap(&hm, p_asset->indices.n);

    // Allocate maximum potential amount of memory for asset vertex normals and indices
    p_asset->vertices.v3d.mul.nn = 0;
    p_asset->vertices.v3d.mul.norm = (das_ObjNormalData*) calloc(p_asset->indices.n / 3, 
        sizeof(das_ObjNormalData));
    p_asset->indices.norm = (deng_ui32_t*) calloc(p_asset->indices.n, sizeof(deng_ui32_t));

    
    // Additionally allocate an index buffer for keeping hashmap values in
    deng_ui32_t *unique_ind_buf = (deng_ui32_t*) calloc(p_asset->indices.n, sizeof(deng_ui32_t));
    deng_ui32_t max_ind = 0;

    // For each 3 index read their corresponding vertices and calculate vertex normals
    for(size_t i = 0; i < p_asset->indices.n; i += 3) {
        // Find the first side vector of the triangle
        das_ObjPosData s1 = { 
            p_asset->vertices.v3d.mul.pos[i + 1].vert_x - p_asset->vertices.v3d.mul.pos[i].vert_x,
            p_asset->vertices.v3d.mul.pos[i + 1].vert_y - p_asset->vertices.v3d.mul.pos[i].vert_y,
            p_asset->vertices.v3d.mul.pos[i + 1].vert_z - p_asset->vertices.v3d.mul.pos[i].vert_z
        };

        // Find the second side vector of the triangle
        das_ObjPosData s2 = { 
            p_asset->vertices.v3d.mul.pos[i + 2].vert_x - p_asset->vertices.v3d.mul.pos[i].vert_x,
            p_asset->vertices.v3d.mul.pos[i + 2].vert_y - p_asset->vertices.v3d.mul.pos[i].vert_y,
            p_asset->vertices.v3d.mul.pos[i + 2].vert_z - p_asset->vertices.v3d.mul.pos[i].vert_z
        };

        // Calculate the face normals by finding the cross product between two side vectors
        das_ObjNormalData tmp_norm = { 0 };
        tmp_norm.nor_x = s1.vert_y * s2.vert_z - s1.vert_z * s2.vert_y;
        tmp_norm.nor_y = s1.vert_z * s2.vert_x - s1.vert_x * s2.vert_z;
        tmp_norm.nor_z = s1.vert_x * s2.vert_y - s1.vert_y * s2.vert_x;

        // Normalise the vertex normals
        deng_vec_t tmp_len = sqrtf(tmp_norm.nor_x * tmp_norm.nor_x + 
            tmp_norm.nor_y * tmp_norm.nor_y + tmp_norm.nor_z * tmp_norm.nor_z);

        tmp_norm.nor_x /= tmp_len;
        tmp_norm.nor_y /= tmp_len;
        tmp_norm.nor_z /= tmp_len;

        deng_ui32_t *p_ind = findValue(&hm, &tmp_norm, sizeof(das_ObjNormalData));

        // If the current vertex normals do not exist in the map, add them
        if(!p_ind) {
            // Push the new unique value to the map
            // Vertex normal is the key and index + 1 is the value
            unique_ind_buf[max_ind] = max_ind + 1;            
            p_asset->vertices.v3d.mul.norm[p_asset->vertices.v3d.mul.nn] = tmp_norm;
            pushToHashmap(&hm, (void*) (p_asset->vertices.v3d.mul.norm + p_asset->vertices.v3d.mul.nn), 
                sizeof(das_ObjNormalData), (void*) (unique_ind_buf + max_ind));

            p_asset->indices.norm[i] = max_ind;
            p_asset->indices.norm[i + 1] = max_ind;
            p_asset->indices.norm[i + 2] = max_ind;
            max_ind++;
            p_asset->vertices.v3d.mul.nn++;
        }

        // Calculated vertex normal instance is not unique, assign the retrieved value
        else p_asset->indices.norm[i] = *p_ind - 1;
    }

    // Free all temporarly allocated resources
    free(unique_ind_buf);
    destroyHashmap(&hm);
}


/* 
 * Create a new 3D asset structure instance from given vertices' data
 * This function returns a pointer to a stack allocated asset instance, which
 * means that the return asset values are overwritten in each function call
 */
das_Asset *das_CreateNewAsset (
    das_ObjPosData *pd,
    size_t pd_c,
    das_ObjTextureData *td,
    size_t td_c,
    das_ObjNormalData *nd,
    size_t nd_c,
    das_IndicesDynamic *idyn,
    size_t ind_c
) {
    static das_Asset out_asset = { 0 };

    // Check for the asset mode based on the input vertices count
    if(!td_c && !nd_c)
        out_asset.asset_mode = __DAS_ASSET_MODE_3D_UNMAPPED_UNOR;
    else if(!td_c && nd_c) 
        out_asset.asset_mode = DAS_ASSET_MODE_3D_UNMAPPED;
    else if(td_c && !nd_c)
        out_asset.asset_mode = __DAS_ASSET_MODE_3D_TEXTURE_MAPPED_UNOR;
    else out_asset.asset_mode = DAS_ASSET_MODE_3D_TEXTURE_MAPPED;

    // Set all destination asset vertices
    out_asset.vertices.v3d.mul.pos = pd;
    out_asset.vertices.v3d.mul.pn = pd_c;
    out_asset.vertices.v3d.mul.tex = td;
    out_asset.vertices.v3d.mul.tn = td_c;
    out_asset.vertices.v3d.mul.norm = nd;
    out_asset.vertices.v3d.mul.nn = nd_c;

    // Set all destination asset indices
    out_asset.indices = *idyn;
    out_asset.diffuse = DAS_DEFAULT_DIFFUSE_COLOR;
    out_asset.ambient = DAS_DEFAULT_AMBIENT_COLOR;
    out_asset.specular = DAS_DEFAULT_SPECULAR_COLOR;
    out_asset.uuid = uuid_Generate();

    return &out_asset;
}


void das_MergeUM2DIndexBuffers(das_Asset *p_asset) {
    // Set GL vertex and index values
    p_asset->vertices.v2d.mer.uvert = p_asset->vertices.v2d.mul.pos;
    p_asset->vertices.v2d.mer.n = p_asset->vertices.v2d.mul.pn;
}


void das_MergeTM2DIndexBuffers(das_Asset *p_asset) {
    das_GL2DVertex *vert = (das_GL2DVertex*) calloc(p_asset->indices.n, sizeof(das_GL2DVertex));
    deng_idx_t *idxs = (deng_idx_t*) calloc(p_asset->indices.n, sizeof(deng_idx_t));
    size_t n = 0;

    Hashmap map;
    newHashmap(&map, p_asset->indices.n * 2);

    // For each index, check if the corresponding previous vertex instance is in hashmap
    for(size_t i = 0; i < p_asset->indices.n; i++) {
        das_GL2DVertex v = { .pos = p_asset->vertices.v2d.mul.pos[p_asset->indices.pos[i]], 
                             .tex = p_asset->vertices.v2d.mul.tex[p_asset->indices.tex[i]]};

        // Retrieve index and check if none were found
        void *idx;
        if(!(idx = findValue(&map, &v, sizeof(das_GL2DVertex)))) {
            vert[n] = v;
            idxs[i] = n;
            n++;
            pushToHashmap(&map, vert + n - 1, sizeof(das_GL2DVertex), (void*) (n + 1));
        }
        else idxs[i] = (deng_ui32_t)((deng_ui64_t) idx - 1);
    }

    // Free previous data
    free(p_asset->vertices.v2d.mul.pos);
    free(p_asset->vertices.v2d.mul.tex);
    free(p_asset->indices.pos);
    free(p_asset->indices.tex);

    // Set the correct vertex and indices pointers
    p_asset->vertices.v2d.mer.vert = vert;
    p_asset->indices.gl = idxs;
}


void das_MergeUM3DIndexBuffers(das_Asset *p_asset) {
    das_GL3DVertexUnmapped *uvert = (das_GL3DVertexUnmapped*) calloc(p_asset->indices.n, sizeof(das_GL3DVertexUnmapped));
    deng_idx_t *idxs = (deng_idx_t*) calloc(p_asset->indices.n, sizeof(deng_idx_t));
    size_t n = 0;

    Hashmap map;
    newHashmap(&map, p_asset->indices.n * 2);
 
    // For each index, check if the corresponding previous vertex instance is in hashmap
    for(size_t i = 0; i < p_asset->indices.n; i++) {
        das_GL3DVertexUnmapped v = { .pos = p_asset->vertices.v3d.mul.pos[p_asset->indices.pos[i]], 
                                     .norm = p_asset->vertices.v3d.mul.norm[p_asset->indices.norm[i]]};

        // Retrieve index and check if none were found
        void *idx;
        if(!(idx = findValue(&map, &v, sizeof(v)))) {
            uvert[n] = v;
            idxs[i] = n;
            n++;
            pushToHashmap(&map, uvert + n - 1, sizeof(das_GL3DVertexUnmapped), (void*) (n + 1));
        }
        else idxs[i] = (deng_ui32_t)((deng_ui64_t) idx - 1);
    }

    // Free previous data
    free(p_asset->vertices.v3d.mul.pos);
    free(p_asset->vertices.v3d.mul.norm);
    free(p_asset->indices.pos);
    free(p_asset->indices.tex);

    // Set the correct vertex and indices pointers
    p_asset->vertices.v3d.mer.uvert = uvert;
    p_asset->indices.gl = idxs;
}


void das_MergeTM3DIndexBuffers(das_Asset *p_asset) {
    das_GL3DVertex *vert = (das_GL3DVertex*) calloc(p_asset->indices.n, sizeof(das_GL3DVertex));
    deng_idx_t *idxs = (deng_idx_t*) calloc(p_asset->indices.n, sizeof(deng_idx_t));
    size_t n = 0;

    Hashmap map;
    newHashmap(&map, p_asset->indices.n * 2);

    // For each index, check if the corresponding previous vertex instance is in hashmap
    for(size_t i = 0; i < p_asset->indices.n; i++) {
        das_GL3DVertex v = { .pos = p_asset->vertices.v3d.mul.pos[p_asset->indices.pos[i]], 
                             .tex = p_asset->vertices.v3d.mul.tex[p_asset->indices.tex[i]],
                             .norm = p_asset->vertices.v3d.mul.norm[p_asset->indices.norm[i]] };

        // Retrieve index and check if none were found
        void *idx = NULL;
        if(!(idx = findValue(&map, &v, sizeof(das_GL3DVertex)))) {
            vert[n] = v;
            idxs[i] = n;
            pushToHashmap(&map, vert + n, sizeof(das_GL3DVertex), (void*) (n + 1));
            n++;
        }
        else idxs[i] = (deng_ui32_t)((deng_ui64_t) idx - 1);
    }

    // Free previous data
    free(p_asset->vertices.v3d.mul.pos);
    free(p_asset->vertices.v3d.mul.tex);
    free(p_asset->vertices.v3d.mul.norm);
    free(p_asset->indices.pos);
    free(p_asset->indices.tex);
    free(p_asset->indices.norm);

    destroyHashmap(&map);
    // Set the correct vertex and indices pointers
    p_asset->vertices.v3d.mer.vert = vert;
    p_asset->vertices.v3d.mer.n = n;
    p_asset->indices.gl = idxs;
}



/// Merge multiple index buffers into single index buffer and duplicate associated vertex attributes if needed
void das_MergeIndexBuffers(das_Asset *p_asset) {

    // Check the asset mode and merge index buffers accordingly
    switch(p_asset->asset_mode) {
    case DAS_ASSET_MODE_2D_UNMAPPED:
        das_MergeUM2DIndexBuffers(p_asset);
        break;

    case DAS_ASSET_MODE_2D_TEXTURE_MAPPED:
        das_MergeTM2DIndexBuffers(p_asset);
        break;

    case DAS_ASSET_MODE_3D_UNMAPPED:
        das_MergeUM3DIndexBuffers(p_asset);
        break;

    case DAS_ASSET_MODE_3D_TEXTURE_MAPPED:
        das_MergeTM3DIndexBuffers(p_asset);
        break;

    default:
        break;
    }
}
