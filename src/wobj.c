/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: wobj.c - wavefront obj parser source file
/// author: Karl-Mihkel Ott


#define DAS_FILE
#define __WOBJ_C
#include <wobj.h>


/// Parse all data in Wavefront OBJ file and write all information about vertices and indices to p_asset
void das_ParseWavefrontOBJ(das_WavefrontObjEntity **p_ents, uint32_t *p_ent_c, char *file_name) {
    tokenise();

    // Load object into memory
    openFileStreamRO(file_name);

    // Add some extra complexity by removing all comments
    preProcessFileData(file_name);

    uint32_t entity_cap = 8;
    *p_ent_c = 0;
    (*p_ents) = (das_WavefrontObjEntity*) calloc(entity_cap, sizeof(das_WavefrontObjEntity));

    // Set up the initial global entity
    newEntity(p_ents, &entity_cap, p_ent_c, DAS_ENTITY_TYPE_OBJECT, "Default");

    // Allocate memory for line words
    uint32_t word_cap = 8;
    uint32_t word_c = 0;

    // Allocate memory for line words
    char **swords = (char**) calloc(word_cap, sizeof(char*));
    for(uint32_t i = 0; i < word_cap; i++) 
        swords[i] = (char*) calloc(MAX_WORD_SIZE, sizeof(char));

    // Read the buffer line by line
    char *cur = __buffer;
    char *end = NULL;
    uint64_t lc = 1;
    while(cur < __buffer + __buf_len) {
        // Find the newline and if it does not exist set the end as eof
        end = strchr(cur, 0x0a);
        end = !end ? __buffer + __buf_len : end;

        // Find all the statements in the line
        extractBlocks(cur, end - cur, &swords, &word_c, &word_cap, MAX_WORD_SIZE);
        
        // If no blocks were found skip the iteration
        if(!word_c) {
            lc++;
            cur = end + 1;
            continue;
        }

        // Analyse the retrieved line
        analyseStatement(swords, word_c, p_ents, &entity_cap, p_ent_c, lc);
        cur = end + 1;

        // Reset all the words that were read
        for(uint32_t i = 0; i < word_c; i++)
            memset(swords[i], 0, MAX_WORD_SIZE * sizeof(char));
        
        lc++;
        word_c = 0;
    }

    // Clear all memory that was allocated for tokens
    untokenise();

    // Clean all the memory that was for line words
    for(uint32_t i = 0; i < word_cap; i++)
        free(swords[i]);
    free(swords);

    closeFileStream();
    free(__buffer);
}


/// Check if memory reallocations need to be done
static void reallocCheck(void **p_data, uint32_t *p_cap, uint32_t n, uint32_t size, char *err_msg) {
    // Reallocate if the capacity is smaller than the required amount of elements
    if(n >= (*p_cap)) {
        uint32_t old_cap = (*p_cap);
        (*p_cap) = n * 2;
        (*p_cap) = (*p_cap) < (old_cap << 1) ? old_cap << 1 : (*p_cap);
        void *tmp = realloc((*p_data), (*p_cap) * size);
        if(!tmp) MEM_ERR(err_msg);
        (*p_data) = tmp;
    }
}


static void preProcessFileData(const char *file_name) {
    __buf_len = getBufferLen();
    __buffer = (char*) malloc(__buf_len + 1);
    dataRead(__buffer, __buf_len, "Could not read wavefront obj data into memory", file_name);

    // Preprocessor stage 1: elliminate all comments
    char *beg, *lend = 0;
    while((beg = strchr(__buffer, (int) '#'))) {
        lend = strchr(beg, (int) 0x0a);
        lend = !lend ? __buffer + __buf_len : lend;
        memset(beg, (int) 0x20, lend - beg);
    }
}


