
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
	else
		ContinuousIterator = Params.IterationMax;
	atomicAdd(Histogram + IterationCounter, 1);
	Counter[PixelCoordinates.Y * Params.ScreenSize.X + PixelCoordinates.X] = ContinuousIterator;
}

__device__ static void ColorFromHSV(f64 const hue, f64 const saturation, f64 value, u8 & r, u8 & g, u8 & b)
{
    int const hi = int(floor(hue / 60)) % 6;
    double const f = hue / 60 - floor(hue / 60);

    value = value * 255;
    int v = int(value);
    int p = int(value * (1 - saturation));
    int q = int(value * (1 - f * saturation));
    int t = int(value * (1 - (1 - f) * saturation));
	
    if (hi == 0)
	{
		r = v;
		g = t;
		b = p;
	}
    else if (hi == 1)
	{
		r = q;
		g = v;
		b = p;
	}
    else if (hi == 2)
	{
		r = p;
		g = v;
		b = t;
	}
    else if (hi == 3)
	{
		r = p;
		g = q;
		b = v;
	}
    else if (hi == 4)
	{
		r = t;
		g = p;
		b = v;
	}
    else
	{
		r = v;
		g = p;
		b = q;
	}
}

__global__ void DrawKernel(u8 * Image, f64 * Counter, u32 * Histogram, SFractalParams Params)
{
	cvec2u PixelCoordinates(blockIdx.x * blockDim.x + threadIdx.x, blockIdx.y * blockDim.y + threadIdx.y);
	
	if (PixelCoordinates.X >= Params.ScreenSize.X || PixelCoordinates.Y >= Params.ScreenSize.Y)
		return;

	if (Counter[PixelCoordinates.Y * Params.ScreenSize.X + PixelCoordinates.X] == Params.IterationMax)
	{
		Image[PixelCoordinates.Y *  Params.ScreenSize.X * 3 + PixelCoordinates.X * 3 + 0] = 0;
		Image[PixelCoordinates.Y *  Params.ScreenSize.X * 3 + PixelCoordinates.X * 3 + 1] = 0;
		Image[PixelCoordinates.Y *  Params.ScreenSize.X * 3 + PixelCoordinates.X * 3 + 2] = 0;
		return;
	}

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

	u8 r, g, b;
	f64 hueit = pow(hue, 8);
	ColorFromHSV(hueit * 255, 1, 1, r, g, b);
	Image[PixelCoordinates.Y *  Params.ScreenSize.X * 3 + PixelCoordinates.X * 3 + 0] = 0;
	Image[PixelCoordinates.Y *  Params.ScreenSize.X * 3 + PixelCoordinates.X * 3 + 1] = (u8) (hueit * 255);
	Image[PixelCoordinates.Y *  Params.ScreenSize.X * 3 + PixelCoordinates.X * 3 + 2] = (u8) ((1 - hueit) * 255);
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
