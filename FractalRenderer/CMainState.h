
#pragma once

#include <ionEngine.h>
#include "CudaFractalRenderer.cuh"
#include "CFrameRateCounter.h"


class CMainState : public CContextState<CMainState>
{

public:

	CMainState();

	void Begin();
	void Update(f32 const Elapsed);

	void OnEvent(SKeyboardEvent & Event);
	void OnEvent(SMouseEvent & Event);

protected:

	bool IsRenderReady() const;

	void DumpFrameToFile();
	void PrintTextOverlay();
	void PrintLocation();

	void DoRender();
	void DrawRenderToScreen();

	void BindTexture();
	void LoadTextureData();
	void DoTextureDraw();
	
	CudaFractalRenderer FractalRenderer;

	bool DumpFrames;
	bool RenderZoom;
	f64 LastRotation;
	int CurrentDumpFrame;

	CFrameRateCounter FrameRateCounter;
	freetype::font_data Font;

	u32 CudaDrawBufferHandle, ScreenTextureHandle;
	CShader * Finalize;
	CTexture * CopyTexture;

	std::vector<CImage *> ColorMaps;
	int CurrentColor;
	int Multisample;

	bool ShowText;

};
