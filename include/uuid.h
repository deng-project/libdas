/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: uuid.h - simple UUID generation function header 
/// author: Karl-Mihkel Ott


#ifndef __UUID_H
#define __UUID_H

#define __UUID_VERSION 0x10
#define __UUID_VARIANT 0x40

#ifdef __UUID_CPP
    #define BUF_LEN     512
    #include <cstring>
    #include <cstdlib>
    #include <cstdio>
    #include <chrono>

    #ifdef __linux__
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
    //char *__uuid_GetDevMacAddress();
#endif

typedef struct {
    char bytes[33];
} uuid_t;


/// Random number generators
//uint64_t randUI64();
//uint32_t randUI32();
//uint16_t randUI16();
//uint8_t randUI8();

/// Generate and allocate memory for new uuid instance
//uuid_t uuid_Generate();

namespace libdas {

    class id_generator {
        private:
            uuid_t m_id = {};

        private:
            uint64_t randUI64();
            uint32_t randUI32();
            uint16_t randUI16();
            uint8_t randUI8();
            const char *getDeviceMacAddress();

        public:
            id_generator();
            void newID();
            uuid_t &getUUID();
    };

}


#endif
