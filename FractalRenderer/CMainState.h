
#pragma once

#include <ionEngine.h>
#include "CudaFractalRender.cuh"
#include "CFrameRateCounter.h"


enum EFractalType
{
	EFT_MANDEL,
	EFT_BURNING_SHIP,
	EFT_TRICORN,
	EFT_MULTIBROT_1,
	EFT_MULTIBROT_2,
	EFT_JULIA,
	EFT_COUNT
};

enum EShaderSettings
{
	ESS_DEFAULT,
	ESS_MS2,
	ESS_MS3,
	ESS_MS4,
	ESS_STOCH,
	ESS_STOCH2,
	ESS_COUNT
};

class CMainState : public CContextState<CMainState>
{
	CShader * Finalize;
	CTexture * CopyTexture;

	int CurrentFractal;
	int CurrentSettings;
	int CurrentColor;
	bool DumpFrames;

	CudaFractalRenderer FractalRenderer;
	u32 CudaDrawBufferHandle, ScreenTextureHandle;
	CFrameRateCounter FrameRateCounter;
	freetype::font_data Font;

	std::vector<CTexture *> ColorMaps;
	vec3f uSetColor;
	int SetColorCounter;
	
	float TextureScaling;
	int ScaleFactor;
	
	void RecalcScale();
	void SetSetColor();

public:

	CMainState();
	void Begin();
	void Update(f32 const Elapsed);
	
	void OnEvent(SKeyboardEvent & Event);
	void OnEvent(SMouseEvent & Event);

	void PrintLocation();

};
