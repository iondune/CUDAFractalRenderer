
#pragma once

#include <ionCore/ionTypes.h>
#include "CudaVec2.cuh"


struct SFractalParams
{
	cvec2u ScreenSize;
	cvec2d Center;
	cvec2d Scale;
	u32 IterationMax;
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
};

class CudaFractalRenderer
{
	u32 IterationMax;

	SPixelState * DeviceStates;
	u32 * DeviceHistogram;

public:

	CudaFractalRenderer(SFractalParams const & Params);
	~CudaFractalRenderer();
	void Render(void * deviceBuffer, SFractalParams Params);
	void Reset(SFractalParams const & Params);

	u32 GetIterationMax() const
	{
		return IterationMax;
	}

};
