
#include "CMainState.h"
#include "SRenderPass.h"


CMainState::CMainState()
	: sX(1.0), sY(1.0), cX(0.0), cY(0.7), max_iteration(1000), uSetColor(0.0f), ScaleFactor(1), TextureScaling(1.f),
	CurrentFractal(EFT_MANDEL), CurrentSettings(ESS_DEFAULT), CurrentColor(0), SetColorCounter(0)
{
	sX *= Application->GetWindow().GetAspectRatio();
}

void CMainState::Begin()
{
	glClearColor(0.6f, 0.6f, 0.6f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	for (int i = 0; i < EFT_COUNT; ++ i)
	{
		for (int j = 0; j < ESS_COUNT; ++ j)
		{
			Shader[i][j] = 0;
		}
	}

	Shader[EFT_MANDEL][ESS_DEFAULT] = CShaderLoader::loadShader("QuadCopyUV.glsl", "Mandelbrot1.frag");
	Shader[EFT_MANDEL][ESS_MS2] = CShaderLoader::loadShader("QuadCopyUV.glsl", "Mandelbrot1-2x2MS.frag");
	Shader[EFT_MANDEL][ESS_MS3] = CShaderLoader::loadShader("QuadCopyUV.glsl", "Mandelbrot1-3x3MS.frag");
	Shader[EFT_MANDEL][ESS_MS4] = CShaderLoader::loadShader("QuadCopyUV.glsl", "Mandelbrot1-4x4MS.frag");
	Shader[EFT_MANDEL][ESS_STOCH] = CShaderLoader::loadShader("QuadCopyUV.glsl", "Mandelbrot1-Stoch.frag");
	Shader[EFT_MANDEL][ESS_STOCH2] = CShaderLoader::loadShader("QuadCopyUV.glsl", "Mandelbrot1-2x2Stoch.frag");

	Shader[EFT_BURNING_SHIP][ESS_DEFAULT] = CShaderLoader::loadShader("QuadCopyUV.glsl", "BurningShip.frag");
	Shader[EFT_BURNING_SHIP][ESS_MS2] = CShaderLoader::loadShader("QuadCopyUV.glsl", "BurningShip-2x2MS.frag");
	Shader[EFT_BURNING_SHIP][ESS_MS3] = CShaderLoader::loadShader("QuadCopyUV.glsl", "BurningShip-3x3MS.frag");
	Shader[EFT_BURNING_SHIP][ESS_MS4] = CShaderLoader::loadShader("QuadCopyUV.glsl", "BurningShip-4x4MS.frag");

	Shader[EFT_TRICORN][ESS_DEFAULT] = CShaderLoader::loadShader("QuadCopyUV.glsl", "Tricorn.frag");
	Shader[EFT_TRICORN][ESS_MS2] = CShaderLoader::loadShader("QuadCopyUV.glsl", "Tricorn-2x2MS.frag");
	Shader[EFT_TRICORN][ESS_MS3] = CShaderLoader::loadShader("QuadCopyUV.glsl", "Tricorn-3x3MS.frag");
	Shader[EFT_TRICORN][ESS_MS4] = CShaderLoader::loadShader("QuadCopyUV.glsl", "Tricorn-4x4MS.frag");
		
	Shader[EFT_MULTIBROT_1][ESS_DEFAULT] = CShaderLoader::loadShader("QuadCopyUV.glsl", "Multibrot1.frag");
	Shader[EFT_MULTIBROT_1][ESS_MS2] = CShaderLoader::loadShader("QuadCopyUV.glsl", "Multibrot1-2x2MS.frag");
	Shader[EFT_MULTIBROT_1][ESS_MS3] = CShaderLoader::loadShader("QuadCopyUV.glsl", "Multibrot1-3x3MS.frag");
	Shader[EFT_MULTIBROT_1][ESS_MS4] = CShaderLoader::loadShader("QuadCopyUV.glsl", "Multibrot1-4x4MS.frag");

		
	Shader[EFT_MULTIBROT_2][ESS_DEFAULT] = CShaderLoader::loadShader("QuadCopyUV.glsl", "Multibrot2.frag");
	Shader[EFT_MULTIBROT_2][ESS_MS2] = CShaderLoader::loadShader("QuadCopyUV.glsl", "Multibrot2-2x2MS.frag");
	Shader[EFT_MULTIBROT_2][ESS_MS3] = CShaderLoader::loadShader("QuadCopyUV.glsl", "Multibrot2-3x3MS.frag");
	Shader[EFT_MULTIBROT_2][ESS_MS4] = CShaderLoader::loadShader("QuadCopyUV.glsl", "Multibrot2-4x4MS.frag");
		
	//Shader[EFT_JULIA][ESS_MS3] = CShaderLoader::loadShader("QuadCopyUV.glsl", "Multibrot1.frag");

	STextureCreationFlags Flags;
	Flags.Wrap = GL_MIRRORED_REPEAT;
	Flags.MipMaps = false;
		
	ColorMaps.push_back(new CTexture(CImageLoader::LoadImage("Spectrum1.bmp"), Flags));
	ColorMaps.push_back(new CTexture(CImageLoader::LoadImage("Spectrum2.bmp"), Flags));
	ColorMaps.push_back(new CTexture(CImageLoader::LoadImage("Quadrant1.bmp"), Flags));
	ColorMaps.push_back(new CTexture(CImageLoader::LoadImage("Quadrant2.bmp"), Flags));
}

void CMainState::Update(f32 const Elapsed)
{
	SRenderPass Pass;
	Pass.Shader = Shader[CurrentFractal][CurrentSettings];

	if (Pass.Shader)
	{
		Pass.Doubles["cX"] = cX;
		Pass.Doubles["cY"] = cY;
		Pass.Doubles["sX"] = sX;
		Pass.Doubles["sY"] = sY;
		Pass.Floats["TextureScaling"] = TextureScaling;
		Pass.Ints["max_iteration"] = max_iteration;
		Pass.Textures["uColorMap"] = ColorMaps[CurrentColor];
		Pass.Vector3s["uSetColor"] = uSetColor;

		if (CurrentSettings != ESS_DEFAULT)
		{
			Pass.Ints["uScreenWidth"] = Application->GetWindow().GetSize().X;
			Pass.Ints["uScreenHeight"] = Application->GetWindow().GetSize().Y;
		}

		Pass.DoPass();
	}

	Application->GetWindow().SwapBuffers();
}
