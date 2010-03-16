/*
 * aio_reader.cpp
 *
 *  Created on: Feb 25, 2010
 *      Author: pushkar
 */

#include "aio_reader.h"
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

aio_reader::aio_reader(uint32_t max_buffer_size, uint32_t max_async_reads) {
	if(max_async_reads < 1) max_async_reads = 1;
	if(max_buffer_size < 1) max_buffer_size = 1;

	fs = 0;
	fr_max = max_async_reads;
	bf_max = max_buffer_size;
	aio = (aiocb*) malloc (sizeof(aiocb) * fr_max);
	bf = (aiobf**) malloc (sizeof(aiobf*) * fr_max);
	data = (char**) malloc (sizeof(char*) * fr_max);

	for(uint32_t i = 0; i < fr_max; i++) {
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

aio_reader::~aio_reader() {
	fclose();
	free(aio);
	free(&bf);
	free(&data);
}

int aio_reader::fopen(const char* filename) {
	fd = open(filename, O_RDONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
	for(uint32_t i = 0; i < fr_max && fd > 0; i++) {
		aio[i].aio_fildes = fd;
	}
	return fd;
}

int aio_reader::fetch(int fs) {
	int fr = fs % fr_max;
	aio[fr].aio_offset = fs * (sizeof(aiobf) + bf_max);
	return aio_read(&aio[fr]);
}

int aio_reader::freadfullbuffer() {
	int ret = -1;
	for(uint32_t i = 0; i < fr_max; i++) {
		ret = fetch(fs+i);
		if(ret != 0) break;
	}
	return ret;
}

// Returns number of bytes returned, same as sync read()
int aio_reader::fread(void* buffer, uint32_t* buffer_size) {
	int fr = fs % fr_max;
	int ret = aio_error(&aio[fr]);
	if(ret != 0) {
		cb[0] = &aio[fr];
		ret = aio_suspend(cb, 1, NULL);
	}

	if(ret == 0) {
		ret = aio_return(&aio[fr]);
		memcpy(bf[fr], data[fr], sizeof(aiobf));
		if(bf[fr]->valid) {
			memcpy(buffer, data[fr] + sizeof(aiobf), bf[fr]->size);
			*buffer_size = bf[fr]->size;
		}
	}

	return ret;
}

int aio_reader::fetchnew() {
	int ret = fetch(fs + fr_max);
	fs++;
	return ret;
}

void aio_reader::fclose() {
	aio_fsync(O_SYNC, &aio[0]);
	close(fd);
}



