#include <iio.h>
#include <stdio.h>
#include <time.h>
#include <fitsio.h>
#include <sys/time.h>
#include <string.h>
#include "PlutoSpectrum.h"

//#define NANO_PER_SEC 1000000000.0


int main (int argc, char *argv[])
{
	struct iio_context *ctx;
	struct iio_device *phy;
	struct iio_scan_context *ctx_scan;
	struct iio_context_info ***ctx_info;
	struct iio_channel * 	chn;
	ssize_t nctx;
	const char *buf, *uri;	
	char attrib[64];
	double hwgain, rssi;

	long int rxbbrate, lomin, lomax;
	struct timespec start, end;
     	double start_sec, end_sec, elapsed_sec;

	float *waterfall_data;
	double *pspectrum;
	long naxes[2];

	struct fits_keywords fk;
	int status, attr_num, k;

	Pluto_Param plp;

	plp = get_options(argc, argv);
/* Find USB URI for ADALM-Pluto */
/* ToDo: add other non-usb contexts discovery */
	ctx_scan = iio_create_scan_context("usb", 0);
	if(!(nctx = iio_scan_context_get_info_list(ctx_scan, ctx_info))){
		printf("No contexts are found, exiting...\n");
		return -1;
	}
	
	printf("Found %d context(s)\n", (int)nctx);
	buf = iio_context_info_get_description(ctx_info[0][0]);
	printf("Found %s\n", buf);
	uri = iio_context_info_get_uri(ctx_info[0][0]);
	printf("URI = %s\n", uri);
//	iio_context_info_list_free(ctx_info);
	iio_scan_context_destroy(ctx_scan);

/* Return if -i option */
	if(plp.deviceinfo) return 0;

	// PlutoScan parameters
//	printf("\nA list of the PlutoScan parameters in main():\n\n");
    	printf("Scan start frequency, Hz: %e\n", plp.start_f); 
    	printf("Scan end frequency, Hz: %e\n", plp.end_f); 
    	printf("Bitrate, SPS: %d\n", plp.bitrate); 
    	printf("Survey time, sec: %d\n", plp.scantime_sec); 
//    	printf("Device info: %s\n", (plp.deviceinfo)?("True"):("False")); 


/* Create a context */
	printf("Creating a context...\n");
//	ctx = iio_create_context_from_uri("ip:192.168.2.1");
	ctx = iio_create_context_from_uri(uri);
 
	phy = iio_context_find_device(ctx, "ad9361-phy");
	chn = iio_device_find_channel(phy, "voltage0", false);
//	attr_num = (int) iio_channel_get_attrs_count(chn);
//	printf("Number of the attributes is %d\n", attr_num);

/* Setting hardware gain */

	// Read hardware gain	
	buf = iio_channel_get_attr(chn, 3);
//	iio_channel_attr_read(chn, buf, attrib, 64);
	iio_channel_attr_read_double(chn, buf, &hwgain);
	printf("%s is %e dB\n", buf, hwgain);

	printf("Setting %s to %f dB...\n", buf, plp.gain);

	// Read gain control type
	buf = iio_channel_get_attr(chn, 0);
	iio_channel_attr_read(chn, buf, attrib, 64);
//	printf("%s is %s\n", buf, attrib);

	// Set gain control type to manual
	strcpy(attrib, "manual");
	status = iio_channel_attr_write(chn, buf, attrib);
	buf = iio_channel_get_attr(chn, 0);
	iio_channel_attr_read(chn, buf, attrib, 64);
	printf("%s is %s, status = %d\n", buf, attrib, status);

	// Read RSSI at start
	buf = iio_channel_get_attr(chn, 1);
	iio_channel_attr_read(chn, buf, attrib, 64);
	iio_channel_attr_read_double(chn, buf, &rssi);
	printf("Starting %s is %s / %e dB\n", buf, attrib, rssi);
	fk.rssi_start = (float)rssi;

	// Set hardware gain according to the -g option
	hwgain = (double)(plp.gain);
//	strcpy(attrib, "20.000000 dB");	
	
	buf = iio_channel_get_attr(chn, 3);
//	status = iio_channel_attr_write(chn, buf, attrib);
	status = iio_channel_attr_write_double(chn, buf, hwgain);
	iio_channel_attr_read(chn, buf, attrib, 64);
	iio_channel_attr_read_double(chn, buf, &hwgain);
	printf("%s is %s / %e dB, status = %d\n", buf, attrib, hwgain, status);
	fk.gain = (float)hwgain;

//	iio_context_destroy(ctx);
//	exit(0);

	rxbbrate = (long int)plp.bitrate;//5000000;
	lomin = (long int)plp.start_f;//1000000000;
	lomax = (long int)plp.end_f;//6000000000;
 
	iio_channel_attr_write_longlong(
		iio_device_find_channel(phy, "altvoltage0", true),
		"frequency",
		lomin); /* RX LO frequency lomin, Hz */
 

	iio_channel_attr_write_longlong(
		iio_device_find_channel(phy, "voltage0", false),
		"sampling_frequency",
		rxbbrate); /* RX baseband rate rxbbrate, SPS */


// Filling a structure with the keywords

/*
struct  fits_keywords 
	{float crval1;
	float cdelt1;
	float crpix1;
	char *cunit1;
	char *ctype1;
	float crval2;
	float cdelt2;
	float crpix2;		
	char *cunit2;
	char *ctype2;
	char *datestart;
	char *timestart;
	char *dateend;
	char *timeend;
	char *instrument;
};	
*/
	fk.crval1 = (float)lomin/1000000; //Starting frequency, in MHz
	fk.cdelt1 = (float)rxbbrate/1000000; //Step in frequency, in MHz
	fk.crpix1 = 1.0;
	fk.cunit1 = "MHz";
	fk.ctype1 = "FREQ";

	fk.crval2 = 0.0; //Starting time in sec
	fk.cdelt2 = 2.67; // Scan time, sec - an initial guess, to be owerwritten later
	fk.crpix2 = 1.0;
	fk.cunit2 = "Sec";
	fk.ctype2 = "TIME";

	fk.instrument = "ADALM-Pluto";


/********************** Calibration scans **************************/
	printf("Starting 4 calibration scans:\n");	

	naxes[0] = (lomax-lomin)/rxbbrate;	// Number of the frequency channels
	naxes[1] = 4; 				// Number of swipes, each swipe takes ~ 3 sec (1-6GHz, 5 MSPS rate)
	printf("%ld point in a swipe, %ld swipes\n", naxes[0], naxes[1]);	

	if(!(waterfall_data = calloc(naxes[0]*naxes[1],sizeof(float)))) {
		free(waterfall_data);
		return -1;
	}
	
	clock_gettime(CLOCK_REALTIME, &start);
	receive_swipe(ctx, phy, lomin, lomax, rxbbrate, waterfall_data, (int)naxes[0], (int)naxes[1]);
	clock_gettime(CLOCK_REALTIME, &end);
	start_sec = start.tv_sec + start.tv_nsec/NANO_PER_SEC;
	end_sec = end.tv_sec + end.tv_nsec/NANO_PER_SEC;
	elapsed_sec = end_sec - start_sec;
	printf("Swipe time for %d iterations from %ld Hz to %ld Hz is %e sec\n", (int)naxes[1], lomin, lomax, elapsed_sec);

	fk.cdelt2 = (float)elapsed_sec/(float)naxes[1]; // Estimated one swipe time, sec


/********************** Main scans **************************/
	naxes[1] = (int)((float)plp.scantime_sec/fk.cdelt2); // Estimated number of swipes in a required scantime
	if(naxes[1] > 0) 
		printf("It will be %d swipes in %d sec\n", naxes[1], plp.scantime_sec);
	else {
		printf("ERROR: one swipe time, %f sec is more than the total survey time, %d sec. Exiting...\n", fk.cdelt2, plp.scantime_sec);
		return -1;
	}
	// Allocate memory for the main survey scans
	free(waterfall_data);
	if(!(waterfall_data = calloc(naxes[0]*naxes[1],sizeof(float)))) {
		free(waterfall_data);
		return -1;
	}

	// Generate the file names wrt the current date/time
	status =  get_filenames(plp.scantime_sec, plp.fitsfile, plp.asciifile);
	printf("FITS  file: %s\n", plp.fitsfile);
	printf("ASCII file: %s\n", plp.asciifile);

	// Start date/time for a FITS header

	status = datetime_populate(fk.datestart, fk.timestart, "Starting a wideband scan: ");
	printf("%s %s\n",fk.datestart, fk.timestart );
	
	clock_gettime(CLOCK_REALTIME, &start);
	receive_swipe(ctx, phy, lomin, lomax, rxbbrate, waterfall_data, (int)naxes[0], (int)naxes[1]);
	clock_gettime(CLOCK_REALTIME, &end);
	start_sec = start.tv_sec + start.tv_nsec/NANO_PER_SEC;
	end_sec = end.tv_sec + end.tv_nsec/NANO_PER_SEC;
	elapsed_sec = end_sec - start_sec;
	printf("Swipe time for %d iterations from %ld Hz to %ld Hz is %e sec\n", (int)naxes[1], lomin, lomax, elapsed_sec);


	// End date/time for a FITS header
	status = datetime_populate(fk.dateend, fk.timeend, "Finishing a wideband scan: ");

	fk.cdelt2 = (float)elapsed_sec/(float)naxes[1]; // Actual one swipe time, sec
	fk.length = (float)elapsed_sec; // Total scan time, sec

	// Read RSSI at the end
	buf = iio_channel_get_attr(chn, 1);
	iio_channel_attr_read(chn, buf, attrib, 64);
	iio_channel_attr_read_double(chn, buf, &rssi);
	printf("Final %s is %s / %e dB\n", buf, attrib, rssi);
	fk.rssi_end = (float)rssi;


/************************************************************/ 
	iio_context_destroy(ctx);

/* Mean spectrum calculation */
	if(!(pspectrum = calloc(naxes[0],sizeof(double)))) {
		free(pspectrum);
		return -1;
	}

	status = mean_spectrum(waterfall_data, naxes, pspectrum);
/* ASCII file output */
	export_pspectrum_to_ascii(pspectrum, (int)naxes[0], fk.crval1, fk.cdelt1 , plp.asciifile);

/* FITS file output */
	export_image_to_fits_c(waterfall_data, naxes, plp.fitsfile, fk);

	free(waterfall_data);
	free(pspectrum);
 
	return 0;
} 