/// Parse a single face block
static __das_IndexBlock parseFace(char *face) {
    // UINT32_MAX basically means that this index is going to be ignored
    __das_IndexBlock ind_block = { UINT32_MAX };
    uint32_t face_len = strlen(face);
    
    // Separator and number buffer declaration
    char buf[32] = { 0 };
    char *sep[2] = { 0 };
    
    // Find all separators
    for(uint32_t i = 1, j = 0; i < face_len && j < 3; i++) {
        if(face[i] == '/') {
            sep[j] = face + i;
            j++;
        }
    }

    // Check if the first separator exists and if it does then copy value till separator
    if(sep[0]) {
        strncpy(buf, face, sep[0] - face);
        ind_block.pos = (uint32_t) atoi(buf);
        memset(buf, 0, 32);
    }
    else {
        ind_block.pos = (uint32_t) atoi(face);
        return ind_block;
    }
    
    // Check if the texture index exists
    char *end = !sep[1] ? face + face_len : sep[1];
    if(end - sep[0] > 1) {
        strncpy(buf, sep[0] + 1, end - sep[0]);
        ind_block.tex = (uint32_t) atoi(buf);
        memset(buf, 0, 32);
    }

    // Check if second separator exists and has space between the third one
    end = face + face_len;
    if(sep[1] && end - sep[1] > 1) {
        strncpy(buf, sep[1] + 1, end - sep[1]);
        ind_block.norm = (uint32_t) atoi(buf);
        memset(buf, 0, 32);
    }

    return ind_block;
} 

        
/// Copy all face indices to entity structure
static void copyFaceIndices(das_WavefrontObjEntity *p_ent, char **words, uint32_t word_c) {
    // Check if indices need reallocation
    uint32_t cap = p_ent->data.ind_cap;
    reallocCheck((void**) &p_ent->data.ind_data.pos, &cap, p_ent->data.ind_data.n + 1, sizeof(uint32_t), 
                 "Failed to allocate memory for position indices");

    cap = p_ent->data.ind_cap;
    reallocCheck((void**) &p_ent->data.ind_data.tex, &cap, p_ent->data.ind_data.n + 1, sizeof(uint32_t), 
                 "Failed to allocate memory for texture indices");

    cap = p_ent->data.ind_cap;
    reallocCheck((void**) &p_ent->data.ind_data.norm, &cap, p_ent->data.ind_data.n + 1, sizeof(uint32_t), 
                 "Failed to allocate memory vertex normal indices");

    p_ent->data.ind_cap = cap;

    // For each face block parse it and save its data
    for(uint32_t i = 1; i < word_c; i++) {
        __das_IndexBlock block = parseFace(words[i]);
        
        // Check if parsed vertices index is UINT32_MAX
        p_ent->data.ind_data.n++;
        p_ent->data.ind_data.pos[p_ent->data.ind_data.n - 1] = block.pos - 1;
        p_ent->data.ind_data.tex[p_ent->data.ind_data.n - 1] = block.tex - 1;
        p_ent->data.ind_data.norm[p_ent->data.ind_data.n - 1] = block.norm - 1;
    }
}


/// Parse one line statement
static __das_WavefrontObjSpecType parseStatement(char **words, uint32_t word_c, uint64_t line) {
    // If no words are present then return
    if(!word_c) return DAS_WAVEFRONT_OBJ_SPEC_TYPE_NONE;

    // Find the keyword statement specifier
    __das_WavefrontObjStatement *statement = getTokenInfo(words[0]);

    // Check if statement is valid and not NULL otherwise throw syntax error
    if(!statement) {
        char buf[MAX_WORD_SIZE] = { 0 }; 
        sprintf(buf, "invalid keyword \"%s\"\n", words[0]);
        WAVEFRONT_SYNTAX_ERROR(line, buf);
    }

    // Check if statement has enough arguments
    if((int32_t) (word_c - 1) < statement->min_obj_c)
        NOT_ENOUGH_ARGS(line);

    // Check if statement has too many arguments
    else if((int32_t) (word_c - 1) > statement->max_obj_c)
        TOO_MANY_ARGS(line);

    return statement->spec_type;
}


