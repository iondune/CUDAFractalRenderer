
#include "CudaFractalRender.cuh"
#include "cuda_runtime.h"

#include <ionCore/ionUtils.h>


__global__ void HistogramKernel(f64 * Counter, u32 * Histogram, u32 const Width, u32 const Height)
{
	f32 const AspectRatio = (f32) Width / (f32) Height;
	f64 const sX(AspectRatio * 3.0), sY(3.0), cX(-0.5), cY(0);
	u32 const max_iteration = 1000;

	u32 posX = (blockIdx.x) * blockDim.x + threadIdx.x;
	u32 posY = (blockIdx.y) * blockDim.y + threadIdx.y;

	if (posX >= Width || posY >= Height)
		return;

	u32 iteration = 0;

	f64 x0 = posX / (f64) Width;
	f64 y0 = posY / (f64) Height;
	x0 -= 0.5;
	y0 -= 0.5;
	x0 *= sX;
	y0 *= sY;
	x0 += cX;
	y0 += cY;

	f64 x = 0.0, y = 0.0;
	while (x*x + y*y < 16.0 && iteration < max_iteration)
	{
		f64 xtemp = x*x - y*y + x0;
		y = 2.0*x*y + y0;

		x = xtemp;
		++ iteration;
	}

	f64 f_iteration = 0;
	if (iteration < max_iteration)
	{
		f64 zn = sqrt(x*x + y*y);
		f64 nu = log(log(zn) / log(2.0)) / log(2.0);
		f_iteration = iteration + 1.0 - nu;
	}
	atomicAdd(Histogram + iteration, 1);
	Counter[posY * Width + posX] = f_iteration;
}

__global__ void DrawKernel(u8 * Image, f64 * Counter, u32 * Histogram, u32 const Width, u32 const Height)
{
	u32 const max_iteration = 1000;

	u32 posX = (blockIdx.x) * blockDim.x + threadIdx.x;
	u32 posY = (blockIdx.y) * blockDim.y + threadIdx.y;

	if (posX >= Width || posY >= Height)
		return;

	u32 iteration = Counter[posY * Width + posX];
	f64 total = 0;
	for (u32 i = 0; i < max_iteration; ++ i)
	{
		total += Histogram[i];
	}

	f64 hue = 0;
	for (u32 i = 0; i < iteration; ++ i)
	{
		hue += Histogram[i] / total;
	}

	Image[posY * Width * 3 + posX * 3 + 0] = 0;
	Image[posY * Width * 3 + posX * 3 + 1] = (u8) ((hue/* + Counter[posY * Width + posX] - (f64) iteration*/) * 255.0);
	Image[posY * Width * 3 + posX * 3 + 2] = 50;
}

u8 const * CudaRenderFractal(u32 const Width, u32 const Height)
{
	u8 * HostImage = new u8[Width * Height * 3];

	u8 * DeviceImage; cudaMalloc((void**) & DeviceImage, Width * Height * 3 * sizeof(u8));
	f64 * DeviceCounter; cudaMalloc((void**) & DeviceCounter, Width * Height * sizeof(f64));
		cudaMemset(DeviceCounter, 0, Width * Height * sizeof(f64));
	u32 * DeviceHistogram; cudaMalloc((void**) & DeviceHistogram, 10000 * sizeof(u32));
		cudaMemset(DeviceHistogram, 0, 10000 * sizeof(u32));

	u32 const BlockSize = 16;
	dim3 const Grid(
		Width / BlockSize + (Width % BlockSize ? 1 : 0), 
		Height / BlockSize + (Height % BlockSize ? 1 : 0));
	dim3 const Block(BlockSize, BlockSize);
	HistogramKernel<<<Grid, Block>>>(DeviceCounter, DeviceHistogram, Width, Height);
	DrawKernel<<<Grid, Block>>>(DeviceImage, DeviceCounter, DeviceHistogram, Width, Height);

	cudaMemcpy(HostImage, DeviceImage, Width * Height * 3 * sizeof(u8), cudaMemcpyDeviceToHost);
	cudaFree(DeviceImage);
	cudaFree(DeviceCounter);
	cudaFree(DeviceHistogram);

	return HostImage;
}
