/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: wobj.h - wavefront obj parser header
/// author: Karl-Mihkel Ott


#ifndef __WOBJ_H
#define __WOBJ_H


#ifdef __cplusplus
extern "C" {
#endif

#ifdef __WOBJ_C
    #include <stdlib.h>
    #include <stdio.h>
    #include <stdint.h>
    #include <stddef.h>
    #include <stdbool.h>
    #include <string.h>
    #include <float.h>
    #include <limits.h>

    #define BUFFER_CAP(N, T) N * sizeof(T) 
    #include <uuid.h>
    #include <assets.h>
    #include <hashmap.h>
    #include <das_loader.h>

    /// Syntax error handling macros for Wavefront OBJ files
    #define WAVEFRONT_SYNTAX_ERROR(line, desc)          fprintf(stderr, "Wavefront OBJ syntax error on line %ld: %s\n", line, desc), \
                                                        exit(EXIT_FAILURE)

    #define WAVEFRONT_EMPTY_OBJECT(line)                fprintf(stderr, "Wavefront OBJ error, empty object on line %ld\n", line), \
                                                        exit(EXIT_FAILURE)

    #define WAVEFRONT_EMPTY_GROUP(line)                 fprintf(stderr, "Wavefront OBJ error, empty group specified on line %ld\n", line), \
                                                        exit(EXIT_FAILURE)

    #define TOO_MANY_ARGS(line)                         fprintf(stderr, "Wavefront OBJ error, too many arguments line %ld\n", line), \
                                                        exit(EXIT_FAILURE)
    
    #define NOT_ENOUGH_ARGS(line)                       fprintf(stderr, "Wavefront OBJ error, not enough arguments on line %ld\n", line), \
                                                        exit(EXIT_FAILURE)

    #define TOO_LONG_WORD(line)                         fprintf(stderr, "Wavefront OBJ error, too long word %d\n", line), \
                                                        exit(EXIT_FAILURE)

    #define MEM_ERR(err_msg)                            fprintf(stderr, "Memory allocation error: %s\n", err_msg), \
                                                        exit(EXIT_FAILURE)


    /**************************/
    /******* Tokenising *******/
    /**************************/


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


    static void tokenise();
    static void untokenise();
    static __das_WavefrontObjStatement *getTokenInfo(char *kword);
#endif

#define DEFAULT_MEM_CAP                             128
#define MAX_WORD_SIZE                               512

/// Type specifier for entities used in OBJ files
typedef enum das_WavefrontObjEntityType {
    DAS_ENTITY_TYPE_GROUP       = 0,
    DAS_ENTITY_TYPE_OBJECT      = 1,
    DAS_ENTITY_TYPE_FIRST       = DAS_ENTITY_TYPE_GROUP,
    DAS_ENTITY_TYPE_LAST        = DAS_ENTITY_TYPE_OBJECT,
} das_WavefrontObjEntityType;


/// Struct for entity(groups and objects) positions and names that are used in Wavefront OBJ files
typedef struct das_WavefrontObjEntityData {
    char name[MAX_WORD_SIZE];
    char *start_ptr;
    das_IndicesDynamic ind_data;
    uint32_t ind_cap;

    das_VertDynamic vert_data;
    uint32_t v_cap;
    uint32_t vt_cap;
    uint32_t vn_cap;
} das_WavefrontObjEntityData;


/// Main entity structure for Wavefront obj entities
typedef struct das_WavefrontObjEntity {
    das_WavefrontObjEntityData data;
    das_WavefrontObjEntityType type;
} das_WavefrontObjEntity;

#ifdef __WOBJ_C
    static char *__buffer = NULL;
    static uint64_t __buf_len = 0;

    
    /// This data structure is used in block parsing only
    typedef struct __das_IndexBlock {
        uint32_t pos;
        uint32_t tex;
        uint32_t norm;
    } __das_IndexBlock;

    
    
    /// Parse one line statement
    static void reallocCheck(void **p_data, uint32_t *p_cap, uint32_t n, uint32_t size, char *err_msg);
    static void preProcessFileData(const char *file_name);
    static __das_IndexBlock parseFace(char *face);
    static void copyFaceIndices(das_WavefrontObjEntity *p_ent, char **words, uint32_t word_c);
    static __das_WavefrontObjSpecType parseStatement(char **words, uint32_t word_c, uint64_t line);
    static void analyseStatement(char **line_words, uint32_t word_c,
                                       das_WavefrontObjEntity **p_entities, uint32_t *p_ent_cap,
                                       uint32_t *p_ent_c, uint64_t lc);
    static void extractBlocks(char *beg, uint32_t len, char ***p_words, uint32_t *p_word_c, 
                              uint32_t *p_word_cap, uint64_t max_word_len);
    static void newEntity(das_WavefrontObjEntity **p_entities, uint32_t *p_entity_cap, uint32_t *p_entity_c, 
                          das_WavefrontObjEntityType ent_type, char *name);
    static uint32_t findLineCount(char *str, uint32_t len);

#endif 



/// Parse all data in Wavefront OBJ file and write all information about vertices and indices to p_asset
void das_ParseWavefrontOBJ(das_WavefrontObjEntity **p_ents, uint32_t *p_ent_c, char *file_name);

#ifdef __cplusplus
}
#endif
#endif
