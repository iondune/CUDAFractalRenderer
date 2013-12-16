
#include "CMainState.h"
#include "SRenderPass.h"


CMainState::CMainState()
	: sX(1.0), sY(1.0), cX(0.0), cY(0.7), max_iteration(1000), uSetColor(0.0f), ScaleFactor(1), TextureScaling(1.f),
	CurrentFractal(EFT_MANDEL), CurrentSettings(ESS_DEFAULT), CurrentColor(0), SetColorCounter(0), FractalRenderer(0)
{
	sX *= Application->GetWindow().GetAspectRatio();
}

void CMainState::Begin()
{
	glClearColor(0.6f, 0.6f, 0.6f, 1.0f);

	Finalize = CShaderLoader::loadShader("QuadCopyUV.glsl", "Finalize.frag");
	CopyTexture = new CTexture(Application->GetWindow().GetSize(), false);
}

void CMainState::Reset()
{
	SFractalParams Params;
	Params.Center = cvec2d(cX, cY);
	Params.Scale = cvec2d(sX, sY);
	Params.IterationMax = max_iteration;
	Params.ScreenSize = cvec2u(Application->GetWindow().GetSize().X, Application->GetWindow().GetSize().Y);
	
	FractalRenderer->Reset(Params);
}

void CMainState::Update(f32 const Elapsed)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	SFractalParams Params;
	Params.Center = cvec2d(cX, cY);
	Params.Scale = cvec2d(sX, sY);
	Params.IterationMax = max_iteration;
	Params.ScreenSize = cvec2u(Application->GetWindow().GetSize().X, Application->GetWindow().GetSize().Y);

	if (! FractalRenderer)
		FractalRenderer = new CudaFractalRenderer(Params);

	u8 const * const Image = FractalRenderer->Render(Params);
	glBindTexture(GL_TEXTURE_2D, CopyTexture->getTextureHandle());
	glTexSubImage2D(
		GL_TEXTURE_2D, 0, 0, 0, 
		Application->GetWindow().GetSize().X, Application->GetWindow().GetSize().Y,
		GL_RGB, GL_UNSIGNED_BYTE, Image);
			
	SRenderPass Pass;
	Pass.Shader = Finalize;
	if (Pass.Shader)
	{
		Pass.Textures["uColorMap"] = CopyTexture;
		Pass.DoPass();
	}
	Application->GetWindow().SwapBuffers();
}
