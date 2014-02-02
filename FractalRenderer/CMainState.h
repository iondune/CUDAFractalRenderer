
#pragma once

#include <ionEngine.h>
#include "CudaFractalRender.cuh"
#include "CFrameRateCounter.h"


class CMainState : public CContextState<CMainState>
{
	CShader * Finalize;
	CTexture * CopyTexture;

	bool DumpFrames;
	int CurrentDumpFrame;

	CudaFractalRenderer FractalRenderer;
	u32 CudaDrawBufferHandle, ScreenTextureHandle;
	CFrameRateCounter FrameRateCounter;
	freetype::font_data Font;

	std::vector<CImage *> ColorMaps;
	int CurrentColor;
	int Multisample;

public:

	CMainState();
	void Begin();
	void Update(f32 const Elapsed);

	void DumpFrameToFile();
	void PrintTextOverlay();
	
	void OnEvent(SKeyboardEvent & Event);
	void OnEvent(SMouseEvent & Event);

	void PrintLocation();

};
