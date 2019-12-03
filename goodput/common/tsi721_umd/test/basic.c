#include <stdio.h>
#include "tsi721_umd.h"

#define MAX_MSG_PER_BUF (16)

// Test basic opening and setup of the user-mode driver

int main(int argc, char** argv)
{
	int ret;
	uint32_t i;
	struct tsi721_umd umd;
	const uint64_t q_addr = 0x50000000;
	const uint32_t q_size = 8 * 8192;
	const uint64_t dma_buf_addr = q_addr + 256*1024;
	const uint32_t dma_buf_size = 1024*1024;
	const uint32_t msg_size = dma_buf_size/MAX_MSG_PER_BUF;
	const uint16_t dest_id = 0;
	const uint64_t rio_base = 0xB1000000;
	uint32_t num_writes = 1;

	if (argc > 1)
		num_writes = atoi(argv[1]);

	ret = tsi721_umd_open(&umd, 0);
	if (ret < 0)
		return -1;

	ret = tsi721_umd_queue_config_multi(&umd, 0x03, (void*)q_addr, q_size);
	if (ret < 0)
		return -1;

	ret = tsi721_umd_start(&umd);
	if (ret < 0)
		return -1;

	// Write some data into physical memory to be used for DMA test
	uint64_t handle = dma_buf_addr;
	ret = rio_dbuf_alloc(umd.dev_fd, dma_buf_size, &handle);
	if (ret != 0)
	{
		printf("Failed to allocate buffer to hold output payload\n");
		return -1;
	}

	uint32_t* ptr = mmap(NULL, dma_buf_size, PROT_READ | PROT_WRITE, MAP_SHARED, umd.dev_fd, handle);
	if (ptr == MAP_FAILED)
	{
		printf("Failed to mmap DMA buffer\n");
		return -1;
	}

	printf("Allocated DMA buffer at %p (phys %lx)\n",ptr,dma_buf_addr);

	for (i=0; i<dma_buf_size/4; i++)
		ptr[i] = (i & 0x0000FFFF) | 0xCAFE0000;

	for (i=0; i<num_writes; i++)
	{
		ret = tsi721_umd_send(
				&umd, 
				(void*)dma_buf_addr + i*msg_size, // phys addr
				msg_size, // size 
				rio_base + i*msg_size, // rio addr
			   	dest_id); // dest ID

		if (ret != 0)
		{
			printf("tsi721_umd_send %d failed\n",i);
			return -1;
		}

		printf("tsi721_umd_send %d success\n",i);
	}

	return 0;
}
