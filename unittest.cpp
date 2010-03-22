/*
 * unittest.cpp
 *
 *  Created on: Mar 20, 2010
 *      Author: pushkar
 */

#include "aio_reader.h"
#include "aio_writer.h"


int aio_test() {
	uint32_t dsize = 50;
	uint32_t rsize = 0;
	char* wbuffer;
	char* rbuffer;
	wbuffer = (char*) malloc(dsize);
	rbuffer = (char*) malloc(dsize);

	aio_reader* reader = new aio_reader(dsize, _MAX);
	aio_writer* writer = new aio_writer(dsize, _MAX);

	if(writer->fopen("file.dat") < 0) printf("Failed to open file\n");

	for(int i = 0; i < 10*_MAX; i++) {
		sprintf(wbuffer, "This is line %d\n", i);
		if(writer->fwrite(wbuffer, dsize) != 0)
			printf("Failed to write at %d\n", i);
	}

	printf("Closing file\n");
	writer->fclose();

	if(reader->fopen("file.dat") < 0) printf("Failed to open file\n");
	printf("Reading full buffer: %d\n", reader->freadfullbuffer());

	for(int i = 0; i < 20*_MAX; i++) {
		memset(rbuffer, '\0', dsize);
		if(reader->fread(rbuffer, &rsize) > 0)
			printf("Read %s", rbuffer);
		if(reader->fetchnew() != 0)
			printf("Failed to cache new data\n");
	}

	printf("Closing file\n");
	reader->fclose();

	printf("Done\n");

	return 0;
}
