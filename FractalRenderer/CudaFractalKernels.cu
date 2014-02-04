
#include "CudaFractalKernels.cuh"


__global__ void InitKernel(SPixelState * States,  SFractalParams Params)
{
	u32 const MSWidth = Params.ScreenSize.X * Params.MultiSample;
	u32 const MSHeight = Params.ScreenSize.Y * Params.MultiSample;

	cvec2u PixelCoordinates(blockIdx.x * blockDim.x + threadIdx.x, blockIdx.y * blockDim.y + threadIdx.y);
	if (PixelCoordinates.X >= MSWidth || PixelCoordinates.Y >= MSHeight)
		return;

	SPixelState & State = States[PixelCoordinates.Y * MSWidth + PixelCoordinates.X];
	State.Counter = 0;
	State.Point = cvec2d();
	State.Iteration = 0;
	State.LastMax = 0;
	State.LastTotal = 0;
	State.FinalSum = 0;
	State.Finished = false;
	State.Calculated = false;
	State.R = State.G = State.B = 0;
}

__global__ void HistogramKernel(SPixelState * States, u32 * Histogram, SFractalParams Params)
{
	u32 const MSWidth = Params.ScreenSize.X * Params.MultiSample;
	u32 const MSHeight = Params.ScreenSize.Y * Params.MultiSample;

	cvec2u PixelCoordinates(blockIdx.x * blockDim.x + threadIdx.x, blockIdx.y * blockDim.y + threadIdx.y);
	if (PixelCoordinates.X >= MSWidth || PixelCoordinates.Y >= MSHeight)
		return;

	SPixelState & State = States[PixelCoordinates.Y * MSWidth + PixelCoordinates.X];
	if (State.Finished)
		return;

	cvec2d Point = State.Point;
	u32 IterationCounter = State.Iteration;
	cvec2d StartPosition(PixelCoordinates.X / (f64) MSWidth, PixelCoordinates.Y / (f64) MSHeight);
	StartPosition -= 0.5;
	StartPosition *= Params.Scale;
	cvec2d const Original = StartPosition;
	f64 const S = Params.RotationVector.X;
	f64 const C = Params.RotationVector.Y;
	StartPosition.X = Original.X * C - Original.Y * S;
	StartPosition.Y = Original.X * S + Original.Y * C;
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
	u32 const MSWidth = Params.ScreenSize.X * Params.MultiSample;
	u32 const MSHeight = Params.ScreenSize.Y * Params.MultiSample;

	cvec2u PixelCoordinates(blockIdx.x * blockDim.x + threadIdx.x, blockIdx.y * blockDim.y + threadIdx.y);
	if (PixelCoordinates.X >= MSWidth || PixelCoordinates.Y >= MSHeight)
		return;

	SPixelState & State = States[PixelCoordinates.Y * MSWidth + PixelCoordinates.X];
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
		State.R = r;
		State.G = g;
		State.B = b;
	}
	else
	{
		State.R = State.G = State.B = 0;
		return;
	}
}

__global__ void FinalKernel(void * Image, SPixelState * States, u32 * Histogram, SFractalParams Params)
{
	cvec2u PixelCoordinates(blockIdx.x * blockDim.x + threadIdx.x, blockIdx.y * blockDim.y + threadIdx.y);
	if (PixelCoordinates.X >= Params.ScreenSize.X || PixelCoordinates.Y >= Params.ScreenSize.Y)
		return;

	f64 R = 0, G = 0, B = 0;
	for (u32 y = 0; y < Params.MultiSample; ++ y)
	for (u32 x = 0; x < Params.MultiSample; ++ x)
	{
		SPixelState & State = States[
			PixelCoordinates.Y * Params.MultiSample * Params.ScreenSize.X * Params.MultiSample +
			PixelCoordinates.X * Params.MultiSample +
			y * Params.ScreenSize.X * Params.MultiSample +
			x];
		R += State.R;
		G += State.G;
		B += State.B;
	}

	R /= Params.MultiSample * Params.MultiSample;
	G /= Params.MultiSample * Params.MultiSample;
	B /= Params.MultiSample * Params.MultiSample;

	((u8 *) Image)[PixelCoordinates.Y * Params.ScreenSize.X * 4 + PixelCoordinates.X * 4 + 0] = R;
	((u8 *) Image)[PixelCoordinates.Y * Params.ScreenSize.X * 4 + PixelCoordinates.X * 4 + 1] = G;
	((u8 *) Image)[PixelCoordinates.Y * Params.ScreenSize.X * 4 + PixelCoordinates.X * 4 + 2] = B;
	((u8 *) Image)[PixelCoordinates.Y * Params.ScreenSize.X * 4 + PixelCoordinates.X * 4 + 3] = 255;
}