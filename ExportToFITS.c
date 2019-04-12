#include <fitsio.h>
#include "PlutoSpectrum.h"
#include <stdio.h>

int export_image_to_fits_c(float *data, long *naxes, char * filename, struct fits_keywords fk)
{
	int status = 0, exists;
	fitsfile *fptr;       /* pointer to the FITS file; defined in fitsio.h */
	long  fpixel = 1, naxis = 2, nelements;

	printf("Creating a FITS-file %s\n", filename);
	printf("Image size is %ld x %ld\n", naxes[0], naxes[1]);
	fits_file_exists(filename, &exists, &status); /* check if the file exists */

	if(exists != 0) {
		fits_open_file(&fptr, filename, READWRITE, &status); /* open existed file */
		fits_delete_file(fptr, &status);
	}
//	else {
//	Rewrite the old file if it is leading "!" in a filename
	fits_create_file(&fptr, filename, &status);   /* create new file */
//	}

	/* Create the primary array image  */
	fits_create_img(fptr, FLOAT_IMG, naxis, naxes, &status);
	nelements = naxes[0] * naxes[1];         /* number of pixels to write */
	/* Write the array of integers to the image */
	fits_write_img(fptr, TFLOAT, fpixel, nelements, data, &status);
	/* Write keywords */
	fits_write_key(fptr, TSTRING, "BUNIT", "ADC_SCORE^2", "Units of the data", &status);

	fits_write_key(fptr, TFLOAT,  "CRVAL1", &fk.crval1, "[MHz] frequency value at reference point", &status);
	fits_write_key(fptr, TFLOAT,  "CDELT1", &fk.cdelt1, "[MHz] frequency increment at reference point", &status);
	fits_write_key(fptr, TFLOAT,  "CRPIX1", &fk.crpix1, "Pixel coordinate of reference point", &status);
	fits_write_key(fptr, TSTRING, "CUNIT1", fk.cunit1, "Units of frequency increment and value", &status);
	fits_write_key(fptr, TSTRING, "CTYPE1", fk.ctype1, "Frequency (linear)", &status);

	fits_write_key(fptr, TFLOAT,  "CRVAL2", &fk.crval2, "[sec] time value at reference point", &status);
	fits_write_key(fptr, TFLOAT,  "CDELT2", &fk.cdelt2, "[sec] time increment at reference point", &status);
	fits_write_key(fptr, TFLOAT,  "CRPIX2", &fk.crpix2, "Pixel coordinate of reference point", &status);
	fits_write_key(fptr, TSTRING, "CUNIT2", fk.cunit2, "Units of time increment and value", &status);
	fits_write_key(fptr, TSTRING, "CTYPE2", fk.ctype2, "Time (linear)", &status);

	fits_write_key(fptr, TSTRING, "DATE-OBS", fk.datestart, "date of observation start (dd/mm/yy) ", &status);
	fits_write_key(fptr, TSTRING, "TIME-OBS", fk.timestart, "time of observation start (hh:mm:ss) ", &status);
	fits_write_key(fptr, TSTRING, "DATE-END", fk.dateend, "date of observation end (dd/mm/yy) ", &status);
	fits_write_key(fptr, TSTRING, "TIME-END", fk.timeend, "time of observation end (hh:mm:ss) ", &status);
	fits_write_key(fptr, TFLOAT,  "LENGTH",   &fk.length, "[sec] total scan length from start to end", &status);

	fits_write_key(fptr, TSTRING, "INSTRUME", fk.instrument, "instrument name ", &status);

	fits_write_key(fptr, TFLOAT,  "HWGAIN",   &fk.gain, "[dB] hardware gain parameter", &status);
	fits_write_key(fptr, TFLOAT,  "RSSI",     &fk.rssi_start, "[dB] RSSI at start", &status);
	fits_write_key(fptr, TFLOAT,  "RSSI-END", &fk.rssi_end, "[dB] RSSI at the end", &status);

	fits_close_file(fptr, &status);            /* close the file */
	fits_report_error(stderr, status);  /* print out any error messages */
	return status;
}


int export_pspectrum_to_ascii(double *pspectrum, int npoints, float fstart, float fdelt, char *filename){

	FILE *fp;
	float freq;
		
	if ( (fp = fopen(filename, "w") ) == NULL )
  	{
     		printf("ERROR: can't create file %s\n", filename);
     		exit(-1);
  	}
	printf("Writing power spectrum into %s\n", filename);
	fprintf(fp, "#Frequency, MHz\tPower\n#\n");
	for (int i = 0;i < npoints; ++i){
		freq = fstart + i*fdelt;
		fprintf(fp, "%e %e\n", freq, (float)pspectrum[i]);
	}
	
	fclose(fp);

	
	return 0;
}

