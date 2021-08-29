/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: wobj_tokens.h - wavefront obj token handler header
/// author: Karl-Mihkel Ott


#ifndef __WOBJ_TOKENS_H
#define __WOBJ_TOKENS_H


#ifdef __WOBJ_TOKENS_C
    #include <stdlib.h>
    #include <stdint.h>
    #include <string.h>   

    #include <hashmap.h>
#endif


typedef enum __das_WavefrontObjSpecType {
    DAS_WAVEFRONT_OBJ_SPEC_TYPE_NONE            = -1,
    DAS_WAVEFRONT_OBJ_SPEC_TYPE_OBJ_DECL        = 0,
    DAS_WAVEFRONT_OBJ_SPEC_TYPE_GROUP_DECL      = 1,
    DAS_WAVEFRONT_OBJ_SPEC_TYPE_VERT_DECL       = 2,
    DAS_WAVEFRONT_OBJ_SPEC_TYPE_VERT_TEX_DECL   = 3,
    DAS_WAVEFRONT_OBJ_SPEC_TYPE_VERT_NORM_DECL  = 4,
    DAS_WAVEFRONT_OBJ_SPEC_TYPE_VERT_PARAM_DECL = 5,
    DAS_WAVEFRONT_OBJ_SPEC_TYPE_FACE_DECL       = 6,
    DAS_WAVEFRONT_OBJ_SPEC_TYPE_POL_LINE_DECL   = 7,
    DAS_WAVEFRONT_OBJ_SPEC_TYPE_MTL_INCL        = 8,
    DAS_WAVEFRONT_OBJ_SPEC_TYPE_MTL_USE         = 9,
    DAS_WAVEFRONT_OBJ_SPEC_TYPE_SHADING_SPEC    = 10
} __das_WavefrontObjSpecType;


/// Structure for defining all statements with their keywords and valid object types that could be used
typedef struct __das_WavefrontObjStatement {
    char *keyword;
    int32_t min_obj_c;
    int32_t max_obj_c;
    __das_WavefrontObjSpecType spec_type;
} __das_WavefrontObjStatement;


#ifdef __WOBJ_TOKENS_C
    /// Map for containing data about all possible statements
    static Hashmap __statement_map;

    /// Array with all statement specifications
    static __das_WavefrontObjStatement __statements[] = {
        { "o", 0, 1, DAS_WAVEFRONT_OBJ_SPEC_TYPE_OBJ_DECL },
        { "g", 0, 1, DAS_WAVEFRONT_OBJ_SPEC_TYPE_GROUP_DECL },
        { "v", 3, 4, DAS_WAVEFRONT_OBJ_SPEC_TYPE_VERT_DECL },
        { "vt", 1, 3, DAS_WAVEFRONT_OBJ_SPEC_TYPE_VERT_TEX_DECL },
        { "vn", 3, 3, DAS_WAVEFRONT_OBJ_SPEC_TYPE_VERT_NORM_DECL },
        { "vp", 1, 3, DAS_WAVEFRONT_OBJ_SPEC_TYPE_VERT_PARAM_DECL },
        { "f", 3, 4, DAS_WAVEFRONT_OBJ_SPEC_TYPE_FACE_DECL },
        { "l", 1, UINT32_MAX, DAS_WAVEFRONT_OBJ_SPEC_TYPE_POL_LINE_DECL },
        { "mtllib", 1, 1, DAS_WAVEFRONT_OBJ_SPEC_TYPE_MTL_INCL },
        { "usemtl", 1, 1, DAS_WAVEFRONT_OBJ_SPEC_TYPE_MTL_USE },
        { "s", 1, 1, DAS_WAVEFRONT_OBJ_SPEC_TYPE_SHADING_SPEC }
    };
#endif


/// Create a hashmap out of all statements that can be accessed with keywords
void das_WavefrontObjTokenise();


/// Free all memory that was used by tokens
void das_WavefrontObjUntokenise();


/// Retrieve token statement info from the hashmap using the keyword's value
__das_WavefrontObjStatement *das_GetTokenInfo(char *id);

#endif
