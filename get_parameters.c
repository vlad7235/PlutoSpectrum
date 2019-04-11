#include <stdio.h>  
#include <unistd.h> 
#include <stdlib.h> 
#include <time.h>       /* time_t, time, ctime */
#include <string.h>
#include <sys/time.h>
#include <assert.h>

#include "PlutoSpectrum.h"

Pluto_Param get_options(int argc, char *argv[])//, Pluto_Param *plp)  
{ 
    int opt; 
    int bitrate = 5000000; // Default bitrate
    int scantime_sec = 600; // sec, default length

    time_t rawtime;
    int deviceinfo = 0;
    float start_f = 1.0e9; // Hz, default start frequency
    float end_f   = 3.0e9; // Hz, default end frequency
    float gain    = 71.0; // dB, default hardware gain
    Pluto_Param plp;
      
    // put ':' in the starting of the 
    // string so that program can  
    //distinguish between '?' and ':'  
    while((opt = getopt(argc, argv, ":if:b:t:s:e:lhg:")) != -1)  
    {  
	switch(opt)  
        {  
            case 'i':  
		printf("Showing the device informaton:\n");
		deviceinfo = 1;
		break;
            case 'l':  
	    case 'h':
		printf("PlutoScan is a programm to make the wide-band scans up to 6 GHz.\n");
		printf("It makes the swipe scans from start to end frequency with a resulution\n");
		printf("determined by the bitrate parameter. In the end it writes the waterfall\n");
		printf("data in a FITS file and a mean power spectrum in a ASCII file.\n");
		printf("The file names are generated using a system date and time.\n");
		printf("The options are:\n");
		printf("\t-h\tShows this help.\n");
		printf("\t-b\ta bitrate in SPS, less than 62MSPS (the default is 5000000 SPS)\n");
		printf("\t-t\ta monitoring time in sec (the default is 600 sec).\n");
		printf("\t  \tDuring this time ADALM-Pluto will make as many swipes\n\t \tas possible, each swipe takes around 3 sec\n");
		printf("\t  \tin the range 1-6GHz and with 5 MSPS bitrate.\n");
		printf("\t-s\tScan start frequency in Hz, from 70 MHz to 6 GHz (the default is 1.0e9 Hz).\n");
		printf("\t-e\tScan end frequency in Hz from 70 MHz to 6 GHz (the default is 3.0e9 Hz).\n");
		printf("\t-i\tShows information about the device, e.g. URI.\n");
		exit(0);
            case 'g':  
 		gain = atof(optarg);
		printf("Hardware gain: %e dB\n", gain);  
		break;
             case 'f':  
                printf("filename: %s\n", optarg);  
                break;
	    case 'b':
		bitrate = atoi(optarg);
		printf("bitrate: %d\n", bitrate);  
		break;
	    case 't':
		scantime_sec = atoi(optarg);
		printf("Scan time, sec: %d\n", scantime_sec);  
		break;
	    case 's':
		start_f = atof(optarg);
		printf("Scan start frequency, Hz: %e\n", start_f);  
		break;
	    case 'e':
		end_f = atof(optarg);
		printf("Scan end frequency, Hz: %e\n", end_f);  
		break;
            case ':':  
                printf("option needs a value\n");  
                break;  
            case '?':  
                printf("unknown option: %c\n", optopt); 
                break;  
        }  
    }  
      
    // optind is for the extra arguments 
    // which are not parsed 
    for(; optind < argc; optind++){      
        printf("extra arguments: %s\n", argv[optind]);  
    } 
      

    assert(start_f < end_f);
    assert((start_f >= 7.0e7) && (start_f <= 6.0e9));
    assert((end_f   >= 7.0e7) && (end_f   <= 6.0e9));
    assert((bitrate > 0)      && (bitrate < 62000000));
    assert((scantime_sec > 0) && (scantime_sec <= 86400));
    assert((gain >= -3.0)      && (gain <= 100.0));

    // Copy parameters to the structure    
    strcpy(plp.fitsfile, "output.fits");
    strcpy(plp.asciifile, "output.dat");
    plp.bitrate 	= bitrate;
    plp.scantime_sec	= scantime_sec;
    plp.start_f		= start_f;
    plp.end_f		= end_f; 
    plp.gain		= gain; 
    plp.deviceinfo	= deviceinfo;

    return plp; 
}

int get_filenames(int scantime_sec, char *fitsfile, char *asciifile){

    time_t rawtime;
    char buffer [255], time_in_sec[64];
    int bl; // buffer length

    // Creating a filename with a system date/time
    time (&rawtime);
    sprintf(buffer,"PlutoScan_%s",ctime(&rawtime) );

    // Lets convert space to _ in
    char *p = buffer;
    for (; *p; ++p)
    {
       if (*p == ' ')
          *p = '_';
    }

    sprintf(time_in_sec, "%d",scantime_sec);
    strcat(time_in_sec, "sec.");

    // Replace the last char in buffer[] with '_'
    bl = strlen(buffer);
    buffer[bl-1] = '_';
    strcat(buffer, time_in_sec);
    strcpy(fitsfile, buffer);
    strcpy(asciifile, buffer);
    strcat(fitsfile, "fits");
    strcat(asciifile, "dat");
    
/*
    printf("FITS file name: %s\n",fitsfile);
    printf("ASCII file name: %s\n",asciifile);
    printf("Scan start frequency, Hz: %e\n", start_f); 
    printf("Scan end frequency, Hz: %e\n", end_f); 
*/    

    return 0;
}

int datetime_populate(char *datenow, char *timenow, char *message) {
	char            fmt[64], buft[64];
	char            fmtds[64], fmtts[64], fmtde[64], fmtte[64];
    	struct timeval  tv;
    	struct tm       *tm;

	// Start date/time for a FITS header
	gettimeofday(&tv, NULL);
    	if((tm = localtime(&tv.tv_sec)) != NULL)
    	{
            strftime(fmt, sizeof fmt, "%Y-%m-%d %H:%M:%S.%%06u %z", tm);
            snprintf(buft, sizeof buft, fmt, tv.tv_usec);
            printf("%s '%s'\n", message, buft); 
	    
	    strftime(fmtds, sizeof fmtds, "%Y-%m-%d", tm);  
	    snprintf(datenow,  sizeof buft, fmtds);
	    strftime(fmtts, sizeof fmtts, "%H:%M:%S", tm);  
	    snprintf(timenow,  sizeof buft, fmtts);
    	}

	return 0;
}
