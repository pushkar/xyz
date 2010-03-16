/*
 * aio_reader.h
 *
 *  Created on: Feb 25, 2010
 *      Author: pushkar
 */

#ifndef AIO_READER_H_
#define AIO_READER_H_

#include <aio.h>
#include "aio_ds.h"

class aio_reader {
private:
	int fd;
	uint32_t fr_max, bf_max;
	uint64_t fs;
	aiocb *aio;
	aiobf **bf;
	char **data;
	const aiocb *cb[1];

	int fetch(int fs);
public:
	aio_reader(uint32_t max_buffer_size, uint32_t max_async_reads);
	~aio_reader();

	int fopen(const char* filename);
	int freadfullbuffer();
	int fread(void* buffer, uint32_t* buffer_size);
	int fetchnew();
	void fclose();
};

#endif /* AIO_READER_H_ */
