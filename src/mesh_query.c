/// DENG: dynamic engine - small but powerful 3D game engine
/// licence: Apache, see LICENCE file
/// file: mesh_mod.c - a program that attempts to implement O(1) access to following information
//                     * Given a triangle, what are the threee adjacent triangles
//                     * Given an edge, which two triangles share it
//                     * Given a vertex, which faces share it
//                     * Given a vertex, which edges share it
/// author: Karl-Mihkel Ott


// stl includes
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <hashmap.h>
#include <uuid.h>
#include <assets.h>
#include <das_loader.h>

/************************/
/****** Input data ******/
/************************/

// Commandline argument:
// As a commandline parameter, a file name to a das binary file must be used

// stdin:
// There is a single line input that has one value.
// The first value is either a t for triangle, e for an edge or v for vertex, 
// followed by a number that states its index


/*************************/
/****** Output data ******/
/*************************/

// If triangle usage is specified, return three integers which represent the indices
// to its adjacent triangles
// If edge usage is specified, return indicies to two triangles that share it
// If vertex usage is specified, return two lines:
//    - The first line starts with f continued by the indices of faces that share it
//    - The second line starts with e continued by the indicies of edges that share it

/********************************/
/******** Crude examples ********/
/********************************/

// Input for `mesh_mod file.das`:
//   t 12
// Output:
//   13 11 10

// Input for `mesh_mod file2.das`:
//   e 4
// Output:
//   3 1

// Input for `mesh_mod file3.das`:
//   v 19
// Output:
//   f 8 1 2 ...
//   e 2 9 1 ...


/// Function definitions
static void triangulate(das_Asset *asset, das_MeshTriangle **pTri);


/// Triangulate vertices and find all neighbouring triangles
static void triangulate(das_Asset *asset, das_MeshTriangle **pTri) {
    *pTri = (das_MeshTriangle*) calloc(asset->indices.n / 3, sizeof(das_MeshTriangle));

    das_PosData *pos = asset->vertices.v3d.mul.pos;
    das_TextureData *tex = asset->vertices.v3d.mul.tex;
    das_NormalData *norm = asset->vertices.v3d.mul.norm;

    // Edges are needed temporarely in order to find adjacent triangles  
    das_Edge *edges = (das_Edge*) calloc(asset->indices.n, sizeof(das_Edge));
    size_t edge_c = 0;

    // Create a hashmap that has any edge position pointers as a key and edge structure
    // as a value
    Hashmap emap = { 0 };
    newHashmap(&emap, asset->indices.n * sizeof(void*));

    // Set all triangle structures
    for(uint64_t i = 0; i < asset->indices.n / 3; i++) {
        uint32_t *posi = asset->indices.pos + i * 3;
        uint32_t *texi = asset->indices.tex + i * 3;
        uint32_t *normi = asset->indices.norm + i * 3;

        // Set the triangle pointer for a vertex 
        pos[*posi].tri = (void*) ((*pTri) + i);
        pos[(*posi) + 1].tri = (void*) ((*pTri) + i + 1);
        pos[(*posi) + 2].tri = (void*) ((*pTri) + i + 2);

        (*pTri)[i].pos[0] = pos + *posi;
        (*pTri)[i].pos[1] = pos + *(posi + 1);
        (*pTri)[i].pos[2] = pos + *(posi + 2);

        (*pTri)[i].tex[0] = tex + *texi;
        (*pTri)[i].tex[1] = tex + *(texi + 1);
        (*pTri)[i].tex[2] = tex + *(texi + 2);

        (*pTri)[i].nor[0] = norm + *normi;
        (*pTri)[i].nor[1] = norm + *(normi + 1);
        (*pTri)[i].nor[2] = norm + *(normi + 2);

        // For each edge in current (*pTri)angle, check if it exists in the hashmap
        for(uint16_t j = 0; j < 3; j++) {
            das_PosData *key_pos[2] = { (*pTri)[i].pos[(j + 1) % 3], (*pTri)[i].pos[j] };
            das_PosData *npos[2] = { (*pTri)[i].pos[j], (*pTri)[i].pos[(j + 1) % 3] };
            das_Edge *re = NULL;

            // Edge is already declared set its second adjacent (*pTri)angle accordingly
            if((re = (das_Edge*) findValue(&emap, &key_pos, 2 * sizeof(das_PosData*)))) {
                re->adj_tri[1] = (*pTri) + i;
                (*pTri)[i].nbr[(*pTri)[i].nnbr] = re->adj_tri[0];
                (*pTri)[i].nnbr++;

                re->adj_tri[0]->nbr[re->adj_tri[0]->nnbr] = (*pTri) + i;
                re->adj_tri[0]->nnbr++;
            } 
            
            // Edge is not declared so push it to hashmap
            else {
                memcpy(edges[edge_c].pos, npos, 2 * sizeof(das_PosData*));
                edges[edge_c].adj_tri[0] = (*pTri) + i;
                pushToHashmap(&emap, edges + edge_c, 2 * sizeof(das_PosData*), (void*) (edges + edge_c));
                edge_c++;
            }
        }
    }

    free(edges);

    // Print out each triangles adjacent triangles
    for(uint64_t i = 0; i < asset->indices.n / 3; i++) {
        printf("Adjacent triangles of triangle %lu are %lu %lu %lu\n", i,
               (*pTri)[i].nbr[0] - (*pTri), (*pTri)[i].nbr[1] - (*pTri), (*pTri)[i].nbr[2] - (*pTri));
    }
}


int main(int argv, char *argc[]) {
    if(argv < 2) {
        perror("Please specify a file name to load in the program\n");
        exit(1);
    }

    // Attempt to load mesh data from das binary, assuming that the assetdata is kept in DAS_ASSET_MODE_TEXTURE_MAPPED_3D format
    das_Asset asset;
    das_LoadAsset(&asset, DAS_ASSET_MODE_UNDEFINED, DAS_DEFAULT_DIFFUSE_COLOR, NULL,  NULL, argc[1]);
    printf("Total triangle count in mesh %s is %lu\n", asset.uuid.bytes, asset.indices.n / 3);
    das_MeshTriangle *tri;
    triangulate(&asset, &tri);

    free(tri);

    // Assume implicitly that the asset mode is DAS_ASSET_MODE_3D_TEXTURE_MAPPED
    free(asset.vertices.v3d.mul.pos);
    free(asset.vertices.v3d.mul.tex);
    free(asset.vertices.v3d.mul.norm);
    free(asset.indices.pos);
    free(asset.indices.tex);
    free(asset.indices.norm);
    return 0;
}
