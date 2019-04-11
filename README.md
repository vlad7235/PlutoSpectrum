# PlutoSpectrum
Simple monitoring software for ADALM-Pluto

PlutoScan is a programm to make the wide-band scans up to 6 GHz.
It makes the swipe scans from start to end frequency with a resulution
determined by the bitrate parameter. In the end it writes the waterfall
data in a FITS file and a mean power spectrum in a ASCII file.
The file names are generated using a system date and time.
The options are:
```
root@laptop ./PlutoScan -h
	-h	Shows this help.
	-b	a bitrate in SPS, less than 62MSPS (the default is 5000000 SPS)
	-t	a monitoring time in sec (the default is 600 sec).
	  	During this time ADALM-Pluto will make as many swipes
	  	as possible, each swipe takes around 3 sec
	  	in the range 1-6GHz and with 5 MSPS bitrate.
	-s	Scan start frequency in Hz, from 70 MHz to 6 GHz (the default is 1.0e9 Hz).
	-e	Scan end frequency in Hz from 70 MHz to 6 GHz (the default is 3.0e9 Hz).
	-g	A hardware gain in dB (the default is 71 dB).
	-i	Shows information about the device, e.g. URI.
```

At each frequency the programm reads 4096 uncalibrated raw I and Q samples (2 x uint16_t), squares them and 
calculates a sum, which is proportional to the total power per frequency channel. 

The programm was tested in Fedora 27 and 29, it requires libiio and libad9361 installed, e.g.
```
sudo dnf install libiio* libad9361*
```

Here is and example of the waterfall plot produced from the output FITS file,
the monitorng was done from 500MHz (left) to 6GHz (right) during 900 seconds (in up ward direction):

![alt text](https://github.com/vlad7235/PlutoSpectrum/blob/master/figures/ds9.jpeg "Waterfall plot")

An example of the mean power spectrum:

![alt text][pspectrum]

[pspectrum]: https://github.com/vlad7235/PlutoSpectrum/blob/master/figures/mean_spectrum.png "Mean spectrum"
