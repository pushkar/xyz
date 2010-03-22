/*
 * flow.h
 *
 *  Created on: Mar 20, 2010
 *      Author: pushkar
 */

#ifndef FLOW_H_
#define FLOW_H_

#include <opencv/cv.h>

class flow {
public:
	flow(IplImage* img);
	~flow();

	void insert_16(IplImage* img);
	void insert_8(IplImage* img);
};

#endif /* FLOW_H_ */
