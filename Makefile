FITSDIR 	= /usr
FITSINC 	= ${FITSDIR}/include/cfitsio	
FITSLIB		= -L${FITSDIR}/lib64 -lcfitsio

FFTW3DIR	= /usr
FFTW3INC	=
FFTW3LIB	= -L${FFTW3DIR}/lib64 -lfftw3 

NRC_DIR 	= /home/vlad/software.x32/NumRec/nrc
NRC_INC 	= $(NRC_DIR)/OTHER
NRC_LIB 	= 

CC      = gcc
CFLAGS  = -I$(FITSINC) -I${FFTW3INC} -Wall -pipe

.SUFFIXES: .cc .c

.cc.o:
	$(CC) $(CFLAGS) -c $<


PROGRAMS  = PlutoSpectrum
 
SRC     	= ExportToFITS.c receive.c get_parameters.c PlutoSpectrum.c
PSOBJS    	= ExportToFITS.o receive.o get_parameters.o PlutoSpectrum.o

all: $(PROGRAMS)

PlutoSpectrum: $(PSOBJS)
	$(CC) $(CFLAGS) $(PSOBJS) -o $@ $(FITSLIB) $(FFTW3LIB)-lm -liio

clean:
	rm -f *.o *.mod *.l *.anl *.m *~

cleanall:
	make clean
	rm -f ${PROGRAMS}
