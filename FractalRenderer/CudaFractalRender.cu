
#include "CudaFractalRender.cuh"

#include "cuda_runtime.h"


__global__ void Kernel(u8 * Image, u32 const Width, u32 const Height)
{
	u32 x = (blockIdx.x) * blockDim.x + threadIdx.x;
	u32 y = (blockIdx.y) * blockDim.y + threadIdx.y;

	if (x >= Width || y >= Height)
		return;
	
	Image[y * Width * 3 + x * 3 + 0] = (u8) (x / (f32) Width * 255.f);
	Image[y * Width * 3 + x * 3 + 1] = (u8) (x / (f32) Width * 255.f);
	Image[y * Width * 3 + x * 3 + 2] = 0;
}

u8 const * CudaRenderFractal(u32 const Width, u32 const Height)
{
	u8 * HostImage = new u8[Width * Height * 3];
	u8 * DeviceImage; cudaMalloc((void**) & DeviceImage, Width * Height * 3 * sizeof(u8));
	
	u32 const BlockSize = 16;
	dim3 const Grid(
		Width / BlockSize + (Width % BlockSize ? 1 : 0), 
		Height / BlockSize + (Height % BlockSize ? 1 : 0));
	dim3 const Block(BlockSize, BlockSize);
	Kernel<<<Grid, Block>>>(DeviceImage, Width, Height);

	cudaMemcpy(HostImage, DeviceImage, Width * Height * 3 * sizeof(u8), cudaMemcpyDeviceToHost);
	return HostImage;
}