/// Analyse the given line statement and perform actions accordingly
static void analyseStatement(char **swords, uint32_t word_c, das_WavefrontObjEntity **p_ent, 
                             uint32_t *p_ent_cap, uint32_t *p_ent_c, uint64_t lc) { 
    // Parse the line statement
    __das_WavefrontObjSpecType obj_spec_type = parseStatement(swords, word_c, lc);

    // Check for new action 
    switch(obj_spec_type) {
    case DAS_WAVEFRONT_OBJ_SPEC_TYPE_OBJ_DECL: {
        // Set a name variable if it exists
        char *name = NULL;
        if(word_c >= 2) 
            name = swords[1];
        
        // Create a new object entity
        newEntity(p_ent, p_ent_cap, p_ent_c, DAS_ENTITY_TYPE_OBJECT, name);
        break;
    }

    case DAS_WAVEFRONT_OBJ_SPEC_TYPE_GROUP_DECL: {
        // Set a name variable if it exists
        char *name = NULL;
        if(word_c >= 2) 
            name = swords[1];
        
        // Create a new group entity
        newEntity(p_ent, p_ent_cap, p_ent_c, DAS_ENTITY_TYPE_GROUP, name);
        break;
    }

    case DAS_WAVEFRONT_OBJ_SPEC_TYPE_VERT_DECL:
        // Check if vertex reallocation is needed
        reallocCheck((void**) &(*p_ent)[(*p_ent_c) - 1].data.vert_data.v3d.mul.pos, 
                     (void*) &(*p_ent)[(*p_ent_c) - 1].data.v_cap, (*p_ent)[(*p_ent_c) - 1].data.vert_data.v3d.mul.pn + 1, 
                     sizeof(das_PosData), "Failed to reallocate memory for position vertices");
            
        // Set the entity vertices
        (*p_ent)[(*p_ent_c) - 1].data.vert_data.v3d.mul.pos[(*p_ent)[(*p_ent_c) - 1].data.vert_data.v3d.mul.pn].vert_x = 
        (float) atof(swords[1]);

        (*p_ent)[(*p_ent_c) - 1].data.vert_data.v3d.mul.pos[(*p_ent)[(*p_ent_c) - 1].data.vert_data.v3d.mul.pn].vert_y = 
        (float) atof(swords[2]);

        (*p_ent)[(*p_ent_c) - 1].data.vert_data.v3d.mul.pos[(*p_ent)[(*p_ent_c) - 1].data.vert_data.v3d.mul.pn].vert_z = 
        (float) atof(swords[3]);

        (*p_ent)[(*p_ent_c) - 1].data.vert_data.v3d.mul.pn++;
        break;

    case DAS_WAVEFRONT_OBJ_SPEC_TYPE_VERT_TEX_DECL:
        // Check if vertex reallocation is needed
        reallocCheck((void**) &(*p_ent)[(*p_ent_c) - 1].data.vert_data.v3d.mul.tex, 
                     (void*) &(*p_ent)[(*p_ent_c) - 1].data.vt_cap, (*p_ent)[(*p_ent_c) - 1].data.vert_data.v3d.mul.tn + 1,
                     sizeof(das_TextureData), "Failed to reallocate memory for texture vertices");
            
        // Set the entity texture vertices
        (*p_ent)[(*p_ent_c) - 1].data.vert_data.v3d.mul.tex[(*p_ent)[(*p_ent_c) - 1].data.vert_data.v3d.mul.tn].tex_x = 
        (float) atof(swords[1]);

        (*p_ent)[(*p_ent_c) - 1].data.vert_data.v3d.mul.tex[(*p_ent)[(*p_ent_c) - 1].data.vert_data.v3d.mul.tn].tex_y = 
        (float) atof(swords[2]);

        (*p_ent)[(*p_ent_c) - 1].data.vert_data.v3d.mul.tn++;
        break;

    case DAS_WAVEFRONT_OBJ_SPEC_TYPE_VERT_NORM_DECL:
        /*// Check if vertex normal reallocation is needed*/
        reallocCheck((void**) &(*p_ent)[(*p_ent_c) - 1].data.vert_data.v3d.mul.norm, 
            (void*) &(*p_ent)[(*p_ent_c) - 1].data.vn_cap, (*p_ent)[(*p_ent_c) - 1].data.vert_data.v3d.mul.nn + 1,
            sizeof(das_NormalData), "Failed to reallocate memory for vertex normals");
            
        // Set the entity vertex normals
        (*p_ent)[(*p_ent_c) - 1].data.vert_data.v3d.mul.norm[(*p_ent)[(*p_ent_c) - 1].data.vert_data.v3d.mul.nn].nor_x = 
        (float) atof(swords[1]);

        (*p_ent)[(*p_ent_c) - 1].data.vert_data.v3d.mul.norm[(*p_ent)[(*p_ent_c) - 1].data.vert_data.v3d.mul.nn].nor_y = 
        (float) atof(swords[2]);

        (*p_ent)[(*p_ent_c) - 1].data.vert_data.v3d.mul.norm[(*p_ent)[(*p_ent_c) - 1].data.vert_data.v3d.mul.nn].nor_z = 
        (float) atof(swords[3]);

        (*p_ent)[(*p_ent_c) - 1].data.vert_data.v3d.mul.nn++;
        break;

    case DAS_WAVEFRONT_OBJ_SPEC_TYPE_FACE_DECL:
        copyFaceIndices(((*p_ent) + (*p_ent_c) - 1), swords, word_c);
        break;

    default:
        break;
    }
}


