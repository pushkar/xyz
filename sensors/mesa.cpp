#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <definesSR.h>
#include <libMesaSR.h>
#include <unistd.h>

#include "mesa.h"

static int opt_verbosity = 0;
static char opt_host[] = "192.168.1.30";

static SRCAM srd_srcam;
int srd_rows;
int srd_cols;
int srd_len;
unsigned int srd_buf_len;

float *srd_xbuf = NULL;
float *srd_ybuf = NULL;
float *srd_zbuf = NULL;
unsigned char* srd_distbuf = NULL;
unsigned char* srd_ampbuf = NULL;
unsigned char* srd_confbuf = NULL;

static FILE *srd_gnuplot = NULL;

/* ---------- */
/* Functions  */
/* ---------- */

void verbprintf(int level, const char fmt[], ...) {
	va_list argp;
	va_start(argp, fmt);
	if (level <= opt_verbosity) {
		fprintf(stderr, "srd: ");
		vfprintf(stderr, fmt, argp);
	}
	va_end(argp);
}

static const char *imgtype_string(ImgEntry::ImgType i) {
	switch (i) {
		case ImgEntry::IT_DISTANCE: return "DISTANCE";
		case ImgEntry::IT_AMPLITUDE: return "AMPLITUDE";
		case ImgEntry::IT_INTENSITY: return "INTENSITY";
		case ImgEntry::IT_TAP0: return "TAP0";
		case ImgEntry::IT_TAP1: return "TAP1";
		case ImgEntry::IT_TAP2: return "TAP2";
		case ImgEntry::IT_TAP3: return "TAP3";
		case ImgEntry::IT_SUM_DIFF: return "SUM_DIFF";
		case ImgEntry::IT_CONF_MAP: return "CONF_MAP";
		case ImgEntry::IT_UNDEFINED: return "UNDEFINED";
		case ImgEntry::IT_LAST: return "LAST";
	}
	return "Unknown";
}

static const char *datatype_string(ImgEntry::DataType i) {
	switch (i) {
		case ImgEntry::DT_NONE: return "NONE";
		case ImgEntry::DT_UCHAR: return "UCHAR";
		case ImgEntry::DT_CHAR: return "CHAR";
		case ImgEntry::DT_USHORT: return "USHORT";
		case ImgEntry::DT_SHORT: return "SHORT";
		case ImgEntry::DT_UINT: return "UINT";
		case ImgEntry::DT_INT: return "INT";
		case ImgEntry::DT_FLOAT: return "FLOAT";
		case ImgEntry::DT_DOUBLE: return "DOUBLE";
	}
	return "Unknown";
}

static const char *modfrq_to_string(ModulationFrq i) {
	switch (i) {
		case MF_40MHz: return "40MHz";
		case MF_30MHz: return "30MHz";
		case MF_21MHz: return "21MHz";
		case MF_20MHz: return "20MHz";
		case MF_19MHz: return "19MHz";
		case MF_60MHz: return "60MHz";
		case MF_15MHz: return "15MHz";
		case MF_10MHz: return "10MHz";
		case MF_29MHz: return "29MHz";
		case MF_31MHz: return "31MHz";
		case MF_14_5MHz: return "14_5MHz";
		case MF_15_5MHz: return "15_5MHz";
		case MF_LAST: return "LAST";
	}
	return "Unknown";
}

static int maybe_dump_mode(FILE *out, int printed, int mode, const char *str,
		int i) {
	// i really mean bitwise and
	if (i & mode) {
		fprintf(out, printed ? ", %s" : "%s", str);
		return 1;
	}
	return 0;
}

static void dump_modes(FILE *out, int i) {
	int printed = 0;
	printed |= maybe_dump_mode(out, printed, AM_COR_FIX_PTRN, "AM_COR_FIX_PTRN", i);
	printed |= maybe_dump_mode(out, printed, AM_MEDIAN, "AM_MEDIAN", i);
	printed |= maybe_dump_mode(out, printed, AM_TOGGLE_FRQ, "AM_TOGGLE_FRQ", i);
	printed |= maybe_dump_mode(out, printed, AM_CONV_GRAY, "AM_CONV_GRAY", i);
	printed |= maybe_dump_mode(out, printed, AM_SW_ANF, "AM_SW_ANF", i);
	//printed |= maybe_dump_mode( out, printed, AM_SR3K_2TAP_PROC, "AM_SR3K_2TAP_PROC", i );
	//printed |= maybe_dump_mode( out, printed, AM_SHORT_RANGE, "AM_SHORT_RANGE", i );
	printed |= maybe_dump_mode(out, printed, AM_CONF_MAP, "AM_CONF_MAP", i);
	printed |= maybe_dump_mode(out, printed, AM_HW_TRIGGER, "AM_HW_TRIGGER", i);
	printed |= maybe_dump_mode(out, printed, AM_SW_TRIGGER, "AM_SW_TRIGGER", i);
	printed |= maybe_dump_mode(out, printed, AM_DENOISE_ANF, "AM_DENOISE_ANF", i);
	printed |= maybe_dump_mode(out, printed, AM_MEDIANCROSS, "AM_MEDIANCROSS", i);
}

