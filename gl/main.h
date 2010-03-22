/*
 * main.h
 *
 *  Created on: Mar 10, 2010
 *      Author: pushkar
 *      Brief: User defined functions for a viewer
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv/cvaux.h>
#include <opencv/cxcore.h>
#include "../sensors/mesa.h"
#include "../gl/main.h"
#include "../gl/view.h"

extern CvScalar vx, vy, vz;
extern IplImage* dist_img; extern IplImage* dist_img_1; extern IplImage* dist_img_2;
extern IplImage* ampl_img; extern IplImage* ampl_img_1; extern IplImage* ampl_img_2;
extern IplImage* conf_img; extern IplImage* conf_img_1; extern IplImage* conf_img_2;
extern IplImage* velx; extern IplImage* vely;
extern IplImage* dist_img_disp;
extern IplImage* ampl_img_disp;
extern IplImage* conf_img_disp;
extern IplImage* yz;

void draw_mesa_frame();
void fetch_mesa_xyz_buffer();

void _keyboard(unsigned char key);
void _draw();

#endif /* MAIN_H_ */
