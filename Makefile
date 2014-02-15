
CC=g++
CCFLAGS=-O3

INCLUDES=-I ~/484/ionEngine \
	-I ~/484/deps/stb_image/include

MCC=mpic++
MCCFLAGS=-O3

NCC=nvcc
NCCFLAGS=-arch sm_20 $(INCLUDES)

NMCC=nvcc
MPI_INCLUDES=/usr/include/openmpi-x86_64/
MPI_LIBS=/usr/lib64/openmpi/lib/
NMCCFLAGS=-arch sm_20 $(INCLUDES) -I$(MPI_INCLUDES) -L$(MPI_LIBS) -lmpi -lmpi_cxx

SRC=FractalRenderer


all: FractalMPI

FractalMPI: $(SRC)/LinuxMain.cpp $(SRC)/CudaFractalRenderer.cu $(SRC)/CudaFractalKernels.cu
	$(NMCC) $(NMCCFLAGS) -o $@ $^

clean:
	rm -f FractalMPI Image*.png

remake: clean all
