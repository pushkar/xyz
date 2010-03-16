/*
 * aio_writter.cpp
 *
 *  Created on: Feb 24, 2010
 *      Author: pushkar
 */

#include "aio_writer.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

aio_writer::aio_writer(uint32_t max_buffer_size , uint32_t max_async_writes) {
	if(max_async_writes < 1) max_async_writes = 1;
	if(max_buffer_size < 1) max_buffer_size = 1;

	fs = 0;
	fw_max = max_async_writes;
	bf_max = max_buffer_size;
	aio = (aiocb*) malloc (sizeof(aiocb) * fw_max);
	bf = (aiobf**) malloc (sizeof(aiobf*) * fw_max);
	data = (char**) malloc (sizeof(char*) * fw_max);

	for(uint32_t i = 0; i < fw_max; i++) {
		memset(&aio[i], '\0', sizeof(aiocb));
		bf[i] = (aiobf*) malloc (sizeof(aiobf));
		memset(bf[i], '\0', sizeof(aiobf));
		data[i] = (char*) malloc (sizeof(aiobf) + bf_max);
		memset(data[i], '\0', sizeof(bf_max));
		aio[i].aio_offset = 0;
		aio[i].aio_lio_opcode = 0;
		aio[i].aio_reqprio = 0;
		aio[i].aio_buf = data[i];
		aio[i].aio_nbytes = sizeof(aiobf)+bf_max;
		aio[i].aio_sigevent.sigev_notify = SIGEV_NONE;
		aio[i].aio_sigevent.sigev_signo = 0;
		aio[i].aio_sigevent.sigev_value.sival_int = 0;
	}
}

aio_writer::~aio_writer() {
	fclose();
	free(aio);
	free(&bf);
	free(&data);
}

int aio_writer::fopen(const char* filename) {
	fd = open(filename, O_CREAT | O_TRUNC | O_RDWR | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
	for(uint32_t i = 0; i < fw_max && fd > 0; i++) {
		aio[i].aio_fildes = fd;
	}
	return fd;
}

// Returns number of bytes returned, same as sync write()
int aio_writer::fwait(uint32_t fw) {
	int ret = 0;
	ret = aio_error(&aio[fw]);
	if(ret != 0) {
		cb[0] = &aio[fw];
		ret = aio_suspend(cb, 1, NULL);
	}
	if(ret == 0)
		return aio_return(&aio[fw]);

	return ret;
}

// Returns 0 on success
int aio_writer::fwrite(void* buf, uint32_t buf_size) {
	int ret = 0;
	if(buf_size > bf_max) buf_size = bf_max;
	uint32_t fw = fs % fw_max;

	if(fs > fw_max) {
		ret = fwait(fw);
		if(ret < 0) return ret;
	}

	bf[fw]->valid = 1;
	bf[fw]->serial = fs;
	bf[fw]->size = buf_size;
	memcpy(data[fw], bf[fw], sizeof(aiobf));
	memcpy(data[fw] + sizeof(aiobf), buf, buf_size);

	aio[fw].aio_buf = data[fw];
	aio[fw].aio_offset = fs * (sizeof(aiobf) + bf_max);
	aio[fw].aio_nbytes = sizeof(aiobf) + buf_size;
	ret = aio_write(&aio[fw]);
	fs++;

	return ret;
}

void aio_writer::fclose() {
	aio_fsync(O_SYNC, &aio[0]);
	for(uint32_t i = 0; i < fw_max; i++) {
		fwait(i);
	}
	close(fd);
}
