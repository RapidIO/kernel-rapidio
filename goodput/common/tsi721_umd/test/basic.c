#include <stdio.h>
#include "tsi721_umd.h"

// Test basic opening and setup of the user-mode driver

int main(void)
{
	int ret;
	struct tsi721_umd umd;

	ret = tsi721_umd_open(&umd, 0);
	if (ret < 0)
		return -1;

	ret = tsi721_umd_queue_config_multi(&umd, 0x03, (void*)0x50000000, 256*1024*1024);
	if (ret < 0)
		return -1;

	ret = tsi721_umd_start(&umd);
	if (ret < 0)
		return -1;

	return 0;
}