/// Find all text blocks used between beg and beg + len
static void extractBlocks(char *beg, uint32_t len, char ***p_words, uint32_t *p_word_c, 
                   uint32_t *p_word_cap, uint64_t max_word_len) {
    char *cur = beg;
    while(cur < beg + len) {
        // Skip all whitespaces, tabs and newlines
        while(cur < beg + len && (*cur == 0x20 || *cur == 0x09 || *cur == 0x0A || *cur == 0x0D))
            cur++;

        // Quit the loop if the maximum reading length is reached
        if(cur >= beg + len) break;
        
        // Check if block needs to be reallocated
        uint32_t old_cap = *p_word_cap;
        reallocCheck((void**) p_words, p_word_cap, (*p_word_c) + 1, sizeof(char*), 
                     "Could not reallocate memory for wavefront obj statements");

        // For each allocated pointer location allocate memory for each character
        for(uint32_t i = old_cap; i < (*p_word_cap); i++)
            (*p_words)[i] = (char*) calloc(max_word_len, sizeof(char));

        // For each character until whitespace or newline set the block's value
        for(uint32_t i = 0; cur < beg + len && *cur != 0x20 && *cur != 0x09 && *cur != 0x0A && *cur != 0x0D; i++, cur++) {
            // Check if the block size is larger than allowed
            if(i >= max_word_len) {
                uint32_t lc = findLineCount(__buffer, cur - __buffer);
                TOO_LONG_WORD(lc);
            }
            (*p_words)[(*p_word_c)][i] = *cur;
        }
        
        (*p_word_c)++;
        cur++;
    }
}


/// Create a new object or group instance for entity type
void newEntity(das_WavefrontObjEntity **p_ent, uint32_t *p_ecap, uint32_t *p_ent_c, 
               das_WavefrontObjEntityType ent_type, char *name) {
    // Check if memory needs to be reallocated for entity array
    uint32_t old_cap = *p_ecap;
    reallocCheck((void**) p_ent, p_ecap, (*p_ent_c) + 1, sizeof(das_WavefrontObjEntity), 
                 "Failed to reallocate memory for wavefront object entities");

    // If reallocation happened set the values of the newly allocated memory area to NULL
    for(uint32_t i = old_cap; i < (*p_ecap); i++)
        memset((void*) ((*p_ent) + i), 0, sizeof(das_WavefrontObjEntity));

    // Copy the name if available
    if(name) strcpy((*p_ent)[*p_ent_c].data.name, name);

    (*p_ent)[(*p_ent_c)].type = ent_type;
    (*p_ent_c)++;

    // Allocate initial amount of memory for indices 
    (*p_ent)[(*p_ent_c) - 1].data.ind_cap = DEFAULT_MEM_CAP;
    (*p_ent)[(*p_ent_c) - 1].data.ind_data.pos = (uint32_t*) calloc(DEFAULT_MEM_CAP, sizeof(uint32_t));
    (*p_ent)[(*p_ent_c) - 1].data.ind_data.tex = (uint32_t*) calloc(DEFAULT_MEM_CAP, sizeof(uint32_t));
    (*p_ent)[(*p_ent_c) - 1].data.ind_data.norm = (uint32_t*) calloc(DEFAULT_MEM_CAP, sizeof(uint32_t));

    // Allocate initial amount of memory for vertices 
    (*p_ent)[(*p_ent_c) - 1].data.v_cap = DEFAULT_MEM_CAP;
    (*p_ent)[(*p_ent_c) - 1].data.vert_data.v3d.mul.pos = (das_PosData*) calloc (
        DEFAULT_MEM_CAP, sizeof(das_PosData));

    (*p_ent)[(*p_ent_c) - 1].data.vt_cap = DEFAULT_MEM_CAP;
    (*p_ent)[(*p_ent_c) - 1].data.vert_data.v3d.mul.tex = (das_TextureData*) calloc (
        DEFAULT_MEM_CAP, sizeof(das_TextureData));

    (*p_ent)[(*p_ent_c) - 1].data.vn_cap = DEFAULT_MEM_CAP;
    (*p_ent)[(*p_ent_c) - 1].data.vert_data.v3d.mul.norm = (das_NormalData*) calloc (
        DEFAULT_MEM_CAP, sizeof(das_NormalData));
}


