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
#include "flow.h"

using namespace std;

aio_reader* reader_p;
aio_reader* reader_i;
uint32_t dsize = 0, rsize = 0;

IplImage* dist_img;
IplImage* ampl_img;
IplImage* ampl_img_8;
IplImage* conf_img;
IplImage* eig;
IplImage* temp;
Flow* flow;
CvPoint2D32f corners[100];

void fetch_mesa_xyz_buffer() {
	if(reader_p->fread(srd_xbuf, &rsize) <= 0) printf("Error reading image with size %d\n", rsize);
	if(reader_p->fetchnew() != 0) printf("Failed to cache new data\n");

	if(reader_p->fread(srd_ybuf, &rsize) <= 0) printf("Error reading image with size %d\n", rsize);
	if(reader_p->fetchnew() != 0) printf("Failed to cache new data\n");

	if(reader_p->fread(srd_zbuf, &rsize) <= 0) printf("Error reading image with size %d\n", rsize);
	if(reader_p->fetchnew() != 0) printf("Failed to cache new data\n");
}

void fetch_mesa_img_buffer() {
	if(reader_i->fread(srd_distbuf, &rsize) <= 0) printf("Error reading image with size %d\n", rsize);
	if(reader_i->fetchnew() != 0) printf("Failed to cache new data\n");

	if(reader_i->fread(srd_ampbuf, &rsize) <= 0) printf("Error reading image with size %d\n", rsize);
	if(reader_i->fetchnew() != 0) printf("Failed to cache new data\n");

	if(reader_i->fread(srd_confbuf, &rsize) <= 0) printf("Error reading image with size %d\n", rsize);
	if(reader_i->fetchnew() != 0) printf("Failed to cache new data\n");
}

void write() {
	aio_writer* writer;
	aio_writer* fwriter;
	writer = new aio_writer(dsize, _MAX);
	fwriter = new aio_writer(dsize, _MAX);
	if(writer->fopen(_FILE_IMG_BUF) < 0) printf("Failed to open file\n");
	if(fwriter->fopen(_FILE_PNT_CLD) < 0) printf("Failed to open file\n");

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
		conf_img->imageData = (char*) srd_confbuf;
		dist_img->imageData = (char*) srd_distbuf;
		ampl_img->imageData = (char*) srd_ampbuf;
		cvNamedWindow("Distance", 1); cvMoveWindow("Distance", 0, 0); cvShowImage("Distance", dist_img);
		cvNamedWindow("Amplitude", 1); cvMoveWindow("Amplitude", 200, 200); cvShowImage("Amplitude", ampl_img);
		cvNamedWindow("Confidence", 1); cvMoveWindow("Confidence", 400, 400); cvShowImage("Confidence", conf_img);

		cvWaitKey(100);
		i++;
	}

	printf("Closing file\n");
	writer->fclose();
	fwriter->fclose();
}

void draw_img_frame() {
	fetch_mesa_img_buffer();
	conf_img->imageData = (char*) srd_confbuf;
	dist_img->imageData = (char*) srd_distbuf;
	ampl_img->imageData = (char*) srd_ampbuf;

	flow->new_flow(srd_distbuf, srd_ampbuf, srd_confbuf);
	flow->calculate_flow_hs();
	flow->calculate_flow_z();
	CvPoint3D32f v = flow->get_velocity();
	flow->end_flow();

	flow->draw(&dist_img);
	int corner_count = 100;
	ampl_img_8 = flow->get_ampl_8();
	cvGoodFeaturesToTrack(ampl_img_8, eig, temp, corners, &corner_count, 0.01, 50, NULL, 5, true);
	for(int i = 0; i < corner_count; i++) {
		cvDrawCircle(ampl_img_8, cvPoint(corners[i].x, corners[i].y), 4, cvScalar(255, 255, 255, 0), 2, 8, 0);
	}

	printf("Vel is %.2f, %.2f, %.2f, Corners: %d\n", v.x, v.y, v.z, corner_count	);

	cvNamedWindow("Distance", 1); cvMoveWindow("Distance", 0, 0); cvShowImage("Distance", dist_img);
	cvNamedWindow("Amplitude", 1); cvMoveWindow("Amplitude", 200, 0); cvShowImage("Amplitude", ampl_img_8);
	cvNamedWindow("Confidence", 1); cvMoveWindow("Confidence", 400, 0); cvShowImage("Confidence", conf_img);

	cvWaitKey(100);
}


int main(int argc, char* argv[]) {

	if(argc > 1) {
		printf("Functionality to use %s is not made yet.\n", argv[1]);
		// TODO: Use configuration file
	}

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
	eig = cvCreateImage(cvSize(srd_cols, srd_rows), IPL_DEPTH_32F, 1);
	temp = cvCreateImage(cvSize(srd_cols, srd_rows), IPL_DEPTH_32F, 1);
	dsize = srd_buf_len * 2;
	flow = new Flow();

	reader_p = new aio_reader(dsize, _MAX);
	reader_i = new aio_reader(dsize, _MAX);
	if(reader_p->fopen(_FILE_PNT_CLD) < 0) printf("Failed to open file\n");
	if(reader_i->fopen(_FILE_IMG_BUF) < 0) printf("Failed to open file\n");

	printf("Reading full buffer: %d\n", reader_p->freadfullbuffer());
	printf("Reading full buffer: %d\n", reader_i->freadfullbuffer());

	fetch_mesa_xyz_buffer();
	fetch_mesa_img_buffer();

	gl_init(argc, argv, "Mesa Viewer", 800, 600);
	glutMainLoop();

	printf("Closing file\n");
	reader_i->fclose();
	reader_p->fclose();

	if(mesa) mesa_finish();
	delete flow;
	return 0;
}
