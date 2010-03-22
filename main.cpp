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
#include <opencv/cvaux.h>
#include <opencv/cxcore.h>
#include "sensors/mesa.h"
#include "gl/main.h"
#include "gl/view.h"

#define READER2
#define _FILE_IMG_BUF "mesa_chess_img_rnd3_buf.dat"
#define _FILE_PNT_CLD "mesa_chess_pnt_rnd3_cld.dat"
#define _MAX 250
static int mesa = 0;
using namespace std;

aio_reader* reader;
uint32_t dsize = 0, rsize = 0;

IplImage* dist_img;  IplImage* dist_img_1;  IplImage* dist_img_2;
IplImage* ampl_img;  IplImage* ampl_img_1;  IplImage* ampl_img_2;
IplImage* conf_img;  IplImage* conf_img_1;  IplImage* conf_img_2;
IplImage* velx;  IplImage* vely;
CvScalar vx, vy, vz;

void fetch_mesa_xyz_buffer() {
	if(reader->fread(srd_xbuf, &rsize) <= 0) printf("Error reading image with size %d\n", rsize);
	if(reader->fetchnew() != 0) printf("Failed to cache new data\n");

	if(reader->fread(srd_ybuf, &rsize) <= 0) printf("Error reading image with size %d\n", rsize);
	if(reader->fetchnew() != 0) printf("Failed to cache new data\n");

	if(reader->fread(srd_zbuf, &rsize) <= 0) printf("Error reading image with size %d\n", rsize);
	if(reader->fetchnew() != 0) printf("Failed to cache new data\n");
}

void fetch_mesa_img_buffer() {
	if(reader->fread(srd_distbuf, &rsize) <= 0) printf("Error reading image with size %d\n", rsize);
	if(reader->fetchnew() != 0) printf("Failed to cache new data\n");

	if(reader->fread(srd_ampbuf, &rsize) <= 0) printf("Error reading image with size %d\n", rsize);
	if(reader->fetchnew() != 0) printf("Failed to cache new data\n");

	if(reader->fread(srd_confbuf, &rsize) <= 0) printf("Error reading image with size %d\n", rsize);
	if(reader->fetchnew() != 0) printf("Failed to cache new data\n");
}

int main(int argc, char* argv[]) {

	if(mesa) {
		mesa_init();
		mesa_info();
		mesa_update();
		fprintf(stderr, "Mesa Device Initialized\n");
	}
	else {
		mesa_mem_init();
		fprintf(stderr, "Mesa Memory Initialized\n");
	}
	dist_img = cvCreateImage(cvSize(srd_cols, srd_rows), IPL_DEPTH_16U, 1);
	ampl_img = cvCreateImage(cvSize(srd_cols, srd_rows), IPL_DEPTH_16U, 1);
	conf_img = cvCreateImage(cvSize(srd_cols, srd_rows), IPL_DEPTH_16U, 1);
	dist_img_1 = cvCreateImage(cvSize(srd_cols, srd_rows), IPL_DEPTH_8U, 1);
	ampl_img_1 = cvCreateImage(cvSize(srd_cols, srd_rows), IPL_DEPTH_8U, 1);
	conf_img_1 = cvCreateImage(cvSize(srd_cols, srd_rows), IPL_DEPTH_8U, 1);
	dist_img_2 = cvCreateImage(cvSize(srd_cols, srd_rows), IPL_DEPTH_8U, 1);
	ampl_img_2 = cvCreateImage(cvSize(srd_cols, srd_rows), IPL_DEPTH_8U, 1);
	conf_img_2 = cvCreateImage(cvSize(srd_cols, srd_rows), IPL_DEPTH_8U, 1);
	velx = cvCreateImage(cvSize(srd_cols, srd_rows), IPL_DEPTH_32F, 1);
	vely = cvCreateImage(cvSize(srd_cols, srd_rows), IPL_DEPTH_32F, 1);

	dist_img_disp = cvCreateImage(cvSize(srd_cols*2, srd_rows*2), IPL_DEPTH_8U, 1);
	ampl_img_disp = cvCreateImage(cvSize(srd_cols*2, srd_rows*2), IPL_DEPTH_8U, 1);
	conf_img_disp = cvCreateImage(cvSize(srd_cols*2, srd_rows*2), IPL_DEPTH_8U, 1);

	yz = cvCreateImage(cvSize(300, 200), IPL_DEPTH_8U, 1);

	dsize = srd_buf_len * 2;

#ifdef FILTERING
	int i = 0;
	while (i < 100) {
		mesa_update();
		dist_img->imageData = (char*) srd_distbuf;
		ampl_img->imageData = (char*) srd_ampbuf;
		conf_img->imageData = (char*) srd_confbuf;
		for(int j = 0; j < srd_len; j++) {
			memcpy(dist_img_1->imageData+j, srd_distbuf+ j*2+1, 1);
			memcpy(ampl_img_1->imageData+j, srd_ampbuf+ j*2+1, 1);
			memcpy(conf_img_1->imageData+j, srd_confbuf+ j*2+1, 1);
		}

		if(i > 2) {
			cvCalcOpticalFlowLK(dist_img_2, dist_img_1, cvSize(5, 5), velx, vely);
			vx = cvAvg(velx);
			vy = cvAvg(vely);
			fprintf(stderr, "Velocity is %.2f, %.2f\n", vx.val[0], vy.val[0]);
		}

		cvDrawLine(dist_img_1, cvPoint(dist_img_1->width/2, dist_img_1->height/2), cvPoint(dist_img_1->width/2+vx.val[0]*20, dist_img_1->height/2+vy	.val[0]*20), cvScalar(255, 255, 255, 255), 4);
		cvNamedWindow("Distance", 1); cvMoveWindow("Distance", 0, 0); cvShowImage("Distance", dist_img_1);
		cvNamedWindow("Amplitude", 1); cvMoveWindow("Amplitude", 200, 200); cvShowImage("Amplitude", ampl_img);
		cvNamedWindow("Confidence", 1); cvMoveWindow("Confidence", 400, 400); cvShowImage("Confidence", conf_img);

		cvWaitKey(100);
		dist_img_2 = cvCloneImage(dist_img_1);
		conf_img_2 = cvCloneImage(conf_img_1);
		ampl_img_2 = cvCloneImage(ampl_img_1);
 		i++;
	}

#endif

#ifdef WRITE1
	aio_writer* writer;
	aio_writer* fwriter;
	writer = new aio_writer(dsize, _MAX);
	fwriter = new aio_writer(dsize, _MAX);
	if(writer->fopen(_FILE_IMG_BUF) < 0) printf("Failed to open file\n");
	if(fwriter->fopen(_FILE_PNT_CLD) < 0) printf("Failed to open file\n");

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
		conf_img->imageData = (char*) srd_confbuf;
		dist_img->imageData = (char*) srd_distbuf;
		ampl_img->imageData = (char*) srd_ampbuf;
		conf_img->imageData = (char*) srd_confbuf;
		cvNamedWindow("Distance", 1); cvMoveWindow("Distance", 0, 0); cvShowImage("Distance", dist_img);
		cvNamedWindow("Amplitude", 1); cvMoveWindow("Amplitude", 200, 200); cvShowImage("Amplitude", ampl_img);
		cvNamedWindow("Confidence", 1); cvMoveWindow("Confidence", 400, 400); cvShowImage("Confidence", conf_img);

		cvWaitKey(100);
		i++;
	}

	printf("Closing file\n");
	writer->fclose();
	fwriter->fclose();
