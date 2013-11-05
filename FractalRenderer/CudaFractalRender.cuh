
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

u8 const * CudaRenderFractal(SFractalParams const & Params);
