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

	{
		fetch_mesa_xyz_buffer();
		fetch_mesa_img_buffer();
	}


	draw_img_frame();
	draw_mesa_frame();
}

void draw_mesa_frame() {
	int n = 0;
	CvPoint3D32f v;
	for (int i = 0; i < srd_rows; i+=1) {
		for (int j = 0; j < srd_cols; j+=1) {
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
				v = flow->get_velocity(cvPoint2D32f(i, j));
				glBegin(GL_LINES);
				glVertex3f(srd_zbuf[n], srd_xbuf[n], srd_ybuf[n]);
				glVertex3f(srd_zbuf[n]+v.z/1000.0f, srd_xbuf[n]+v.x/1000.0f, srd_ybuf[n]+v.y/1000.0f);
				glEnd();
			}
		}
	}
}
