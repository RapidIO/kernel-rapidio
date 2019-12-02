#include <stdint.h>
#include "Tsi721.h"

/* Register offsets */

/* Register access macros */
#define TSI721_WR32(reg, channel, value) (*(volatile uint32_t*)((uintptr_t)h->all_regs + reg(channel))= value)
#define TSI721_RD32(reg, channel)        (*(volatile uint32_t*)((uintptr_t)h->all_regs + reg(channel)))


/* Descriptor types for BDMA and Messaging blocks */
enum dma_dtype {
	DTYPE1 = 1, /* Data Transfer DMA Descriptor */
	DTYPE2 = 2, /* Immediate Data Transfer DMA Descriptor */
	DTYPE3 = 3, /* Block Pointer DMA Descriptor */
	DTYPE4 = 4, /* Outbound Msg DMA Descriptor */
	DTYPE5 = 5, /* OB Messaging Block Pointer Descriptor */
	DTYPE6 = 6  /* Inbound Messaging Descriptor */
};

enum dma_rtype {
	NREAD = 0,
	LAST_NWRITE_R = 1,
	ALL_NWRITE = 2,
	ALL_NWRITE_R = 3,
	MAINT_RD = 4,
	MAINT_WR = 5
};

typedef struct {
	uint32_t type_id;

#define TSI721_DMAD_DEVID	0x0000ffff
#define TSI721_DMAD_CRF		0x00010000
#define TSI721_DMAD_PRIO	0x00060000
#define TSI721_DMAD_RTYPE	0x00780000
#define TSI721_DMAD_IOF		0x08000000
#define TSI721_DMAD_DTYPE	0xe0000000

	uint32_t bcount;

#define TSI721_DMAD_BCOUNT1	0x03ffffff /* if DTYPE == 1 */
#define TSI721_DMAD_BCOUNT2	0x0000000f /* if DTYPE == 2 */
#define TSI721_DMAD_TT		0x0c000000
#define TSI721_DMAD_RADDR0	0xc0000000

	union {
		uint32_t raddr_lo;	   /* if DTYPE == (1 || 2) */
		uint32_t next_lo;		   /* if DTYPE == 3 */
	};

#define TSI721_DMAD_CFGOFF	0x00ffffff
#define TSI721_DMAD_HOPCNT	0xff000000

	union {
		uint32_t raddr_hi;	   /* if DTYPE == (1 || 2) */
		uint32_t next_hi;		   /* if DTYPE == 3 */
	};

	union {
		struct {		   /* if DTYPE == 1 */
			uint32_t bufptr_lo;
			uint32_t bufptr_hi;
			uint32_t s_dist;
			uint32_t s_size;
		} t1;
		uint32_t data[4];		   /* if DTYPE == 2 */
		uint32_t    reserved[4];	   /* if DTYPE == 3 */
	};
} tsi721_dma_desc __attribute__ ((aligned(32)));

#define TSI721_BDMA_MAX_BCOUNT	(TSI721_DMAD_BCOUNT1 + 1)

void tsi721_umd_create_dma_descriptor(tsi721_dma_desc* bd_ptr,
			  uint16_t devid,
			  uint32_t bcount,
			  uint64_t raddr_lsb64,
			  uint8_t raddr_msb2,
			  uint8_t *buffer_ptr);
