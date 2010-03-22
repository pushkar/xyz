/*
 * main.cpp
 *
 *  Created on: Mar 10, 2010
 *      Author: pushkar
 */

#include "view.h"
#include "main.h"
#include "../sensors/mesa.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

IplImage* dist_img_disp;
IplImage* ampl_img_disp;
IplImage* conf_img_disp;
IplImage* yz;
void imgs();

void _keyboard(unsigned char key) {
	switch (key) {
	case 'f':
		fetch_mesa_xyz_buffer();
		break;
	case 'g':
		fetch_mesa_xyz_buffer();
		break;
	}
	 srand ( time(NULL) );
}

void _draw() {
	mesa_update();
	imgs();
	draw_mesa_frame();
}

void imgs() {
	dist_img->imageData = (char*) srd_distbuf;
	ampl_img->imageData = (char*) srd_ampbuf;
	conf_img->imageData = (char*) srd_confbuf;
	for(int j = 0; j < srd_len; j++) {
		memcpy(dist_img_1->imageData+j, srd_distbuf+ j*2+1, 1);
		memcpy(ampl_img_1->imageData+j, srd_ampbuf+ j*2+1, 1);
		memcpy(conf_img_1->imageData+j, srd_confbuf+ j*2+1, 1);
	}

	cvResize(dist_img_1, dist_img_disp, CV_INTER_CUBIC);
	cvResize(ampl_img_1, ampl_img_disp, CV_INTER_CUBIC);
	cvResize(conf_img_1, conf_img_disp, CV_INTER_CUBIC);

	cvCalcOpticalFlowLK(dist_img_2, dist_img_1, cvSize(5, 5), velx, vely);
	vx = cvAvg(velx);
	vy = cvAvg(vely);
	fprintf(stderr, "Velocity is %.2f, %.2f\n", vx.val[0], vy.val[0]);

	cvDrawLine(dist_img_disp, cvPoint(dist_img_1->width/2, dist_img_1->height/2), cvPoint(dist_img_1->width/2+vx.val[0]*20, dist_img_1->height/2+vy	.val[0]*20), cvScalar(255, 255, 255, 255), 4);
	cvNamedWindow("Distance", 1); cvMoveWindow("Distance", 0, 0); cvShowImage("Distance", dist_img_disp);
	cvNamedWindow("Amplitude", 1); cvMoveWindow("Amplitude", 400, 0); cvShowImage("Amplitude", ampl_img_disp);
	cvNamedWindow("Confidence", 1); cvMoveWindow("Confidence", 800, 0); cvShowImage("Confidence", conf_img_disp);

	cvWaitKey(100);
	dist_img_2 = cvCloneImage(dist_img_1);
	conf_img_2 = cvCloneImage(conf_img_1);
	ampl_img_2 = cvCloneImage(ampl_img_1);
}

void draw_mesa_frame() {
	for (int i = 0; i < srd_len*2; ++i) {
		double len = srd_xbuf[i] * srd_xbuf[i] + srd_ybuf[i] * srd_ybuf[i]
				+ srd_zbuf[i] * srd_zbuf[i];
		if (len > 0.5 && len < 20.0) {
			float g = 0.0f;
			float r = 255 - (len / 24) * 255;
			float b = (len / 24) * 255;
			glColor3f(r, g, b);
			glBegin(GL_POINTS);
			glVertex3f(srd_zbuf[i], srd_xbuf[i], srd_ybuf[i]);
			glEnd();

			if(i % 2== 0) {
			glBegin(GL_LINES);
			glVertex3f(srd_zbuf[i], srd_xbuf[i], srd_ybuf[i]);
			float _x = float(rand() % 10 + 1)/1000.0f;
			float _y = float(rand() % 10 + 1)/1000.0f;
			float _z = float(rand() % 10 + 1)/1000.0f;
			glVertex3f(srd_zbuf[i]+0.01+_x, srd_xbuf[i]+0.00+_y, srd_ybuf[i]+0.00+_z);
			glEnd();
			}

		}
	}
}
