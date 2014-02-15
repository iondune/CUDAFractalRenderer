
#include "CMainState.h"
#include "SRenderPass.h"
#include "CudaVec2.cuh"
#include <cuda_gl_interop.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>


CMainState::CMainState()
	: CurrentColor(0), DumpFrames(false), CurrentDumpFrame(0), RenderZoom(false), LastRotation(0), ShowText(true), Resource(0)
{}

void CMainState::Begin()
{
	Font.init("Media/OpenSans.ttf", 16);
	glClearColor(0.6f, 0.6f, 0.6f, 1.0f);
	vec2i const ScreenSize = Application->GetWindow().GetSize();

	// Texture and shader for draw operations
	Finalize = CShaderLoader::loadShader("QuadCopyUV.glsl", "Finalize.frag");
	CopyTexture = new CTexture(ScreenSize, false);

	int Count;
	cudaGetDeviceCount(& Count);
	printf("There are %d devices.\n", Count);
	CheckedCudaCall(cudaGLSetGLDevice(0), "SetGLDevice");

	// Pixel Unpack Buffer for CUDA draw operations
	glGenBuffers(1, & CudaDrawBufferHandle);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, CudaDrawBufferHandle);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, ScreenSize.X * ScreenSize.Y * 4, NULL, GL_DYNAMIC_COPY);
	CheckedCudaCall(cudaGraphicsGLRegisterBuffer(& Resource, CudaDrawBufferHandle, cudaGraphicsRegisterFlagsWriteDiscard), "RegisterBuffer");
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

bool CMainState::IsRenderReady() const
{
	return FractalRenderer.GetIterationMax() == FractalRenderer.Params.IterationMax;
}

void CMainState::Update(f32 const Elapsed)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	FrameRateCounter.Update(Elapsed);

	DoRender();
	DrawRenderToScreen();

	if (DumpFrames)
	{
		DumpFrameToFile();

		if (IsRenderReady())
			DumpFrames = false;
	}
	else if (RenderZoom)
	{
		if (IsRenderReady())
		{
			static f64 const ZoomSpeed = 0.995;
			static f64 const RotateSpeed = 0.001;

			DumpFrameToFile();
			
			FractalRenderer.Params.Scale.X *= ZoomSpeed;
			FractalRenderer.Params.Scale.Y *= ZoomSpeed;
			FractalRenderer.Params.SetRotation(LastRotation += RotateSpeed);
			FractalRenderer.Reset();
		}
	}

	PrintTextOverlay();
	Application->GetWindow().SwapBuffers();
}

void CMainState::DoRender()
{
	void * DeviceBuffer;

	CheckedCudaCall(cudaGraphicsMapResources(1, & Resource), "MapResource");
	size_t Size;
	CheckedCudaCall(cudaGraphicsResourceGetMappedPointer(& DeviceBuffer, & Size, Resource), "GetMappedPointer");
	FractalRenderer.Render(DeviceBuffer);
	CheckedCudaCall(cudaGraphicsUnmapResources(1, & Resource), "UnmapResources");
}

void CMainState::DrawRenderToScreen()
{
	LoadTextureData();
	DoTextureDraw();
}

void CMainState::LoadTextureData()
{
	BindTexture();

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, CudaDrawBufferHandle);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, FractalRenderer.Params.ScreenSize.X, FractalRenderer.Params.ScreenSize.Y, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

void CMainState::BindTexture()
{
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ScreenTextureHandle);
}

void CMainState::DoTextureDraw()
{
	SRenderPass Pass;
	Pass.Shader = Finalize;
	if (Pass.Shader)
	{
		Pass.Textures["uColorMap"] = ScreenTextureHandle;
		Pass.DoPass();
	}
}

void CMainState::DumpFrameToFile()
{
	u32 const FrameWidth = Application->GetWindow().GetSize().X;
	u32 const FrameHeight = Application->GetWindow().GetSize().Y;

	unsigned char * ImageData = new unsigned char[FrameWidth * FrameHeight * 3];
	glReadPixels(0, 0, FrameWidth, FrameHeight, GL_RGB, GL_UNSIGNED_BYTE, ImageData);

	std::stringstream FileName;
	FileName << "OutputImages/";
	FileName << std::setw(5) << std::setfill('0') << CurrentDumpFrame ++;
	FileName << ".png";

	stbi_write_png(FileName.str().c_str(), FrameWidth, FrameHeight, 3, ImageData, FrameWidth * 3);
	delete [] ImageData;
}

void CMainState::PrintTextOverlay()
{
	if (! ShowText)
		return;

	freetype::print(Font, 10, 10, "FPS: %.3f", FrameRateCounter.GetAverage());
	freetype::print(Font, 10, 40, "Max: %d of %d", FractalRenderer.GetIterationMax(), FractalRenderer.Params.IterationMax);
	freetype::print(Font, 10, 70, "Increment: %d", FractalRenderer.IterationIncrement);
}

void CMainState::PrintLocation()
{
	printf("sX: %.7f   sY: %.7f   cX: %.15f   cY: %.15f\n",
		FractalRenderer.Params.Scale.X, FractalRenderer.Params.Scale.Y,
		FractalRenderer.Params.Center.X, FractalRenderer.Params.Center.Y);
}
