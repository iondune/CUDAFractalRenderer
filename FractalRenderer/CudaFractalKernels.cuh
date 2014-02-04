
#include "CudaFractalRenderer.cuh"


__global__ void InitKernel(SPixelState * States,  SFractalParams Params);
__global__ void HistogramKernel(SPixelState * States, u32 * Histogram, SFractalParams Params);
__global__ void DrawKernel(void * Image, SPixelState * States, u32 * Histogram, SFractalParams Params);
__global__ void FinalKernel(void * Image, SPixelState * States, u32 * Histogram, SFractalParams Params);
