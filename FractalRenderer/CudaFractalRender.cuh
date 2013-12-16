
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
	bool Finished;
};

class CudaFractalRenderer
{
	u8 * HostImage;
	u32 IterationMax;

	u8 * DeviceImage;
	SPixelState * DeviceStates;
	u32 * DeviceHistogram;

public:

	CudaFractalRenderer(SFractalParams const & Params);
	~CudaFractalRenderer();
	u8 const * Render(SFractalParams Params);
	void Reset(SFractalParams const & Params);

};
