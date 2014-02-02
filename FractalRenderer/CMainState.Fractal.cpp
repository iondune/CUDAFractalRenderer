
#pragma once

#include "CMainState.h"


void CMainState::PrintLocation()
{
	printf("sX: %f   sY: %f   cX: %.15f   cY: %.15f\n",
		FractalRenderer.Params.Scale.X, FractalRenderer.Params.Scale.Y,
		FractalRenderer.Params.Center.X, FractalRenderer.Params.Center.Y);
}
