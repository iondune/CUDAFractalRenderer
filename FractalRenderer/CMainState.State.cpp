
#include "CMainState.h"
#include "SRenderPass.h"


#include <cuda.h>
#include <cuda_gl_interop.h>

CMainState::CMainState()
	: sX(1.0), sY(1.0), cX(0.0), cY(0.7), max_iteration(1000), uSetColor(0.0f), ScaleFactor(1), TextureScaling(1.f),
	CurrentFractal(EFT_MANDEL), CurrentSettings(ESS_DEFAULT), CurrentColor(0), SetColorCounter(0), FractalRenderer(0)
{
	sX *= Application->GetWindow().GetAspectRatio();
}

void CMainState::Begin()
{
	Font.init("Media/OpenSans.ttf", 16);

	glClearColor(0.6f, 0.6f, 0.6f, 1.0f);

	Finalize = CShaderLoader::loadShader("QuadCopyUV.glsl", "Finalize.frag");
	CopyTexture = new CTexture(Application->GetWindow().GetSize(), false);
	vec2i const ScreenSize = Application->GetWindow().GetSize();

	// Pixel Unpack Buffer for CUDA draw operations
	glGenBuffers(1, & CudaDrawBufferHandle);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, CudaDrawBufferHandle);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, ScreenSize.X * ScreenSize.Y * 4, NULL, GL_DYNAMIC_COPY);
	cudaGLRegisterBufferObject(CudaDrawBufferHandle);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	// Texture for screen copy
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, & ScreenTextureHandle);
	glBindTexture(GL_TEXTURE_2D, ScreenTextureHandle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ScreenSize.X, ScreenSize.Y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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
	FrameRateCounter.Update(Elapsed);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	SFractalParams Params;
	Params.Center = cvec2d(cX, cY);
	Params.Scale = cvec2d(sX, sY);
	Params.IterationMax = max_iteration;
	Params.ScreenSize = cvec2u(Application->GetWindow().GetSize().X, Application->GetWindow().GetSize().Y);

	if (! FractalRenderer)
		FractalRenderer = new CudaFractalRenderer(Params);
	
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ScreenTextureHandle);

	void * deviceBuffer;
	cudaGLMapBufferObject(& deviceBuffer, CudaDrawBufferHandle);
	FractalRenderer->Render(deviceBuffer, Params);
	cudaGLUnmapBufferObject(CudaDrawBufferHandle);

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, CudaDrawBufferHandle);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, Params.ScreenSize.X, Params.ScreenSize.Y, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
			
	SRenderPass Pass;
	Pass.Shader = Finalize;
	if (Pass.Shader)
	{
		Pass.Textures["uColorMap"] = ScreenTextureHandle;
		Pass.DoPass();
	}
	
	freetype::print(Font, 10, 10, "FPS: %.3f", FrameRateCounter.GetAverage());
	Application->GetWindow().SwapBuffers();
}
