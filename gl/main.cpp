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
	if(mesa) mesa_update();
	else fetch_mesa_xyz_buffer();
	draw_mesa_frame();
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
		}
	}
}