#endif
#ifdef READER1
	reader = new aio_reader(dsize, _MAX);
	if(reader->fopen(_FILE_PNT_CLD) < 0) printf("Failed to open file\n");

	printf("Reading full buffer: %d\n", reader->freadfullbuffer());

	fetch_mesa_xyz_buffer();

	gl_init(argc, argv, "Mesa Viewer", 800, 600);
	glutMainLoop();

	printf("Closing file\n");
	reader->fclose();
#endif
#ifdef READER2
	reader = new aio_reader(dsize, _MAX);
	if(reader->fopen(_FILE_IMG_BUF) < 0) printf("Failed to open file\n");

	printf("Reading full buffer: %d\n", reader->freadfullbuffer());
	while(1) {
		fetch_mesa_img_buffer();
		conf_img->imageData = (char*) srd_confbuf;
		dist_img->imageData = (char*) srd_distbuf;
		ampl_img->imageData = (char*) srd_ampbuf;

		for(int j = 0; j < srd_len; j++) {
			memcpy(dist_img_1->imageData+j, srd_distbuf+ j*2+1, 1);
			memcpy(ampl_img_1->imageData+j, srd_ampbuf+ j*2+1, 1);
			memcpy(conf_img_1->imageData+j, srd_confbuf+ j*2+1, 1);
		}

		cvCalcOpticalFlowLK(ampl_img_2, ampl_img_1, cvSize(5, 5), velx, vely);
		vx = cvAvg(velx);
		vy = cvAvg(vely);

		cvDrawLine(dist_img, cvPoint(dist_img->width/2, dist_img->height/2), cvPoint(dist_img->width/2+vx.val[0]*20, dist_img->height/2+vy.val[0]*20), cvScalar(255, 255, 255, 255), 4);

		cvNamedWindow("Distance", 1); cvMoveWindow("Distance", 0, 0); cvShowImage("Distance", dist_img);
		cvNamedWindow("Amplitude", 1); cvMoveWindow("Amplitude", 200, 0); cvShowImage("Amplitude", ampl_img);
		cvNamedWindow("Confidence", 1); cvMoveWindow("Confidence", 400, 0); cvShowImage("Confidence", conf_img);

		cvWaitKey(100);
		dist_img_2 = cvCloneImage(dist_img_1);
		conf_img_2 = cvCloneImage(conf_img_1);
		ampl_img_2 = cvCloneImage(ampl_img_1);
	}

#endif

	if(mesa) mesa_finish();
	return 0;
}
