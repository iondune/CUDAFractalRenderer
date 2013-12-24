
#include "CMainState.h"
#include "SRenderPass.h"

#include <cuda.h>
#include <cuda_gl_interop.h>


CMainState::CMainState()
	: uSetColor(0.0f), ScaleFactor(1), TextureScaling(1.f),
	CurrentFractal(EFT_MANDEL), CurrentSettings(ESS_DEFAULT), CurrentColor(0), SetColorCounter(0),
	DumpFrames(false)
{}

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

	FractalRenderer.Init(cvec2u(Application->GetWindow().GetSize().X, Application->GetWindow().GetSize().Y));
}

void CMainState::Update(f32 const Elapsed)
{
	FrameRateCounter.Update(Elapsed);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ScreenTextureHandle);

	void * deviceBuffer;
	cudaGLMapBufferObject(& deviceBuffer, CudaDrawBufferHandle);
	FractalRenderer.Render(deviceBuffer);
	cudaGLUnmapBufferObject(CudaDrawBufferHandle);

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, CudaDrawBufferHandle);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, FractalRenderer.Params.ScreenSize.X, FractalRenderer.Params.ScreenSize.Y, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
			
	SRenderPass Pass;
	Pass.Shader = Finalize;
	if (Pass.Shader)
	{
		Pass.Textures["uColorMap"] = ScreenTextureHandle;
		Pass.DoPass();
	}

	// Dump Frame
	if (DumpFrames)
	{
		u32 const FrameWidth = Application->GetWindow().GetSize().X;
		u32 const FrameHeight = Application->GetWindow().GetSize().Y;
		unsigned char * ImageData = new unsigned char[FrameWidth * FrameHeight * 3];

		static u32 Counter = 0;
		glReadPixels(0, 0, FrameWidth, FrameHeight, GL_RGB, GL_UNSIGNED_BYTE, ImageData);
		CImage * Image = new CImage(ImageData, FrameWidth, FrameHeight, false);
		std::stringstream Stream;
		Stream << "OutputImages/";
		Stream << std::setw(5) << std::setfill('0') << Counter++;
		Stream << ".bmp";
		Image->Write(Stream.str());
		delete Image;
		if (FractalRenderer.GetIterationMax() == FractalRenderer.Params.IterationMax)
			DumpFrames = false;
	}
	
	freetype::print(Font, 10, 10, "FPS: %.3f", FrameRateCounter.GetAverage());
	freetype::print(Font, 10, 40, "Max: %d of %d", FractalRenderer.GetIterationMax(), FractalRenderer.Params.IterationMax);
	freetype::print(Font, 10, 70, "Increment: %d", FractalRenderer.IterationIncrement);
	Application->GetWindow().SwapBuffers();
}
