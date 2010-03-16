/*
 * main.cpp
 *
 *  Created on: Feb 21, 2010
 *      Author: pushkar
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "aio_writer.h"
#include "aio_reader.h"

#include <opencv/cv.h>
#include <opencv/highgui.h>

#define _MAX 10

#if 1
int main() {
	uint32_t dsize = 0, rsize = 0;
	aio_writer* writer;
	char* wbuffer;
	char* rbuffer;

	IplImage* img;
	CvCapture* capture = cvCaptureFromCAM(0);
	if(!cvGrabFrame(capture)) {
		printf("Could not grab a frame\n\7");
		exit(0);
	}

	img = cvRetrieveFrame(capture);
	dsize = img->imageSize;
	wbuffer = (char*) malloc (dsize);
	writer = new aio_writer(dsize, _MAX);
	if(writer->fopen("file.dat") < 0) printf("Failed to open file\n");
	cvNamedWindow("Image", 1);

	int i = 0;
	while(cvGrabFrame(capture) && i < 15) {
		img = cvRetrieveFrame(capture);
		wbuffer = img->imageData;
		if(writer->fwrite(wbuffer, dsize) != 0)
			printf("Failed to write at %d\n", i);
		printf("Writing %d of size %d\n", i, dsize);
		cvShowImage("Image", img);
		cvWaitKey(100);
		i++;
	}

	printf("Closing file\n");
	writer->fclose();
	cvReleaseCapture(&capture);


	aio_reader* reader;
	rbuffer = (char*) malloc (dsize);
	reader = new aio_reader(dsize, _MAX);
	if(reader->fopen("file.dat") < 0) printf("Failed to open file\n");

	printf("Reading full buffer: %d\n", reader->freadfullbuffer());

	for(int i = 0; i < _MAX; i++) {
		if(reader->fread(rbuffer, &rsize) > 0)
			printf("Read image with size %d\n", rsize);
		img->imageData = rbuffer;
		cvShowImage("Image", img);
		cvWaitKey(100);

		if(reader->fetchnew() != 0)
			printf("Failed to cache new data\n");
	}

	printf("Closing file\n");
	reader->fclose();

	cvReleaseImage(&img);
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
