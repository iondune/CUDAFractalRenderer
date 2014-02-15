
CC=g++
CCFLAGS=-O3

INCLUDES=-I ~/484/ionEngine \
	-I ~/484/deps/stb_image/include

MCC=mpic++
MCCFLAGS=-O3

NCC=nvcc
NCCFLAGS=-arch sm_20 -I$(INCLUDES)

NMCC=nvcc
MPI_INCLUDES=/usr/include/openmpi-x86_64/
MPI_LIBS=/usr/lib64/openmpi/lib/
NMCCFLAGS=-arch sm_20 $(INCLUDES) -I$(MPI_INCLUDES) -L$(MPI_LIBS)

SRC=FractalRenderer


all: FractalMPI

FractalMPI: $(SRC)/LinuxMain.cpp $(SRC)/CudaFractalRenderer.cu $(SRC)/CudaFractalKernels.cu
	$(NMCC) $(NMCCFLAGS) -lmpi -o $@ $^

clean:
	rm -f FractalRenderer

remake: clean all
