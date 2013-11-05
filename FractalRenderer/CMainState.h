
#pragma once

#include <ionEngine.h>

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

	CShader * Shader[EFT_COUNT][ESS_COUNT];

	int CurrentFractal;
	int CurrentSettings;
	int CurrentColor;

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

	double sX, sY;
	double cX, cY;
	int max_iteration;

	void PrintLocation();

};