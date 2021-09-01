/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: wobj.c - wavefront obj parser source file
/// author: Karl-Mihkel Ott


// stl includes
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <uuid.h>
#include <assets.h>
#include <wobj.h>


static void logData(das_WavefrontObjEntity *pEnt);
static void quadTriangulate(das_WavefrontObjEntity *pEnt);


/// Log wavefront obj data in obj like format to stdout
static void logData(das_WavefrontObjEntity *pEnt) {
    for(uint64_t i = 0; i < pEnt->data.vert_data.v3d.mul.pn; i++) {
        printf("v %f %f %f\n", pEnt->data.vert_data.v3d.mul.pos[i].vert_x, pEnt->data.vert_data.v3d.mul.pos[i].vert_y,
               pEnt->data.vert_data.v3d.mul.pos[i].vert_z);
    }

    for(uint64_t i = 0; i < pEnt->data.vert_data.v3d.mul.tn; i++) {
        printf("vt %f %f\n", pEnt->data.vert_data.v3d.mul.tex[i].tex_x, 
               pEnt->data.vert_data.v3d.mul.tex[i].tex_y);
    }

    for(uint64_t i = 0; i < pEnt->data.vert_data.v3d.mul.nn; i++) {
        printf("vn %f %f %f\n", pEnt->data.vert_data.v3d.mul.norm[i].nor_x, 
               pEnt->data.vert_data.v3d.mul.norm[i].nor_y, pEnt->data.vert_data.v3d.mul.norm[i].nor_z);
    }

    for(uint64_t i = 0; i < pEnt->data.ind_data.n / 3; i++) {
        printf("f %u/%u/%u %u/%u/%u %u/%u/%u\n", pEnt->data.ind_data.pos[i * 3], pEnt->data.ind_data.tex[i * 3], pEnt->data.ind_data.norm[i * 3],
               pEnt->data.ind_data.pos[i * 3 + 1], pEnt->data.ind_data.tex[i * 3 + 1], pEnt->data.ind_data.norm[i * 3 + 1],
               pEnt->data.ind_data.pos[i * 3 + 2], pEnt->data.ind_data.tex[i * 3 + 2], pEnt->data.ind_data.norm[i * 3 + 2]);
    }
}


/// Triangulate a quad faced mesh
static void quadTriangulate(das_WavefrontObjEntity *pEnt) {
    das_IndicesDynamic inds = { 0 };
    inds.n = pEnt->data.ind_data.n / 2 * 3;
    inds.pos = (uint32_t*) malloc(inds.n * sizeof(uint32_t));
    inds.tex = (uint32_t*) malloc(inds.n * sizeof(uint32_t));
    inds.norm = (uint32_t*) malloc(inds.n * sizeof(uint32_t));

    // For each quad in array, create triangle out of it
    for(uint64_t i = 0; i < pEnt->data.ind_data.n / 4; i++) {
        uint32_t *q_pos = pEnt->data.ind_data.pos + i * 4;
        uint32_t *q_tex = pEnt->data.ind_data.tex + i * 4;
        uint32_t *q_norm = pEnt->data.ind_data.norm + i * 4;

        uint32_t *t_pos = inds.pos + i * 6;
        uint32_t *t_tex = inds.tex + i * 6;
        uint32_t *t_norm = inds.norm + i * 6;

        t_pos[0] = q_pos[0], t_pos[1] = q_pos[1], t_pos[2] = q_pos[2], t_pos[3] = q_pos[2], t_pos[4] = q_pos[3], t_pos[5] = q_pos[0];
        t_tex[0] = q_tex[0], t_tex[1] = q_tex[1], t_tex[2] = q_tex[2], t_tex[3] = q_tex[2], t_tex[4] = q_tex[3], t_tex[5] = q_tex[0];
        t_norm[0] = q_norm[0], t_norm[1] = q_norm[1], t_norm[2] = q_norm[2], t_norm[3] = q_norm[2], t_norm[4] = q_norm[3], t_norm[5] = q_norm[0];
    }

    // Free initial quad memory areas
    free(pEnt->data.ind_data.pos);
    free(pEnt->data.ind_data.tex);
    free(pEnt->data.ind_data.norm);

    pEnt->data.ind_data = inds;
}


int main(int argv, char *argc[]) {
    if(argv < 2) {
        printf("Please give a wavefront obj file name as a first argument\n");
        exit(1);
    }

    das_WavefrontObjEntity *ents = NULL;
    uint32_t ent_c = 0;
    das_ParseWavefrontOBJ(&ents, &ent_c, argc[1]);

    // Find the first object with valid vertices
    uint32_t eind = UINT32_MAX;
    for(uint32_t i = 0; i < ent_c; i++) {
        if(ents[i].data.vert_data.v3d.mul.pn) {
            eind = i;
            break;
        }
    }
    
    if(eind == UINT32_MAX) return 2;
    if(ents[eind].data.fv == 4) {
        printf("Triangulating quads\n");
        quadTriangulate(ents + eind);
        printf("Triangulation done\n");
    }

    logData(ents + eind);

    free(ents[eind].data.vert_data.v3d.mul.pos);
    free(ents[eind].data.vert_data.v3d.mul.tex);
    free(ents[eind].data.vert_data.v3d.mul.norm);

    free(ents[eind].data.ind_data.pos);
    free(ents[eind].data.ind_data.tex);
    free(ents[eind].data.ind_data.norm);
    return 0;
}
