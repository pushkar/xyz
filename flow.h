/*
 * flow.h
 *
 *  Created on: Mar 20, 2010
 *      Author: pushkar
 */

#ifndef FLOW_H_
#define FLOW_H_

#include <opencv/cv.h>
#include <opencv/highgui.h>

class Flow {
	IplImage* dist_img_1;  IplImage* dist_img_2;
	IplImage* ampl_img_1;  IplImage* ampl_img_2;
	IplImage* conf_img_1;  IplImage* conf_img_2;
	IplImage* velx;  IplImage* vely;  IplImage* velz;
	CvScalar vx, vy, vz;
	CvPoint3D32f v;

	// Settings
	CvSize win_s;
public:
	Flow();
	~Flow();

	void new_flow(uchar* srd_distbuf, uchar* srd_ampbuf, uchar* srd_confbuf);
	void calculate_flow_hs();
	void calculate_flow_lk();
	void calculate_flow_z();
	void end_flow();

	CvPoint3D32f get_velocity();
	void draw(IplImage** img);
	IplImage* get_ampl_8() { return cvCloneImage(ampl_img_1); }
	IplImage* get_dist_8() { return cvCloneImage(dist_img_1); }
	IplImage* get_conf_8() { return cvCloneImage(conf_img_1); }
};

#endif /* FLOW_H_ */
