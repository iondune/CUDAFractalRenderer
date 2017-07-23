
#pragma once

#include <cmath>
#include "CudaVec2.cuh"


struct SFractalParams
{
	cvec2u ScreenSize;
	cvec2d Center;
	cvec2d Scale;
	u32 MultiSample;
	u32 IterationMax;
	cvec2d RotationVector;
	u32 Stride;
	f64 Angle;

	SFractalParams()
	{
		Scale = cvec2d(0.0000023, 0.0000023);
		Center = cvec2d(-0.021201023968658, 0.710450082430672);
		IterationMax = 4000;
		MultiSample = 1;
		RotationVector = cvec2d(0, 1);
		Stride = 4;
	}

	void SetRotation(f64 const angle)
	{
		Angle = angle;
		RotationVector = cvec2d(sin(Angle), cos(Angle));
	}
};

struct SPixelState   //  48 bytes
{
	f64 Counter;     //  8
	cvec2d Point;    // 16
	u32 Iteration;   //  4
	u32 LastMax;     //  4
	u32 LastTotal;   //  4
	bool Finished;   //  4
	u32 FinalSum;    //  4
	bool Calculated; //
	u8 R, G, B;      //  4
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

	bool Done() const
	{
		return IterationMax == Params.IterationMax;
	}

protected:

	u32 IterationMax;
	u32 HistogramSize;

	SPixelState * DeviceStates;
	u32 * DeviceHistogram;

};
