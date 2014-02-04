
#pragma once

#include "CudaVec2.cuh"


struct SFractalParams
{
	cvec2u ScreenSize;
	cvec2d Center;
	cvec2d Scale;
	u32 MultiSample;
	u32 IterationMax;
	cvec2d RotationVector;

	SFractalParams()
	{
		Scale = cvec2d(1, 1);
		Center = cvec2d(0, 0.7);
		IterationMax = 1000;
		MultiSample = 1;
		RotationVector = cvec2d(0, 1);
	}

	void SetRotation(f64 const Angle)
	{
		RotationVector = cvec2d(sin(Angle), cos(Angle));
	}
};

struct SPixelState
{
	f64 Counter;
	cvec2d Point;
	u32 Iteration;
	u32 LastMax;
	u32 LastTotal;
	bool Finished;
	u32 FinalSum;
	bool Calculated;
	u8 R, G, B;
};

class CudaFractalRenderer
{

public:

	void Init(cvec2u const & ScreenSize);
	~CudaFractalRenderer();

	void Render(void * deviceBuffer);
	void FullReset();
	void Reset();
	void SoftReset();

	u32 GetIterationMax() const;

	SFractalParams Params;
	u32 IterationIncrement;

protected:

	u32 IterationMax;
	u32 HistogramSize;

	SPixelState * DeviceStates;
	u32 * DeviceHistogram;

};
