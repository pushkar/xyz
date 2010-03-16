/*
 * aio_writer.h
 *
 *  Created on: Feb 24, 2010
 *      Author: pushkar
 */

#ifndef AIO_WRITER_H_
#define AIO_WRITER_H_

#include <aio.h>
#include "aio_ds.h"

class aio_writer {
private:
	int fd;
	uint32_t fw_max, bf_max;
	uint64_t fs;
	aiocb *aio;
	aiobf **bf;
	char **data;
	const aiocb *cb[1];

	int fwait(uint32_t fw);
public:
	aio_writer(uint32_t max_buffer_size, uint32_t max_async_writes);
	~aio_writer();

	int fopen(const char* filename);
	int fwrite(void* buffer, uint32_t buffer_size);
	void fclose();
};

#endif /* AIO_WRITER_H_ */