/// Find the current char line count from the beginning of the string
static uint32_t findLineCount(char *str, uint32_t cbyte) {
    uint32_t lc = 1;

    // For each character check if it is newline
    for(size_t i = 0; i < cbyte; i++)
        if(str[i] == 0x0A) lc++;

    return lc;
}


/// Temporary function for printing out all entity data
void printEntityData(das_WavefrontObjEntity *entities, uint32_t ent_c) {
    // For each entity print out its contents
    for(uint32_t i = 0; i < ent_c; i++) {
        printf("o %s\n", entities[i].data.name);

        // Print all position vertices data
        for(uint32_t j = 0; j < entities[i].data.vert_data.v3d.mul.pn; j++) {
            printf("v %f %f %f\n", entities[i].data.vert_data.v3d.mul.pos[j].vert_x,
                entities[i].data.vert_data.v3d.mul.pos[j].vert_y, entities[i].data.vert_data.v3d.mul.pos[j].vert_z);
        }

        // Print all texture vertices data
        for(uint32_t j = 0; j < entities[i].data.vert_data.v3d.mul.tn; j++) {
            printf("vt %f %f\n", entities[i].data.vert_data.v3d.mul.tex[j].tex_x,
                entities[i].data.vert_data.v3d.mul.tex[j].tex_y);
        }

        // Print all vertex normals data
        for(uint32_t j = 0; j < entities[i].data.vert_data.v3d.mul.nn; j++) {
            printf("vn %f %f %f\n", entities[i].data.vert_data.v3d.mul.norm[j].nor_x,
                entities[i].data.vert_data.v3d.mul.norm[j].nor_y, entities[i].data.vert_data.v3d.mul.norm[j].nor_z);
        }

        // Print all indices
        for(uint32_t j = 0; j < entities[i].data.ind_data.n; j++) {
            printf("f %d/%d/%d\n", entities[i].data.ind_data.pos[j], entities[i].data.ind_data.tex[j],
                entities[i].data.ind_data.norm[j]);
        }
    }
}


/// Create a hashmap out of all statements that can be accessed with keyword keys
static void tokenise() {
    // Create a new hashmap
    newHashmap(&__statement_map, 4 * sizeof(__statements));

    // For each statement push the entry to the hashmap keyword key
    for(size_t i = 0; i < sizeof(__statements) / sizeof(__statements[0]); i++) {
        pushToHashmap(&__statement_map, __statements[i].keyword, 
            strlen(__statements[i].keyword), &__statements[i]);
    }
}


/// Free all memory that was used by tokens
static void untokenise() {
    destroyHashmap(&__statement_map);
}


/// Retrieve token statement info from the hashmap using the keyword's value
static __das_WavefrontObjStatement *getTokenInfo(char *id) {
    return (__das_WavefrontObjStatement*) findValue(&__statement_map, id, strlen(id));
}
