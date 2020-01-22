#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "tsi721_umd_dma.h"

static inline uint32_t le32(const uint32_t n)
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
    return n;
#else
    return __builtin_bswap32(n);
#endif
}

void tsi721_umd_create_dma_descriptor (tsi721_dma_desc* bd_ptr, 
                uint16_t devid, uint32_t bcount,
                uint64_t raddr_lsb64, uint8_t raddr_msb2,
                uint8_t *buffer_ptr)

{
    const uint8_t rtype = ALL_NWRITE;
    const uint8_t prio  = 0;
    const uint8_t crf   = 0;
    const uint8_t tt    = 1;

    enum dma_dtype dtype = DTYPE1;

    memset(bd_ptr, 0, sizeof(*bd_ptr));

    if (bcount == (TSI721_DMAD_BCOUNT1+1))
        bcount = 0;
    bcount &= TSI721_DMAD_BCOUNT1;

    bd_ptr->t1.bufptr_lo = le32((uint64_t)buffer_ptr & 0xffffffff);
    bd_ptr->t1.bufptr_hi = le32((uint64_t)buffer_ptr >> 32);

    bd_ptr->word0.all = le32((dtype << 29) |
                    (rtype << 19) | (prio << 17) | (crf << 16) |
                    (uint32_t)devid);

    bd_ptr->bcount = le32(((raddr_lsb64 & 0x3) << 30) |
                    (tt << 26) | bcount);

    raddr_lsb64 = ((uint64_t)(raddr_msb2 & 0x3) << 62) |
                    (raddr_lsb64 >> 2);

    bd_ptr->raddr_lo = le32(raddr_lsb64 & 0xffffffff);
    bd_ptr->raddr_hi = le32(raddr_lsb64 >> 32);
};

void tsi721_umd_init_dma_descriptors(tsi721_dma_desc* bd_ptr, uint32_t num_descriptors)
{
    uint32_t i;
    const uint8_t rtype = ALL_NWRITE;
    const uint8_t prio  = 0;
    const uint8_t crf   = 0;
    enum dma_dtype dtype = DTYPE1;

    memset(bd_ptr, 0, sizeof(*bd_ptr) * num_descriptors);
    tsi721_dma_desc init_desc = {0};
    init_desc.word0.all = le32((dtype << 29) | (rtype << 19) | (prio << 17) | (crf << 16));

    for (i=0; i<num_descriptors; i++)
    {
        *(bd_ptr++) = init_desc;
    }
}

void tsi721_umd_update_dma_descriptor(tsi721_dma_desc* bd_ptr, uint64_t raddr_lsb64, uint64_t raddr_msb2, uint16_t devid, uint32_t bcount, void* buffer_ptr)
{
    const uint8_t tt = 1;
    uint64_t rio_addr_hi64;

    bd_ptr->word0.info.devid  = devid;
    bd_ptr->bcount = le32(((raddr_lsb64 & 0x3) << 30) | (tt < 26) | bcount);

    rio_addr_hi64 = ((uint64_t)(raddr_msb2 & 0x3) << 62) | (raddr_lsb64>>2);
    bd_ptr->raddr_hi = le32(rio_addr_hi64 >> 32);
    bd_ptr->raddr_lo = le32(rio_addr_hi64 & 0xFFFFFFFF);

    bd_ptr->t1.bufptr_hi = le32((uint64_t)buffer_ptr >> 32);
    bd_ptr->t1.bufptr_lo = le32((uint64_t)buffer_ptr & 0xffffffff);
}
