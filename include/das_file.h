
#ifndef __DAS_FILE_H
#define __DAS_FILE_H

#define DAS_FROASSERT(val, msg, fname)          if(!(val)) fprintf(stderr, "%s\nFailed to load file %s\n", msg, fname), exit(-1)
#define DAS_FWOASSERT(val, msg, fname)          if(!(val)) fprintf(stderr, "%s\nFailed to write file %s\n", msg, fname), exit(-1)

#define DAS_FILE_HEADER_SIG                 (uint32_t) 0x00534144
#define DAS_INFO_HEADER_SIG                 (uint64_t) 0x5244485f4f464e49
#define DAS_VERT_HEADER_SIG                 (uint64_t) 0x5244485f54524556
#define DAS_VPOS_HEADER_SIG                 (uint64_t) 0x5244485f534f5056
#define DAS_VTEX_HEADER_SIG                 (uint64_t) 0x5244485f58455456
#define DAS_VNOR_HEADER_SIG                 (uint64_t) 0x5244485f524f4e56
#define DAS_INDX_HEADER_SIG                 (uint64_t) 0x5244485f58444e49
#define DAS_META_HEADER_SIG                 (uint64_t) 0x5244485f4154454d

#define DAS_ASSET_MAGIC_NUMBER              (uint64_t) 0x5453544553534144
#define DAS_ANIMATION_MAGIC_NUMBER          (uint64_t) 0x4e41544553534144
#define DAS_MAP_MAGIC_NUMBER                (uint64_t) 0x504d544553534144
#define DAS_UUID_LEN                        33


typedef struct das_FILE_HDR {
    uint32_t hdr_sig;
    unsigned char pad[12];
} das_FILE_HDR;


typedef struct das_INFO_HDR {
    uint64_t hdr_sig;
    uint32_t hdr_size;
    char uuid[DAS_UUID_LEN];
    uint64_t time_st;
    uint8_t asset_type;
    uint8_t cmpr;
} das_INFO_HDR;


typedef struct das_META_HDR {
    uint64_t hdr_sig;
    uint32_t hdr_size;
    uint32_t data_size;
    char *data;
} das_META_HDR;


typedef struct das_VERT_HDR {
    uint64_t hdr_sig;
    uint32_t hdr_size;
} das_VERT_HDR;


typedef struct das_VertAttribute{
    uint64_t hdr_sig;
    uint32_t hdr_size;
    uint32_t vert_c;
    uint8_t esize;
} das_VertAttribute;


typedef struct das_INDX_HDR {
    uint64_t hdr_sig;
    uint32_t hdr_size;
    uint32_t ind_c;
    uint32_t *pinds;
    uint32_t *tinds;
    uint32_t *ninds;
} das_INDX_HDR;

#endif
