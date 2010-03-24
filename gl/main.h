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
#include <iostream>
#include <vector>
#include "../sensors/mesa.h"
#include "../gl/main.h"
#include "../gl/view.h"
#include "../flow.h"

#define _FILE_IMG_BUF "../logs/rnd4_img.dat"
#define _FILE_PNT_CLD "../logs/rnd4_cld.dat"
#define _MAX 249
extern int mesa;

extern CvScalar vx, vy, vz;
extern IplImage* dist_img;
extern IplImage* ampl_img;
extern IplImage* conf_img;
extern Flow* flow;
extern CvKalman* kalman;
extern CvMat* state;
extern CvMat* measurement;
extern CvMat* process_noise;
extern CvPoint3D32f kalman_v;
extern CvPoint3D32f kalman_lp;
extern std::vector<CvPoint3D32f> kalman_p;
extern std::vector<CvPoint3D32f> landmarks;

void draw_img_frame();
void draw_mesa_frame();
void fetch_mesa_xyz_buffer();
void fetch_mesa_img_buffer();

void _keyboard(unsigned char key);
void _draw();


#endif /* MAIN_H_ */
