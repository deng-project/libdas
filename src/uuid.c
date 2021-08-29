/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: uuid.c - simple UUID generation function implementation 
/// author: Karl-Mihkel Ott


#define __UUID_C
#include <uuid.h>


uint64_t randUI64() {
    uint64_t out = 0;
	#if defined(__linux__)
        // Read random data from /dev/urandom
        FILE* file;
        file = fopen("/dev/urandom", "rb");
        if (!file) {
            fprintf(stderr, "Could not read /dev/urandom");
            exit(-1);
        }

        fread(&out, sizeof(uint64_t), 1, file);
        fclose(file);
    #elif defined(_WIN32)
		HCRYPTPROV crypt_prov = { 0 };
        if (!CryptAcquireContext(&crypt_prov, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT))
            RUN_ERR("cm_randi64()", "Failed to acquire WIN32 cryptographic context");
        if(!CryptGenRandom(crypt_prov, sizeof(uint64_t), (BYTE*) &out))
            RUN_ERR("cm_randi64()", "Failed to generate random cryptgraphically secure number");
	#endif

	return out;
}


uint32_t randUI32() {
    uint64_t tmp = randUI64();
    return *(uint32_t*) &tmp;
}


uint16_t randUI16() {
    uint64_t tmp = randUI64();
    return *(uint16_t*) &tmp;
}


uint8_t randUI8() {
    uint64_t tmp = randUI64();
    return *(uint8_t*) &tmp;
}


/// Find first non loopback network device's mac address
char *__uuid_GetDevMacAddress() {
    static char out[16] = {0};
    char buf[18] = {0};
    size_t offset = 0;

    #if defined(__linux__)
        // Read the mac address of the first network device that isn't lo
        DIR *dir;
        FILE *file;
        struct dirent *c;
         
        dir = opendir(__LINUX_NET_DEV_DIR);
        if(!dir) {
            fprintf(stderr, "Could not open directory %s for reading\n", __LINUX_NET_DEV_DIR);
            exit(-1);
        }

        while((c = readdir(dir))) {
            if(c->d_type == DT_LNK && strcmp(c->d_name, ".") && strcmp(c->d_name, "..") && 
               strcmp(c->d_name, "lo") && strcmp(c->d_name, "sit0")) {
                char addr[128] = { 0 };
                sprintf(addr, "%s/%s/address", __LINUX_NET_DEV_DIR, c->d_name);

                file = fopen(addr, "rb");
                if(!file) {
                    fprintf(stderr, "Could not open file %s\n", addr);
                    exit(-1);
                }

                fread(buf, sizeof(char), 17, file);
                fclose(file);
                break;
            }
        }

        closedir(dir);

        if(buf[0] == 0x00)
            perror("No mac address found!\n");

    #elif defined(_WIN32)
		IP_ADAPTER_INFO *p_ad_info = (IP_ADAPTER_INFO*) malloc(sizeof(IP_ADAPTER_INFO));
        ULONG out_buf_len = sizeof(IP_ADAPTER_INFO);
        DWORD ret_val = 0;

        // According to Microsoft documentation the initial call to GetAdaptersInfo()
        // is supposed to fail, since we do not know how many network devices we have
        if (GetAdaptersInfo(p_ad_info, &out_buf_len) != ERROR_SUCCESS) {
            free(p_ad_info);
            p_ad_info = (IP_ADAPTER_INFO*) malloc(out_buf_len);
        }

        // Now this call should be successful since we have allocated enough memory
        // for all possible network adapters
        if ((ret_val = GetAdaptersInfo(p_ad_info, &out_buf_len)) != ERROR_SUCCESS) {
            RUN_ERR(
                "__uuid_GetDevMacAddress",
                "Failed to retrieve information about network devices"
            );
        }

        PIP_ADAPTER_INFO p_ad = p_ad_info;

        // Test the network device name retrieval
        while (p_ad) {
            char tmp_mac[18] = { 0 };
            for (UINT i = 0; i < p_ad->AddressLength; i++) {
                if (i == p_ad->AddressLength - 1)
                    sprintf(tmp_mac + offset, "%02x", (int)p_ad->Address[i]);
                else {
                    sprintf(tmp_mac + offset, "%02x:", (int)p_ad->Address[i]);
                    offset += 3;
                }
            }

            // Verify that the mac address is not zeroed
            if (strcmp(tmp_mac, "00:00:00:00:00:00"))
                break;

			p_ad = p_ad->Next;
        }
    #endif

    // Parse MAC address into byte array
    char *ptr = buf;
    char *end = buf + 18;
    char *br = buf;
    while(ptr < end && offset < 12) {
        br = strchr(ptr, ':') ? strchr(ptr, ':') : end;
        strncpy (
            out + offset,
            ptr,
            br - ptr
        );

        offset += 2;
        ptr = br + 1;
    }

    return out;
}   


/// Init new uuid instance
id_t uuid_Generate() {
    time_t time_val = time(NULL);
    srand(time_val);

    // Pseudo randomise the time value
    uint32_t tval[2];
    tval[0] = (uint32_t) time(NULL) ^ randUI32();
    tval[1] = (uint32_t) time(NULL) ^ randUI32();
    uint8_t *time_s = (uint8_t*) tval;
    
    // Copy lower 4 bytes of the time to uuid instance
    memcpy(__uuid.bytes, time_s + 4, 4);

    // Copy medium 2 bytes of the time to uuid instance
    memcpy(__uuid.bytes + 4, time_s + 2, 2);

    // Copy highest 2 bytes of the time to uuid instance
    memcpy(__uuid.bytes + 6, time_s, 2);

    // Shift bits to right and add UUID version specier bits
    __uuid.bytes[6] >>= 4;
    __uuid.bytes[7] >>= 4;
    __uuid.bytes[6] |= __UUID_VERSION;
    uint64_t mac_randomizer = 0;

    uint16_t cl_seq = randUI16();
    __clock_seq[0] = *(uint8_t*) &cl_seq;
    __clock_seq[1] = *(((uint8_t*) &cl_seq) + 1);
    mac_randomizer = randUI64();

    // Copy clock sequence to uuid instance 
    memcpy(__uuid.bytes + 8, __clock_seq, 2);
    __uuid.bytes[8] >>= __UUID_VARIANT_C;
    __uuid.bytes[9] >>= __UUID_VARIANT_C;
    __uuid.bytes[8] |= __UUID_VARIANT;
    
    char *mac = __uuid_GetDevMacAddress();
    uint32_t mac_hex = strtol(mac, NULL, 16);
    mac_hex ^= mac_randomizer;

    id_t id = { 0 };

    sprintf (
        id.bytes,
        "%08x-%04x-%04x-%04x-%08x",
        *(uint32_t*) (__uuid.bytes),
        *(uint16_t*) (__uuid.bytes + 4),
        *(uint16_t*) (__uuid.bytes + 6),
        *(uint16_t*) (__uuid.bytes + 8),
        mac_hex
    );

    return id;
}