void mesa_plot() {
	if (NULL == srd_gnuplot) {

		srd_gnuplot = popen("gnuplot -persist", "w");
		assert(srd_gnuplot);

		fprintf(srd_gnuplot, "set xlabel 'X'\n");
		fprintf(srd_gnuplot, "set ylabel 'Y'\n");
		fprintf(srd_gnuplot, "set zlabel 'Z'\n");

		fprintf(srd_gnuplot, "set xrange [-2:2]\n");
		fprintf(srd_gnuplot, "set yrange [-2:2]\n");
		fprintf(srd_gnuplot, "set zrange [-2:2]\n");

		fprintf(srd_gnuplot, "set dgrid3d 50,50,2\n");
		fprintf(srd_gnuplot, "set contour\n");

		fprintf(srd_gnuplot, "set title 'Swiss Ranger Point Cloud'\n");
		fprintf(srd_gnuplot, "splot '-'\n");
	}
	else {
		assert(srd_gnuplot);
		fprintf(srd_gnuplot, "replot\n");
	}

	for (int i = 0; i < srd_len; i += 5) {
		fprintf(srd_gnuplot, "%f %f %f\n", srd_xbuf[i], srd_ybuf[i], srd_zbuf[i]);
	}

	fprintf(srd_gnuplot, "e\n");
}

/// Prints out info about the camera settings
void mesa_info() {
	assert(srd_srcam);
	// library version
	{
		unsigned short version[4];
		SR_GetVersion(version);
		fprintf(stderr, "Library Version: %d.%d.%d.%d\n", version[0], version[1], version[2], version[3]);
	}
	// device string
	{
		size_t len = 512;
		char buf[len];
		memset(buf, 0, len);
		SR_GetDeviceString(srd_srcam, buf, len);
		fprintf(stderr, "Device String: %s\n", buf);
	}
	// Acquire Mode
	{
		int mode = SR_GetMode(srd_srcam);
		fprintf(stderr, "Acquire Mode: 0x%x => ", mode);
		dump_modes(stderr, mode);
		putc('\n', stderr);
	}
	// dimensions
	fprintf(stderr, "Size (r x c): %d x %d\n", SR_GetRows(srd_srcam), SR_GetCols(srd_srcam));
	// image types
	{
		ImgEntry *imgent;
		int r = SR_GetImageList(srd_srcam, &imgent);
		fprintf(stderr, "Images available: %d\n", r);
		for (int i = 0; i < r; i++) {
			fprintf(stderr, "Image %d -- Type: %s, DataType: %s, W: %d, H: %d\n", i,
					imgtype_string(imgent[i].imgType),
					datatype_string(imgent[i].dataType),
					imgent[i].width,
					imgent[i].height);
		}
	}
	// ModFrq
	fprintf(stderr, "Modulation Frequency: %s\n", modfrq_to_string(
			SR_GetModulationFrequency(srd_srcam)));
	// integration time
	fprintf(stderr, "Integration time code: %d\n", SR_GetIntegrationTime(
			srd_srcam));

	// amplitude threshold
	fprintf(stderr, "Amplitude Threshold: %d\n", SR_GetAmplitudeThreshold(
			srd_srcam));

	// Distance Offset
	// Deprecated: Only for SR2
	// fprintf(stderr, "Distance Offset: %d\n", SR_GetDistanceOffset(srd_srcam));
}

void mesa_mem_init() {
	srd_rows = 144;
	srd_cols = 176;
	srd_len = srd_rows * srd_cols;
	srd_xbuf = (float*) malloc(sizeof(float) * srd_len);
	srd_ybuf = (float*) malloc(sizeof(float) * srd_len);
	srd_zbuf = (float*) malloc(sizeof(float) * srd_len);
	srd_ampbuf = (unsigned char*) malloc (sizeof(int)*2*srd_len);
	srd_confbuf = (unsigned char*) malloc (sizeof(int)*2*srd_len);
	srd_distbuf = (unsigned char*) malloc (sizeof(int)*2*srd_len);
	srd_buf_len = sizeof(unsigned char) * srd_len;
	if (NULL == srd_xbuf || NULL == srd_ybuf || NULL == srd_zbuf) {
		fprintf(stderr, "Couldn't malloc {x, y, z} buffer\n");
		exit(-1);
	}
}

