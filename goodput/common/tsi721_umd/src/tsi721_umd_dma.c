#include <stdint.h>
#include <string.h>
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

	bd_ptr->type_id = le32((dtype << 29) |
	                (rtype << 19) | (prio << 17) | (crf << 16) |
	                (uint32_t)devid);

	bd_ptr->bcount = le32(((raddr_lsb64 & 0x3) << 30) |
	                (tt << 26) | bcount);

	raddr_lsb64 = ((uint64_t)(raddr_msb2 & 0x3) << 62) |
	                (raddr_lsb64 >> 2);

	bd_ptr->raddr_lo = le32(raddr_lsb64 & 0xffffffff);
	bd_ptr->raddr_hi = le32(raddr_lsb64 >> 32);
};

