/*
 * flow.cpp
 *
 *  Created on: Mar 20, 2010
 *      Author: pushkar
 */

#include "flow.h"
#include "sensors/mesa.h"

double find_avg_over_neighbourhood(IplImage* img, CvPoint point, CvSize size) {
	double sum = 0.0f;
	int w = point.x - size.width/2;
	int h = point.y - size.height/2;
	if(w < 0) w = 2;
	if(h < 0) h = 2;
	if(w > img->width - size.width) w = w - size.width;
	if(h > img->height - size.height) h = h - size.height;
	for(int i = 0; i < size.width; i++) {
		for(int j = 0; j < size.height; j++) {
			sum += cvGet2D(img, w+i, h+j).val[0];
		}
	}
	return (sum/(size.width*size.height));
}

Flow::Flow() {
	dist_img_1 = cvCreateImage(cvSize(srd_cols, srd_rows), IPL_DEPTH_8U, 1);
	ampl_img_1 = cvCreateImage(cvSize(srd_cols, srd_rows), IPL_DEPTH_8U, 1);
	conf_img_1 = cvCreateImage(cvSize(srd_cols, srd_rows), IPL_DEPTH_8U, 1);
	dist_img_2 = cvCreateImage(cvSize(srd_cols, srd_rows), IPL_DEPTH_8U, 1);
	ampl_img_2 = cvCreateImage(cvSize(srd_cols, srd_rows), IPL_DEPTH_8U, 1);
	conf_img_2 = cvCreateImage(cvSize(srd_cols, srd_rows), IPL_DEPTH_8U, 1);
	velx = cvCreateImage(cvSize(srd_cols, srd_rows), IPL_DEPTH_32F, 1);
	vely = cvCreateImage(cvSize(srd_cols, srd_rows), IPL_DEPTH_32F, 1);
	velz = cvCreateImage(cvSize(srd_cols, srd_rows), IPL_DEPTH_32F, 1);
	win_s = cvSize(5, 5);
}

Flow::~Flow() {
	cvReleaseImage(&dist_img_1);
	cvReleaseImage(&dist_img_2);
	cvReleaseImage(&conf_img_1);
	cvReleaseImage(&conf_img_2);
	cvReleaseImage(&ampl_img_1);
	cvReleaseImage(&ampl_img_2);
	cvReleaseImage(&velx);
	cvReleaseImage(&vely);
	cvReleaseImage(&velz);
}

void Flow::new_flow(uchar* srd_distbuf, uchar* srd_ampbuf, uchar* srd_confbuf) {
	for(int j = 0; j < srd_len; j++) {
		memcpy(dist_img_1->imageData+j, srd_distbuf+ j*2+1, 1);
		memcpy(ampl_img_1->imageData+j, srd_ampbuf+ j*2+1, 1);
		memcpy(conf_img_1->imageData+j, srd_confbuf+ j*2+1, 1);
	}
}

void Flow::calculate_flow_hs() {
	cvCalcOpticalFlowHS(ampl_img_2, ampl_img_1, 1, velx, vely, 1, cvTermCriteria(1, 10, 0.5));
}

void Flow::calculate_flow_lk() {
	cvCalcOpticalFlowLK(ampl_img_2, ampl_img_1, cvSize(5, 5), velx, vely);
}

// TODO: Check in neighbourhood
void Flow::calculate_flow_z() {
	double val = 0.0f;
	for (int i = 0; i < dist_img_1->height; i += 10) {
		for (int j = 0; j < dist_img_1->width; j += 10) {
			vx = cvGet2D(velx, i, j);
			vy = cvGet2D(vely, i, j);
			val = cvGet2D(dist_img_2, i, j).val[0] - cvGet2D(dist_img_1, i, j).val[0];
			cvSet2D(velz, i, j, cvScalar(val, val, val, val));
		}
	}
}

void Flow::end_flow() {
	dist_img_2 = cvCloneImage(dist_img_1);
	conf_img_2 = cvCloneImage(conf_img_1);
	ampl_img_2 = cvCloneImage(ampl_img_1);

}

// TODO: Use RANSAC for estimation
CvPoint3D32f Flow::get_velocity() {
	v.x = cvAvg(velx).val[0];
	v.y = cvAvg(vely).val[0];
	v.z = cvAvg(velz).val[0];
	return v;
}

void Flow::draw(IplImage** img) {
	for (int i = 0; i < dist_img_1->height; i+=10) {
		for (int j = 0; j < dist_img_1->width; j+=10) {
			vx = cvGet2D(velx, i, j);
			vy = cvGet2D(vely, i, j);
			cvDrawLine(*img, cvPoint(j, i), cvPoint(j+vy.val[0], i+vx.val[0]),
					CV_RGB(255, 255, 255), 1);
		}
	}
}