/// Only opens the camera and allocates memory to global data structures
void mesa_init() {
	fprintf(stderr, "Opening Camera on %s\n", opt_host);
	// open camera
	{
		int r = SR_OpenETH(&srd_srcam, opt_host);
		if (r < 0) {
			fprintf(stderr, "Unable to open camera\n");
			exit(-1);
		}
	}

	// get dimensions
	{
		srd_rows = SR_GetRows(srd_srcam);
		srd_cols = SR_GetCols(srd_srcam);
		srd_len = srd_rows * srd_cols;
		srd_xbuf = (float*) malloc(sizeof(float) * srd_len);
		srd_ybuf = (float*) malloc(sizeof(float) * srd_len);
		srd_zbuf = (float*) malloc(sizeof(float) * srd_len);
		srd_buf_len = sizeof(unsigned char) * srd_len;
		if (NULL == srd_xbuf || NULL == srd_ybuf || NULL == srd_zbuf) {
			fprintf(stderr, "Couldn't malloc {x, y, z} buffer\n");
			mesa_finish();
			exit(-1);
		}
	}

#if 1
	int mode = SR_GetMode(srd_srcam);
	fprintf(stderr, "Setting Confidence Map capture... 0x%x\n", mode);
	SR_SetMode(srd_srcam, mode | AM_CONF_MAP);
#endif

#if 1
	mode = SR_GetMode(srd_srcam);
	fprintf(stderr, "Setting Short Range mode... 0x%x\n", mode);
	SR_SetMode(srd_srcam, mode | AM_RESERVED1);
#endif

#if 1
	int freq = SR_GetModulationFrequency(srd_srcam);
	fprintf(stderr, "Modulation Frequency is %d\n", freq);
	SR_SetModulationFrequency(srd_srcam, MF_30MHz);
	freq = SR_GetModulationFrequency(srd_srcam);
	fprintf(stderr, "Modulation Frequency is %d\n", freq);
#endif

#if 1
	int time = SR_GetIntegrationTime(srd_srcam);
	fprintf(stderr, "Integration Time is %d\n", time);
	SR_SetIntegrationTime(srd_srcam, 30);
	fprintf(stderr, "Integration Time is %d\n", time);
#endif
}

void mesa_update() {
	// get images
	{
		int r = SR_Acquire(srd_srcam);
		if (r < 0) {
			fprintf(stderr, "Error getting data from the camera: %d\n", r);
			mesa_finish();
			exit(1);
		}
	}
	// transform to cartesian coords
	{
		// assert that buffers point to something
		assert(srd_xbuf);
		assert(srd_ybuf);
		assert(srd_zbuf);
		// transform
		srd_distbuf = (unsigned char*) SR_GetImage(srd_srcam, 0);
		srd_ampbuf = (unsigned char*) SR_GetImage(srd_srcam, 1);
		srd_confbuf = (unsigned char*) SR_GetImage(srd_srcam, 2);
		assert(srd_distbuf != NULL);
		assert(srd_ampbuf != NULL);
		assert(srd_confbuf != NULL);
		SR_CoordTrfFlt(srd_srcam, srd_xbuf, srd_ybuf, srd_zbuf, sizeof(float), sizeof(float), sizeof(float));
	}
}

void mesa_finish() {
	// close camera
	int r = SR_Close(srd_srcam);
	switch (r) {
	case 0:
		verbprintf(1, "Closed camera\n");
		break;
	case -1:
		fprintf(stderr, "Error closing device: wrong device\n");
		break;
	case -2:
		fprintf(stderr, "Error closing device: can't release interface\n");
		break;
	case -3:
		fprintf(stderr, "Error closing device: can't close device\n");
		break;
	default:
		fprintf(stderr, "Error closing device: unknown error\n");
	}
	// free buffer
	if (NULL != srd_xbuf)
		free(srd_xbuf);
	if (NULL != srd_ybuf)
		free(srd_ybuf);
	if (NULL != srd_zbuf)
		free(srd_zbuf);

	// close gnuplot
	if (srd_gnuplot) {
		pclose(srd_gnuplot);
	}
}
