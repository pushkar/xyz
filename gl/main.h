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
#include "../flow.h"

#define READER1
#define _FILE_IMG_BUF "../logs/mesa_chess_img_rnd3_buf.dat"
#define _FILE_PNT_CLD "../logs/mesa_chess_pnt_rnd3_cld.dat"
#define _MAX 249
static int mesa = 0;

extern CvScalar vx, vy, vz;
extern IplImage* dist_img;
extern IplImage* ampl_img;
extern IplImage* conf_img;
extern Flow* flow;

void draw_img_frame();
void draw_mesa_frame();
void fetch_mesa_xyz_buffer();
void fetch_mesa_img_buffer();

void _keyboard(unsigned char key);
void _draw();


#endif /* MAIN_H_ */
