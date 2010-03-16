/*
 * main.cpp
 *
 *  Created on: Mar 10, 2010
 *      Author: pushkar
 */

#include "view.h"
#include "main.h"
#include "../sensors/mesa.h"

void _keyboard(unsigned char key) {
	switch (key) {
	case 'f':
		fetch_mesa_xyz_buffer();
		break;
	case 'g':
		fetch_mesa_xyz_buffer();
		break;
	}
}

void _draw() {
	draw_mesa_frame();
}

void draw_mesa_frame() {
	glBegin(GL_POINTS);
	for (int i = 0; i < srd_len; ++i) {
		double len = srd_xbuf[i] * srd_xbuf[i] + srd_ybuf[i] * srd_ybuf[i]
				+ srd_zbuf[i] * srd_zbuf[i];
		if (len > 0.01 && len < 24.0) {
			float g = 0.0f;
			float r = 255 - (len / 24) * 255;
			float b = (len / 24) * 255;
			glColor3f(r, g, b);
			glVertex3f(srd_zbuf[i], srd_xbuf[i], srd_ybuf[i]);
		}
	}
	glEnd();
}
