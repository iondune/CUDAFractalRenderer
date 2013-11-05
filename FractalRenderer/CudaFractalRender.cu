
#include "CudaFractalRender.cuh"
#include "cuda_runtime.h"

#include <ionCore/ionUtils.h>


__global__ void HistogramKernel(f64 * Counter, u32 * Histogram, SFractalParams Params)
{
	cvec2u PixelCoordinates(blockIdx.x * blockDim.x + threadIdx.x, blockIdx.y * blockDim.y + threadIdx.y);

	if (PixelCoordinates.X >= Params.ScreenSize.X || PixelCoordinates.Y >= Params.ScreenSize.Y)
		return;

	cvec2d StartPosition(PixelCoordinates.X / (f64) Params.ScreenSize.X, PixelCoordinates.Y / (f64) Params.ScreenSize.Y);
	StartPosition -= 0.5;
	StartPosition *= Params.Scale;
	StartPosition += Params.Center;

	cvec2d Point(0, 0);
	u32 IterationCounter = 0;
	while (Dot(Point, Point) < 256.0 && IterationCounter < Params.IterationMax)
	{
		Point = cvec2d(Point.X*Point.X - Point.Y*Point.Y + StartPosition.X, 2 * Point.X * Point.Y + StartPosition.Y);
		++ IterationCounter;
	}

	f64 ContinuousIterator = 0;
	if (IterationCounter < Params.IterationMax)
	{
		f64 Zn = sqrt(Dot(Point, Point));
		f64 Nu = log(log(Zn) / log(2.0)) / log(2.0);
		ContinuousIterator = IterationCounter + 1 - Nu;
	}
	atomicAdd(Histogram + IterationCounter, 1);
	Counter[PixelCoordinates.Y * Params.ScreenSize.X + PixelCoordinates.X] = ContinuousIterator;
}

__global__ void DrawKernel(u8 * Image, f64 * Counter, u32 * Histogram, SFractalParams Params)
{
	cvec2u PixelCoordinates(blockIdx.x * blockDim.x + threadIdx.x, blockIdx.y * blockDim.y + threadIdx.y);
	
	if (PixelCoordinates.X >= Params.ScreenSize.X || PixelCoordinates.Y >= Params.ScreenSize.Y)
		return;

	u32 iteration = floor(Counter[PixelCoordinates.Y * Params.ScreenSize.X + PixelCoordinates.X]);
	f64 total = 0;
	for (u32 i = 0; i < Params.IterationMax; ++ i)
		total += Histogram[i];

	f64 hue = 0;
	for (u32 i = 0; i < iteration; ++ i)
		hue += Histogram[i] / total;
	f64 oneuphue = hue + Histogram[iteration] / total;

	f64 delta = Counter[PixelCoordinates.Y * Params.ScreenSize.X + PixelCoordinates.X] - (f64) iteration;
	hue = hue * (1 - delta) + oneuphue * delta;

	Image[PixelCoordinates.Y *  Params.ScreenSize.X * 3 + PixelCoordinates.X * 3 + 0] = 0;
	Image[PixelCoordinates.Y *  Params.ScreenSize.X * 3 + PixelCoordinates.X * 3 + 1] = (u8) (hue * 255.0);
	Image[PixelCoordinates.Y *  Params.ScreenSize.X * 3 + PixelCoordinates.X * 3 + 2] = 50;
}

u8 const * CudaRenderFractal(SFractalParams const & Params)
{
	u32 const ScreenSize = Params.ScreenSize.X * Params.ScreenSize.Y;
	u32 const ImageSize = ScreenSize * 3;
	u32 const CounterSize = ScreenSize * sizeof(f64);
	u32 const HistogramSize = (Params.IterationMax + 1) * sizeof(u32);

	u8 * HostImage = new u8[ImageSize];

	u8 * DeviceImage; cudaMalloc((void**) & DeviceImage, ImageSize);
	f64 * DeviceCounter; cudaMalloc((void**) & DeviceCounter, CounterSize);
		cudaMemset(DeviceCounter, 0, CounterSize);
	u32 * DeviceHistogram; cudaMalloc((void**) & DeviceHistogram, HistogramSize);
		cudaMemset(DeviceHistogram, 0, HistogramSize);

	u32 const BlockSize = 16;
	dim3 const Grid(
		Params.ScreenSize.X / BlockSize + (Params.ScreenSize.X % BlockSize ? 1 : 0), 
		Params.ScreenSize.Y / BlockSize + (Params.ScreenSize.Y % BlockSize ? 1 : 0));
	dim3 const Block(BlockSize, BlockSize);
	HistogramKernel<<<Grid, Block>>>(DeviceCounter, DeviceHistogram, Params);
	DrawKernel<<<Grid, Block>>>(DeviceImage, DeviceCounter, DeviceHistogram, Params);

	cudaMemcpy(HostImage, DeviceImage, ImageSize * sizeof(u8), cudaMemcpyDeviceToHost);
	cudaFree(DeviceImage);
	cudaFree(DeviceCounter);
	cudaFree(DeviceHistogram);

	return HostImage;
}
