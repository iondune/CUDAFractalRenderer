
#include "CudaFractalRender.cuh"
#include "cuda_runtime.h"


__global__ void Kernel(u8 * Image, u32 const Width, u32 const Height)
{
	f32 const AspectRatio = (f32) Width / (f32) Height;
	f64 const sX(AspectRatio * 3.0), sY(3.0), cX(-0.5), cY(0);
	u32 const max_iteration = 1000;

	u32 posX = (blockIdx.x) * blockDim.x + threadIdx.x;
	u32 posY = (blockIdx.y) * blockDim.y + threadIdx.y;

	if (posX >= Width || posY >= Height)
		return;

	int iteration = 0;

	f64 x0 = posX / (f64) Width;
	f64 y0 = posY / (f64) Height;
	x0 -= 0.5;
	y0 -= 0.5;
	x0 *= sX;
	y0 *= sY;
	x0 += cX;
	y0 += cY;

	f64 x = 0.0, y = 0.0;
	while (x*x + y*y < 4.0 && iteration < max_iteration)
	{
		f64 xtemp = x*x - y*y + x0;
		y = 2.0*x*y + y0;

		x = xtemp;
		++ iteration;
	}

	Image[posY * Width * 3 + posX * 3 + 0] = (u8) (iteration / (f32) max_iteration * 255.f);
	Image[posY * Width * 3 + posX * 3 + 1] = (u8) (iteration / (f32) max_iteration * 255.f);
	Image[posY * Width * 3 + posX * 3 + 2] = 50;
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
