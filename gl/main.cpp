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
#include <deque>
#include "../sensors/mesa.h"

std::deque<unsigned int> displaylist_mesa;
unsigned int displaylist_mesa_max = 50;

CvPoint3D32f kalman_v;
CvPoint3D32f kalman_lp;
std::vector<CvPoint3D32f> kalman_p;

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
	case 'r':
		kalman_p.clear();
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
	glPushMatrix();
	glTranslatef(0.0f, 0.0f, 0.0f);
	for (std::deque<unsigned int>::iterator i = displaylist_mesa.begin(); i != displaylist_mesa.end(); ++i) {
		glCallList(*i);
	}
	glPopMatrix();


	GLuint curlist = glGenLists(1);
	if (curlist != 0) {
		glNewList(curlist, GL_COMPILE);
		glPushMatrix();
		draw_mesa_frame();
		glPopMatrix();
		glEndList();
	}
	displaylist_mesa.push_back(curlist);
	while (displaylist_mesa.size() > displaylist_mesa_max) {
		glDeleteLists(displaylist_mesa.front(), 1);
		displaylist_mesa.pop_front();
	}

	const CvMat* prediction = cvKalmanPredict(kalman, 0);
	CvPoint3D32f p;
	p.x = prediction->data.fl[0];
	p.y = prediction->data.fl[1];
	p.z = prediction->data.fl[2];
	kalman_p.push_back(p);
	kalman_lp = p;
	kalman_v.x = prediction->data.fl[3];
	kalman_v.y = prediction->data.fl[4];
	kalman_v.z = prediction->data.fl[5];
	fprintf(stderr, "Kalman Pos %.3f, %.3f, %.3f\t\t", p.x, p.y, p.z);
	fprintf(stderr, "Kalman Vel %.3f, %.3f, %.3f\n", kalman_v.x, kalman_v.y, kalman_v.z);
	CvPoint3D32f v = flow->get_velocity();
	measurement->data.fl[0] = v.x;
	measurement->data.fl[1] = v.y;
	measurement->data.fl[2] = v.z;
	cvMatMulAdd(kalman->measurement_matrix, state, measurement, measurement);
	cvKalmanCorrect(kalman, measurement);
	cvMatMulAdd(kalman->transition_matrix, state, process_noise, state);

	for(uint i = 0; i < kalman_p.size()-1; i++) {
		CvPoint3D32f p1 = kalman_p[i];
		CvPoint3D32f p2 = kalman_p[i+1];
		glColor3f(255, 255, 255);
		glBegin(GL_LINES);
		glVertex3f(p1.z, p1.x, p1.y);
		glVertex3f(p2.z, p2.x, p2.y);
		glEnd();
	}

	for(uint i = 0; i < landmarks.size(); i++) {
		glPushMatrix();
		glTranslatef(landmarks[i].z, landmarks[i].x, landmarks[i].y);
		glutSolidSphere(0.005, 40, 40);
		glPopMatrix();
	}
}

void draw_mesa_frame() {
	int n = 0;
	//glTranslatef(kalman_lp.z, kalman_lp.x, kalman_lp.y);
	for (int i = 0; i < srd_rows; i+=2) {
		for (int j = 0; j < srd_cols; j+=2) {
			n = i * srd_cols + j;
			double len = srd_xbuf[n] * srd_xbuf[n] + srd_ybuf[n] * srd_ybuf[n]
					+ srd_zbuf[n] * srd_zbuf[n];
			if (len > 0.5 && len < 20.0) {
				float g = 0.0f;
				float r = 255 - (len / 24) * 255;
				float b = (len / 24) * 255;
				glColor3f(r, g, b);
				/*
				glBegin(GL_POINTS);
				glVertex3f(srd_zbuf[n], srd_xbuf[n], srd_ybuf[n]);
				glEnd();
				*/

				CvPoint3D32f v = flow->get_velocity(cvPoint2D32f(i, j));
				v = kalman_v;
				double scale = 500.0f;
				glBegin(GL_LINES);
				double x = srd_xbuf[n]+kalman_lp.x;
				double y = srd_ybuf[n]+kalman_lp.y;
				double z = srd_zbuf[n]+kalman_lp.z;
				glVertex3f(z, x, y);
				glVertex3f(z-v.z/scale, x+v.x/scale, y+v.y/scale);
				glEnd();

			}
		}
	}
}
