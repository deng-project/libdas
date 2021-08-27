/// DENG: dynamic engine - small but powerful 3D game engine
/// licence: Apache, see LICENCE file
/// file: das_runtime.h - DENG asset structure runtime handling functionality header file
/// author: Karl-Mihkel Ott


#ifndef __DAS_RUNTIME_H
#define __DAS_RUNTIME_H

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __DAS_RUNTIME_C
    #include <stdio.h>
    #include <stdlib.h>
    #include <math.h>

    #include <common/base_types.h>
    #include <common/uuid.h>
    #include <common/hashmap.h>
    #include <common/cerr_def.h>

    #include <data/assets.h>

    /// Asset mode specific index buffer merging algorithms
    void das_MergeUM2DIndexBuffers(das_Asset *p_asset);
    void das_MergeTM2DIndexBuffers(das_Asset *p_asset);
    void das_MergeUM3DIndexBuffers(das_Asset *p_asset);
    void das_MergeTM3DIndexBuffers(das_Asset *p_asset);
#endif


/*
 * Attempt to create vertex normals for unnormalised vertices
 * This function works for 3D assets only
 */
void das_MkAssetNormals(das_Asset *p_asset);


/* 
 * Create a new asset structure instance from given vertices' data
 * This function return a pointer to a stack allocated static asset instance, which
 * means that the return asset values are overwritten in each function call
 */
das_Asset *das_CreateNewAsset(das_ObjPosData *pd, size_t pd_c, das_ObjTextureData *td,
    size_t td_c, das_ObjNormalData *nd, size_t nd_c, das_IndicesDynamic *idyn, size_t ind_c);


/// Merge multiple index buffers into single index buffer and duplicate associated vertex attributes if needed
void das_MergeIndexBuffers(das_Asset *p_asset);


#ifdef __cplusplus
}
#endif

#endif
