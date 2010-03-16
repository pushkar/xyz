/*
 * mesa.h
 *
 *  	Desc: API to the MESA Swiss Ranger 4000
 *      Author: Neil Dantam
 */
#ifndef MESA_H_
#define MESA_H_

extern int srd_rows;
extern int srd_cols;
extern int srd_len;
extern unsigned int srd_buf_len;

extern float *srd_xbuf;
extern float *srd_ybuf;
extern float *srd_zbuf;
extern unsigned char* srd_distbuf;
extern unsigned char* srd_ampbuf;
extern unsigned char* srd_confbuf;

#define SRD_LEN_DEFAULT 25344
#define SRD_ROWS_DEFAULT 144
#define SRD_COLS_DEFAULT 176

void mesa_init();
void mesa_finish();
void mesa_info();
void mesa_update();
void mesa_plot();

void mesa_mem_init();


#endif /* MESA_H_ */
