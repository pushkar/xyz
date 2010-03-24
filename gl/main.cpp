/*
 * main.cpp
 *
 *  Created on: Mar 10, 2010
 *      Author: pushkar
 */

#include "view.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../sensors/mesa.h"

CvPoint3D32f kalman_v;
void _keyboard(unsigned char key) {
	switch (key) {
	case 'f':
		fetch_mesa_xyz_buffer();
		draw_img_frame();
		break;
	case 'g':
		fetch_mesa_xyz_buffer();
		draw_img_frame();
		break;
	}
	 srand ( time(NULL) );
}

void _draw() {
	if(mesa) {
		mesa_update();
	}
	else {
		fetch_mesa_xyz_buffer();
		fetch_mesa_img_buffer();
	}

	draw_img_frame();
	draw_mesa_frame();
	const CvMat* prediction = cvKalmanPredict(kalman, 0);
	kalman_v.x = prediction->data.fl[0];
	kalman_v.y = prediction->data.fl[1];
	kalman_v.z = prediction->data.fl[2];
	fprintf(stderr, "Kalman %.3f, %.3f, %.3f\n", kalman_v.x, kalman_v.y, kalman_v.z);
	CvPoint3D32f v = flow->get_velocity();
	measurement->data.fl[0] = v.x;
	measurement->data.fl[1] = v.y;
	measurement->data.fl[2] = v.z;
	cvMatMulAdd(kalman->measurement_matrix, state, measurement, measurement);
	cvKalmanCorrect(kalman, measurement);
	cvMatMulAdd(kalman->transition_matrix, state, process_noise, state);
}

void draw_mesa_frame() {
	int n = 0;
	for (int i = 0; i < srd_rows; i+=5) {
		for (int j = 0; j < srd_cols; j+=5) {
			n = i * srd_cols + j;
			double len = srd_xbuf[n] * srd_xbuf[n] + srd_ybuf[n] * srd_ybuf[n]
					+ srd_zbuf[n] * srd_zbuf[n];
			if (len > 0.5 && len < 20.0) {
				float g = 0.0f;
				float r = 255 - (len / 24) * 255;
				float b = (len / 24) * 255;
				glColor3f(r, g, b);
				glBegin(GL_POINTS);
				glVertex3f(srd_zbuf[n], srd_xbuf[n], srd_ybuf[n]);
				glEnd();
				CvPoint3D32f v = flow->get_velocity(cvPoint2D32f(i, j));
				v = kalman_v;
				double scale = 20.0f;
				glBegin(GL_LINES);
				glVertex3f(srd_zbuf[n], srd_xbuf[n], srd_ybuf[n]);
				glVertex3f(srd_zbuf[n]-v.z/scale, srd_xbuf[n]+v.x/scale, srd_ybuf[n]+v.y/scale);
				glEnd();
			}
		}
	}
}
