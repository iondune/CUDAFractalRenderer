
#pragma once

#include "CMainState.h"


void CMainState::RecalcScale()
{
	if (-1 <= ScaleFactor && ScaleFactor <= 1)
		TextureScaling = 1.f;
	else if (ScaleFactor < -1)
	{
		TextureScaling = -ScaleFactor / 1.f;
	}
	else if (ScaleFactor > 1)
	{
		TextureScaling = 1.f / ScaleFactor;
	}
}

void CMainState::SetSetColor()
{
	if (SetColorCounter > 8)
		SetColorCounter = 0;
	if (SetColorCounter < 0)
		SetColorCounter = 8;

	switch (SetColorCounter)
	{
	case 0:
		uSetColor = vec3f(0.f);
		break;
	case 1:
		uSetColor = vec3f(1.f);
		break;
	case 2:
		uSetColor = vec3f(0.9f, 0.2f, 0.1f);
		break;
	case 3:
		uSetColor = vec3f(0.2f, 0.9f, 0.1f);
		break;
	case 4:
		uSetColor = vec3f(0.1f, 0.2f, 0.9f);
		break;
	case 5:
		uSetColor = vec3f(0.9f, 0.8f, 0.1f);
		break;
	case 6:
		uSetColor = vec3f(0.8f, 0.6f, 0.4f);
		break;
	case 7:
		uSetColor = vec3f(0.4f, 0.6f, 0.8f);
		break;
	case 8:
		uSetColor = vec3f(0.9f, 0.3f, 0.8f);
		break;
	};
}

void CMainState::PrintLocation()
{
	printf("sX: %f   sY: %f   cX: %.15f   cY: %.15f\n", sX, sY, cX, cY);
}
