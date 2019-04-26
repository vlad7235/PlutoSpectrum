#include <iio.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "PlutoSpectrum.h"

int receive(struct iio_context *ctx)
{
	struct iio_device *dev;
	struct iio_channel *rx0_i, *rx0_q;
	struct iio_buffer *rxbuf;
 
	dev = iio_context_find_device(ctx, "cf-ad9361-lpc");
 
	rx0_i = iio_device_find_channel(dev, "voltage0", 0);
	rx0_q = iio_device_find_channel(dev, "voltage1", 0);
 
	iio_channel_enable(rx0_i);
	iio_channel_enable(rx0_q);
 
	rxbuf = iio_device_create_buffer(dev, 4096, false);
	if (!rxbuf) {
		perror("Could not create RX buffer");
	/* Cleaning up and returning */
	//	shutdown();
		printf("* Disabling streaming channels\n");
		if (rx0_i) { iio_channel_disable(rx0_i); }
    		if (rx0_q) { iio_channel_disable(rx0_q); }

    		printf("* Destroying context\n");
    		if (ctx) { iio_context_destroy(ctx); }
    		exit(0);
	}
 
	while (true) {
		void *p_dat, *p_end, *t_dat;
		ptrdiff_t p_inc;
 
		iio_buffer_refill(rxbuf);
 
		p_inc = iio_buffer_step(rxbuf);
		p_end = iio_buffer_end(rxbuf);
 
		for (p_dat = iio_buffer_first(rxbuf, rx0_i); p_dat < p_end; p_dat += p_inc, t_dat += p_inc) {
			const int16_t i = ((int16_t*)p_dat)[0]; // Real (I)
			const int16_t q = ((int16_t*)p_dat)[1]; // Imag (Q)
 
			/* Process here */
 
		}
	}
 
	iio_buffer_destroy(rxbuf);
 
}

int receive_swipe(struct iio_context *ctx, struct iio_device *phy, long int lomin, long int lomax, long int rxbbrate, float *data, int npoints, int niter)
{
	struct iio_device *dev;
	struct iio_channel *rx0_i, *rx0_q;
	struct iio_buffer *rxbuf;
        struct iio_channel * 	chn;
	
	long int rxlo_tmp = lomin;
        int status;
	float signal_sum_tmp;

	struct timespec start, end;
     	double start_sec, end_sec, elapsed_sec;
 
	dev = iio_context_find_device(ctx, "cf-ad9361-lpc");
 	chn = iio_device_find_channel(phy, "altvoltage0", true);
	rx0_i = iio_device_find_channel(dev, "voltage0", 0);
	rx0_q = iio_device_find_channel(dev, "voltage1", 0);
 
	iio_channel_enable(rx0_i);
	iio_channel_enable(rx0_q);
 
	rxbuf = iio_device_create_buffer(dev, 4096, false);
	if (!rxbuf) {
		perror("Could not create RX buffer");
	/* Cleaning up and returning */
		printf("* Disabling streaming channels\n");
		if (rx0_i) { iio_channel_disable(rx0_i); }
    		if (rx0_q) { iio_channel_disable(rx0_q); }

    		printf("* Destroying context\n");
    		if (ctx) { iio_context_destroy(ctx); }
    		exit(0);
	}
 	
	for(int i=0;i<niter;++i){
	  printf("Iter %d of %d\n", i+1, niter);
	  rxlo_tmp = lomin;
	  clock_gettime(CLOCK_REALTIME, &start);
	  int j = 0;
	  while (rxlo_tmp <= lomax) {
		void *p_dat, *p_end, *t_dat;
		ptrdiff_t p_inc;
 
		iio_buffer_refill(rxbuf);
 
		p_inc = iio_buffer_step(rxbuf);
		p_end = iio_buffer_end(rxbuf);

		status = -1;
		while(status < 0) { 
			status = iio_channel_attr_write_longlong(chn,
		//iio_device_find_channel(phy, "altvoltage0", true),
			"frequency", rxlo_tmp);} /* RX LO frequency rxlo_tmp, Hz */

 		signal_sum_tmp = 0.0;
		for (p_dat = iio_buffer_first(rxbuf, rx0_i); p_dat < p_end; p_dat += p_inc, t_dat += p_inc) {
			const int16_t i = ((int16_t*)p_dat)[0]; // Real (I)
			const int16_t q = ((int16_t*)p_dat)[1]; // Imag (Q)
 
			/* Process here */
 			signal_sum_tmp += (float)((int)i*(int)i + (int)q*(int)q);
		}
		data[i*npoints + j] = signal_sum_tmp;
//		printf("Iter = %d, Freq = %ld Hz, Sum = %e\n", i, rxlo_tmp, signal_sum_tmp);
		rxlo_tmp += rxbbrate;
		j++;
	  }
	  clock_gettime(CLOCK_REALTIME, &end);
	  start_sec = start.tv_sec + start.tv_nsec/NANO_PER_SEC;
	  end_sec = end.tv_sec + end.tv_nsec/NANO_PER_SEC;
	  elapsed_sec = end_sec - start_sec;
	  printf("Swipe time from %ld Hz to %ld Hz is %e sec\n", lomin, lomax, elapsed_sec);
 	}
	iio_buffer_destroy(rxbuf);
	
 
}


int mean_spectrum(float *data, long *naxes, double *pspectrum){
	
	for(int j=0;j<(int)naxes[0];++j){
		pspectrum[j] = 0.0;	
		for(int i=0;i<(int)naxes[1];++i){
			pspectrum[j] += (double)data[i*(int)naxes[0] + j];
		}	
		pspectrum[j] /= (double)naxes[1];	
	}

	return 0;
}

