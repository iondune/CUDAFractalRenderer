
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

// __device__ static void ColorFromHSV(f64 const hue, f64 const saturation, f64 value, u8 & r, u8 & g, u8 & b)
// {
// 	int const hi = int(floor(hue / 60)) % 6;
// 	double const f = hue / 60 - floor(hue / 60);

// 	value = value * 255;
// 	int v = int(value);
// 	int p = int(value * (1 - saturation));
// 	int q = int(value * (1 - f * saturation));
// 	int t = int(value * (1 - (1 - f) * saturation));

// 	if (hi == 0)
// 	{
// 		r = v;
// 		g = t;
// 		b = p;
// 	}
// 	else if (hi == 1)
// 	{
// 		r = q;
// 		g = v;
// 		b = p;
// 	}
// 	else if (hi == 2)
// 	{
// 		r = p;
// 		g = v;
// 		b = t;
// 	}
// 	else if (hi == 3)
// 	{
// 		r = p;
// 		g = q;
// 		b = v;
// 	}
// 	else if (hi == 4)
// 	{
// 		r = t;
// 		g = p;
// 		b = v;
// 	}
// 	else
// 	{
// 		r = v;
// 		g = p;
// 		b = q;
// 	}
// }

struct Color
{
	u8 r, g, b;

	__device__ Color(u8 R, u8 G, u8 B)
		: r(R), g(G), b(B)
	{}
};

__device__ Color Hex(uint const Value)
{
	u8 r = (Value >> 16) & 0xFF;
	u8 g = (Value >> 8) & 0xFF;
	u8 b = Value & 0xFF;
	return Color(r, g, b);
}

__device__ static void ColorFromHue(f64 Hue, u8 & r, u8 & g, u8 & b, f64 const Amp)
{
	Hue = pow(Hue, Amp);
	//ColorFromHSV(fmod(Hue * (360 + 60), 360.0), 1, 1, r, g, b);
	Color const Colors[] =
	{
		//Hex(0x446144),
		//Hex(0x18CC1B),
		//Hex(0x9BE09C),

		// Yellow/Teal
		////Hex(0xED4CA2),
		//Hex(0xEDE74C),
		////Hex(0x9BBDE0),
		//Hex(0x4CED97),
		//Hex(0x4C52ED),
		//Hex(0xffffff),

		// Pastel
		//Hex(0x9BE09C),
		//Hex(0xE09BDF),
		//Hex(0xE0BF9B),
		//Hex(0x9BBDE0),
		//Hex(0xffffff),

		// Red/Blue
		//Hex(0xFF4E33),
		//Hex(0x2A60FF),
		//Color(255, 255, 255),

		// Blue/Green/White
		Hex(0xFFFFFF),
		Hex(0x4769FF),
		Hex(0x47FF81),

		//// Blue/Green/Dark
		//Hex(0x00A835),
		//Hex(0x3A5BF0),
		//Hex(0xFFFFFF),

		// Valentines
		//Color(250, 70, 91),
		//Color(134, 54, 173),
		//Color(255, 255, 255),

		//Color(255, 119, 46),
		//Color(46, 182, 255),
		//Color(255, 255, 255),
	};

	if (Hue < 0)
		Hue = 0;
	if (Hue > 1)
		Hue = 1;

	int const Count = sizeof(Colors) / sizeof(* Colors);
	int const Wrap = 2;
	int const Size = Count * Wrap - 1;
	int const Below = (int) floor(Hue * Size) % Count;
	int const Above = (int) ceil(Hue * Size) % Count;
	f64 const Part = Hue * Size - floor(Hue * Size);

	r = (u8) ((1 - Part) * Colors[Below].r + Part * Colors[Above].r);
	g = (u8) ((1 - Part) * Colors[Below].g + Part * Colors[Above].g);
	b = (u8) ((1 - Part) * Colors[Below].b + Part * Colors[Above].b);
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

		f64 const Hue = Average;
		u8 r, g, b;
		f64 Amp = 1;
		f64 const MaxAmp = 64;
		f64 const StartAmp = 1;
		f64 const EndAmp = 3;
		if (Params.Scale.Y > EndAmp)
			Amp = MaxAmp;
		else if (Params.Scale.Y > StartAmp)
			Amp = pow((Params.Scale.Y - StartAmp) / (EndAmp - StartAmp), 2) * (MaxAmp - 1) + 1;
		ColorFromHue(Hue, r, g, b, Amp);
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

	u32 const Stride = Params.Stride;
	((u8 *) Image)[PixelCoordinates.Y * Params.ScreenSize.X * Stride + PixelCoordinates.X * Stride + 0] = R;
	((u8 *) Image)[PixelCoordinates.Y * Params.ScreenSize.X * Stride + PixelCoordinates.X * Stride + 1] = G;
	((u8 *) Image)[PixelCoordinates.Y * Params.ScreenSize.X * Stride + PixelCoordinates.X * Stride + 2] = B;
	if (Stride > 3)
		((u8 *) Image)[PixelCoordinates.Y * Params.ScreenSize.X * Stride + PixelCoordinates.X * Stride + 3] = 255;
}
