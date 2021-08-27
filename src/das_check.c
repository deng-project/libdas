/// DAM: DENG asset manager 
/// license: Apache, see LICENSE file
/// file: das_check.c


#define __DAS_CHECK_C
#include <data/das_check.h>


/// Check if the requested memory size is available for reading otherwise
/// throw a runtime error
void das_ErrBufferReadCheck (
    size_t req_size, 
    size_t max_size,
    const char *file_name
) {
    if(req_size >= max_size) {
        fprintf(stdout, "Failed to read from file %s, possible file corruption\n", file_name);
        exit(DAS_ERROR_FILE_CORRUPTION);
    }
}


/// Check if the given header specifier is valid
void das_CheckHdrName(char *name, char *exp_name, const char *file_name) {
    // Copy the non null terminated name to bigger string
    char nname[9] = { 0 };
    strncpy(nname, name, 8);

    // Check if the name does not match the expected name
    if(strcmp(nname, exp_name))
        __DAS_READ_CORRUPT_ERROR(file_name);
}


/// Check if provided uuid is valid
das_Error das_UuidCheck(char *uuid) {
    // Check if UUID length is correct and separators are in correct places
    if(strlen(uuid) != __DAS_UUID_LEN || uuid[8] != '-' || 
       uuid[14] != '-' || uuid[19] != '-' || uuid[23] != '-' || uuid[27] != '-')
        return DAS_ERROR_INVALID_UUID;

    // Check each character in uuid sequence for any invalid characters
    for(size_t i = 0; i < strlen(uuid); i++) {
        if(i == 14 || i == 19 || i == 23 || i == 27) 
            continue;

        if((uuid[i] < '0' && uuid[i] > '9') && (uuid[i] < 'a' && uuid[i] > 'z') &&
           (uuid[i] < 'A' && uuid[i] > 'Z')) 
            return DAS_ERROR_INVALID_UUID;
    }

    return DAS_ERROR_SUCCESS;
}


/// Check if magic number provided is valid or not
das_Error das_MagicNumberCheck(deng_ui64_t num) {
    if(num != __DAS_STATIC_MAGIC_NUMBER && 
       num != __DAS_ANIMATION_MAGIC_NUMBER &&
       num != __DAS_MAP_MAGIC_NUMBER)
        return DAS_ERROR_INVALID_MAGIC_NUMBER;

    return DAS_ERROR_SUCCESS;
}
