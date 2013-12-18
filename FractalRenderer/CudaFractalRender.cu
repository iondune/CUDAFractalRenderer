
#include "CudaFractalRender.cuh"
#include "cuda_runtime.h"

#include <ionCore/ionUtils.h>




__global__ void InitKernel(SPixelState * States,  SFractalParams Params)
{
	cvec2u PixelCoordinates(blockIdx.x * blockDim.x + threadIdx.x, blockIdx.y * blockDim.y + threadIdx.y);
	if (PixelCoordinates.X >= Params.ScreenSize.X || PixelCoordinates.Y >= Params.ScreenSize.Y)
		return;

	SPixelState & State = States[PixelCoordinates.Y * Params.ScreenSize.X + PixelCoordinates.X];
	State.Counter = 0;
	State.Point = cvec2d();
	State.Iteration = 0;
	State.LastMax = 0;
	State.LastTotal = 0;
	State.FinalSum = 0;
	State.Finished = false;
	State.Calculated = false;
}

__global__ void HistogramKernel(SPixelState * States, u32 * Histogram, SFractalParams Params)
{
	cvec2u PixelCoordinates(blockIdx.x * blockDim.x + threadIdx.x, blockIdx.y * blockDim.y + threadIdx.y);
	if (PixelCoordinates.X >= Params.ScreenSize.X || PixelCoordinates.Y >= Params.ScreenSize.Y)
		return;

	SPixelState & State = States[PixelCoordinates.Y * Params.ScreenSize.X + PixelCoordinates.X];
	if (State.Finished)
		return;

	cvec2d Point = State.Point;
	u32 IterationCounter = State.Iteration;
	cvec2d StartPosition(PixelCoordinates.X / (f64) Params.ScreenSize.X, PixelCoordinates.Y / (f64) Params.ScreenSize.Y);
	StartPosition -= 0.5;
	StartPosition *= Params.Scale;
	StartPosition += Params.Center;

	while (Dot(Point, Point) < 256.0 && IterationCounter < Params.IterationMax)
	{
		Point = cvec2d(Point.X*Point.X - Point.Y*Point.Y + StartPosition.X, 2 * Point.X * Point.Y + StartPosition.Y);
		++ IterationCounter;
	}
	State.Iteration = IterationCounter;
	State.Point = Point;

	f64 ContinuousIterator = 0;
	if (IterationCounter < Params.IterationMax)
	{
		f64 Zn = sqrt(Dot(Point, Point));
		f64 Nu = log(log(Zn) / log(2.0)) / log(2.0);
		ContinuousIterator = IterationCounter + 1 - Nu;
		atomicAdd(Histogram + IterationCounter, 1);
		State.Finished = true;
	}
	else
	{
		ContinuousIterator = Params.IterationMax;
	}

	State.Counter = ContinuousIterator;
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

__global__ void DrawKernel(void * Image, SPixelState * States, u32 * Histogram, SFractalParams Params)
{
	cvec2u PixelCoordinates(blockIdx.x * blockDim.x + threadIdx.x, blockIdx.y * blockDim.y + threadIdx.y);
	if (PixelCoordinates.X >= Params.ScreenSize.X || PixelCoordinates.Y >= Params.ScreenSize.Y)
		return;

	SPixelState & State = States[PixelCoordinates.Y * Params.ScreenSize.X + PixelCoordinates.X];
	u32 const LastMax = State.LastMax;
	u32 const LastTotal = State.LastTotal;

	// Update Total
	u32 Total = LastTotal;
	for (u32 i = LastMax; i < Params.IterationMax; ++ i)
		Total += Histogram[i];
	State.LastMax = Params.IterationMax;
	State.LastTotal = Total;

	if (State.Finished)
	{
		f64 const Counter = State.Counter;
		u32 const Iteration = floor(Counter);
		f64 const Delta = Counter - (f64) Iteration;

		u32 Sum = 0;
		if (State.Calculated)
		{
			Sum = State.FinalSum;
		}
		else
		{
			for (u32 i = 0; i < Iteration; ++ i)
				Sum += Histogram[i];
			State.FinalSum = Sum;
			State.Calculated = true;
		}

		f64 Average = Sum / (f64) Total;
		f64 AverageOneUp = Average + Histogram[Iteration] / (f64) Total;
		Average = Average * (1 - Delta) + AverageOneUp * Delta;

		f64 const Hue = pow(Average, 8);
		u8 r, g, b;
		ColorFromHSV(Hue * 255, 1, 1, r, g, b);
		((u8 *) Image)[PixelCoordinates.Y *  Params.ScreenSize.X * 4 + PixelCoordinates.X * 4 + 0] = r;//0;
		((u8 *) Image)[PixelCoordinates.Y *  Params.ScreenSize.X * 4 + PixelCoordinates.X * 4 + 1] = g;//(u8) (Hue * 255);
		((u8 *) Image)[PixelCoordinates.Y *  Params.ScreenSize.X * 4 + PixelCoordinates.X * 4 + 2] = b;//(u8) ((1 - Hue) * 255);
		((u8 *) Image)[PixelCoordinates.Y *  Params.ScreenSize.X * 4 + PixelCoordinates.X * 4 + 3] = 255;
	}
	else
	{
		((u8 *) Image)[PixelCoordinates.Y *  Params.ScreenSize.X * 4 + PixelCoordinates.X * 4 + 0] = 0;
		((u8 *) Image)[PixelCoordinates.Y *  Params.ScreenSize.X * 4 + PixelCoordinates.X * 4 + 1] = 0;
		((u8 *) Image)[PixelCoordinates.Y *  Params.ScreenSize.X * 4 + PixelCoordinates.X * 4 + 2] = 0;
		((u8 *) Image)[PixelCoordinates.Y *  Params.ScreenSize.X * 4 + PixelCoordinates.X * 4 + 3] = 255;
		return;
	}
}


void CudaFractalRenderer::Init(cvec2u const & ScreenSize)
{
	u32 const ScreenCount = ScreenSize.X * ScreenSize.Y;
	u32 const StateCount = ScreenCount * sizeof(SPixelState);

	Params.ScreenSize = ScreenSize;
	Params.Scale.X *= ScreenSize.X / (f64) ScreenSize.Y;
	IterationIncrement = 100;
	cudaMalloc((void**) & DeviceStates, StateCount);

	DeviceHistogram = 0;
	Reset();
}

CudaFractalRenderer::~CudaFractalRenderer()
{
	cudaFree(DeviceStates);
	cudaFree(DeviceHistogram);
}

void CudaFractalRenderer::Reset()
{
	HistogramSize = (Params.IterationMax + 1) * sizeof(u32);
	u32 const BlockSize = 16;

	if (DeviceHistogram)
		cudaFree(DeviceHistogram);
	cudaMalloc((void**) & DeviceHistogram, HistogramSize);
	cudaMemset(DeviceHistogram, 0, HistogramSize);

	dim3 const Grid(
		Params.ScreenSize.X / BlockSize + (Params.ScreenSize.X % BlockSize ? 1 : 0), 
		Params.ScreenSize.Y / BlockSize + (Params.ScreenSize.Y % BlockSize ? 1 : 0));
	dim3 const Block(BlockSize, BlockSize);
	InitKernel<<<Grid, Block>>>(DeviceStates, Params);

	IterationMax = 100;
}

void CudaFractalRenderer::SoftReset()
{
	u32 const NewHistogramSize = (Params.IterationMax + 1) * sizeof(u32);

	u32 * NewDeviceHistogram;
	cudaMalloc((void**) & NewDeviceHistogram, NewHistogramSize);
	cudaMemset(NewDeviceHistogram, 0, NewHistogramSize);
	cudaMemcpy(NewDeviceHistogram, DeviceHistogram, HistogramSize, cudaMemcpyDeviceToDevice);
	cudaFree(DeviceHistogram);
	DeviceHistogram = NewDeviceHistogram;
	HistogramSize = NewHistogramSize;
}

void CudaFractalRenderer::Render(void * deviceBuffer)
{
	u32 const BlockSize = 16;
	SFractalParams ParamsCopy = Params;

	if (IterationMax < ParamsCopy.IterationMax)
	{
		IterationMax = Min(IterationMax + IterationIncrement, ParamsCopy.IterationMax);

		dim3 const Grid(
			ParamsCopy.ScreenSize.X / BlockSize + (ParamsCopy.ScreenSize.X % BlockSize ? 1 : 0), 
			ParamsCopy.ScreenSize.Y / BlockSize + (ParamsCopy.ScreenSize.Y % BlockSize ? 1 : 0));
		dim3 const Block(BlockSize, BlockSize);

		if (IterationMax <= ParamsCopy.IterationMax)
		{
			ParamsCopy.IterationMax = IterationMax;
			HistogramKernel<<<Grid, Block>>>(DeviceStates, DeviceHistogram, ParamsCopy);
			DrawKernel<<<Grid, Block>>>(deviceBuffer, DeviceStates, DeviceHistogram, ParamsCopy);
		}
	}
}

u32 CudaFractalRenderer::GetIterationMax() const
{
	return IterationMax;
}
