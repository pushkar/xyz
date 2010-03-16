/*
 * aio_ds.h
 *
 *  Created on: Feb 25, 2010
 *      Author: pushkar
 */

#ifndef AIO_DS_H_
#define AIO_DS_H_

#include <stdint.h>

struct aiobf {
	uint64_t valid;
	uint64_t serial;
	uint64_t size;
};

#endif /* AIO_DS_H_ */
