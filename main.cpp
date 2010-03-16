/*
 * main.cpp
 *
 *  Created on: Feb 21, 2010
 *      Author: pushkar
 */

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "aio_writer.h"
#include "aio_reader.h"

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "sensors/mesa.h"
#include "gl/main.h"
#include "gl/view.h"

#define _MAX 250
static int mesa = 0;
using namespace std;

#if 1
aio_reader* reader;
uint32_t dsize = 0, rsize = 0;

void fetch_mesa_xyz_buffer() {
	if(reader->fread(srd_xbuf, &rsize) > 0) printf("Read image with size %d\n", rsize);
	if(reader->fetchnew() != 0) printf("Failed to cache new data\n");

	if(reader->fread(srd_ybuf, &rsize) > 0) printf("Read image with size %d\n", rsize);
	if(reader->fetchnew() != 0) printf("Failed to cache new data\n");

	if(reader->fread(srd_zbuf, &rsize) > 0) printf("Read image with size %d\n", rsize);
	if(reader->fetchnew() != 0) printf("Failed to cache new data\n");
}

int main(int argc, char* argv[]) {

	if(mesa) {
		mesa_init();
		mesa_info();
		mesa_update();
	}
	else
		mesa_mem_init();
	IplImage* dist_img = 0;
	IplImage* ampl_img = 0;
	IplImage* conf_img = 0;
	dist_img = cvCreateImage(cvSize(srd_cols, srd_rows), 16, 1);
	ampl_img = cvCreateImage(cvSize(srd_cols, srd_rows), 16, 1);
	conf_img = cvCreateImage(cvSize(srd_cols, srd_rows), 16, 1);

	dsize = srd_buf_len * 2;
#ifdef WRITE
	aio_writer* writer;
	aio_writer* fwriter;
	writer = new aio_writer(dsize, _MAX);
	fwriter = new aio_writer(dsize, _MAX);
	if(writer->fopen("mesa_image_buf.dat") < 0) printf("Failed to open file\n");
	if(fwriter->fopen("mesa_point_cld.dat") < 0) printf("Failed to open file\n");

	int i = 0;
	while(i < 2*_MAX) {
		mesa_update();
		if(writer->fwrite((char*) srd_distbuf, dsize) != 0)	printf("Failed to write at %d\n", i);
		if(writer->fwrite((char*) srd_ampbuf, dsize) != 0)	printf("Failed to write at %d\n", i);
		if(writer->fwrite((char*) srd_confbuf, dsize) != 0) printf("Failed to write at %d\n", i);
		if(fwriter->fwrite((char*) srd_xbuf, dsize) != 0) printf("Failed to write at %d\n", i);
		if(fwriter->fwrite((char*) srd_ybuf, dsize) != 0) printf("Failed to write at %d\n", i);
		if(fwriter->fwrite((char*) srd_zbuf, dsize) != 0) printf("Failed to write at %d\n", i);
		printf("Writing %d of size %d\n", i, dsize);
		cerr << i << endl;
		i++;
	}

	printf("Closing file\n");
	writer->fclose();
	fwriter->fclose();
#endif

	reader = new aio_reader(dsize, _MAX);
	if(reader->fopen("mesa_point_cld.dat") < 0) printf("Failed to open file\n");

	printf("Reading full buffer: %d\n", reader->freadfullbuffer());

	fetch_mesa_xyz_buffer();

	gl_init(argc, argv, "Mesa Viewer", 800, 600);
	glutMainLoop();

	if(mesa) mesa_finish();
	printf("Closing file\n");
	reader->fclose();

	return 0;
}

#else
int main() {
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
#endif
