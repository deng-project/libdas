/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: wobj_tokens.c - wavefront obj token handler source file
/// author: Karl-Mihkel Ott


#define __WOBJ_TOKENS_C
#include <wobj_tokens.h>


/// Create a hashmap out of all statements that can be accessed with keyword keys
void das_WavefrontObjTokenise() {
    // Create a new hashmap
    newHashmap(&__statement_map, 4 * sizeof(__statements));

    // For each statement push the entry to the hashmap keyword key
    for(size_t i = 0; i < sizeof(__statements) / sizeof(__statements[0]); i++) {
        pushToHashmap(&__statement_map, __statements[i].keyword, 
            strlen(__statements[i].keyword), &__statements[i]);
    }
}


/// Free all memory that was used by tokens
void das_WavefrontObjUntokenise() {
    destroyHashmap(&__statement_map);
}


/// Retrieve token statement info from the hashmap using the keyword's value
__das_WavefrontObjStatement *das_GetTokenInfo(char *id) {
    return (__das_WavefrontObjStatement*) findValue(&__statement_map, id, strlen(id));
}
