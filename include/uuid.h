/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: uuid.h - simple UUID generation function header 
/// author: Karl-Mihkel Ott


#ifndef __UUID_H
#define __UUID_H

#ifdef __cplusplus
extern "C" {
#endif

#define __UUID_VERSION 0x10
#define __UUID_VARIANT 0x40

#ifdef __UUID_C
    #define BUF_LEN     512
    #include <stdlib.h>
    #include <stdint.h>
    #include <stdio.h>
    #include <time.h>
    #include <string.h>
    #include <limits.h>

    #ifdef __linux__
        #define __USE_MISC
        #include <dirent.h>
        #include <linux/random.h>
        #define __LINUX_NET_DEV_DIR     "/sys/class/net"
    #elif _WIN32
        #include <Windows.h>
        #include <iphlpapi.h>
    #endif

    #define __UUID_VARIANT_C    2
    
    /// Basic structure for 16 byte UUID storage byte per byte
    typedef struct UUIDType {
        uint8_t bytes[10];
    } UUIDType;

    UUIDType __uuid = {0};
    uint8_t __clock_seq[2];


    /// Find first non lo network device's mac address
    char *__uuid_GetDevMacAddress();
#endif

typedef struct {
    char bytes[33];
} id_t;


/// Random number generators
uint64_t randUI64();
uint32_t randUI32();
uint16_t randUI16();
uint8_t randUI8();

/// Generate and allocate memory for new uuid instance
id_t uuid_Generate();


#ifdef __cplusplus
}
#endif

#endif
