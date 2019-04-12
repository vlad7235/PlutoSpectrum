#include <iio.h>

#ifndef PLUTOSPECTRUM_H
#define PLUTOSPECTRUM_H

#define NANO_PER_SEC 1000000000.0

struct  fits_keywords {
	float crval1;
	float cdelt1;
	float crpix1;
	char *cunit1;
	char *ctype1;
	float crval2;
	float cdelt2;
	float crpix2;
	float gain;
	float rssi_start;
	float rssi_end;
	float length;		
	char *cunit2;
	char *ctype2;
	char datestart[64];
	char timestart[64];
	char dateend[64];
	char timeend[64];
	char *instrument;
};

struct pluto_param {
	int bitrate;
	int scantime_sec;
	float start_f;
	float end_f; 
	float gain;
	int deviceinfo;
	char fitsfile[255];
	char asciifile[255];
};

typedef struct pluto_param Pluto_Param ;

// Get the parameters from a command line 
Pluto_Param get_options(int argc, char *argv[]);//, Pluto_Param *plp);

// Export to FITSfile
int get_filenames(int scantime_sec, char *fitsfile, char *asciifile);
int datetime_populate(char *datenow, char *timenow, char *message);
int export_image_to_fits_c(float *data, long *naxes, char * filename, struct fits_keywords fk);

// Receiver functions
int receive(struct iio_context *ctx);
int receive_swipe(struct iio_context *ctx, struct iio_device *phy, 
		long int lomin, long int lomax, long int rxbbrate, float *data, int npoints, int niter);

// Mean power spectrum
int mean_spectrum(float *data, long *naxes, double *pspectrum);
int export_pspectrum_to_ascii(double *pspectrum, int npoints, float fstart, float fdelt, char *filename);

#endif
