
#include "CMainState.h"
#include "SRenderPass.h"
#include "CudaVec2.cuh"
#include <cuda_gl_interop.h>
#include <stb_image_write.h>
#include <ionGraphicsGL/Utilities.h>

using namespace ion;


CMainState::CMainState()
	: CurrentColor(0), DumpFrames(false), CurrentDumpFrame(0), RenderZoom(false), LastRotation(0), ShowText(true), Resource(0)
{}

void CMainState::Run()
{
	GraphicsAPI->Init(new Graphics::COpenGLImplementation());
	WindowManager->Init(GraphicsAPI);
	TimeManager->Init(WindowManager);

	LoadSettings();
	Window = WindowManager->CreateWindow(ApplicationSettings.WindowSize, "Fractal Renderer", ApplicationSettings.WindowType);
	Window->SetPosition(ApplicationSettings.WindowPosition);

	SingletonPointer<CGamePad> GamePad;
	GamePad->AddListener(this);

	GraphicsContext = Window->GetContext();
	BackBuffer = GraphicsContext->GetBackBuffer();
	BackBuffer->SetClearColor(color4i(128, 128, 128, 255));

	GUIManager->Init(Window);
	GUIManager->AddFontFromFile("Assets/Fonts/OpenSans.ttf", 20.f);
	Window->AddListener(GUIManager);
	GUIManager->AddListener(this);

	SceneManager->Init(GraphicsAPI);

	AssetManager->Init(GraphicsAPI);
	AssetManager->AddAssetPath(".");
	AssetManager->SetShaderPath("Shaders");
	AssetManager->SetTexturePath("Media");
	AssetManager->SetMeshPath("Media");

	Begin();


	TimeManager->Start();
	while (WindowManager->Run())
	{
		TimeManager->Update();

		GUIManager->NewFrame();

		BackBuffer->ClearColorAndDepth();

		Update((float) TimeManager->GetElapsedTime());

		GUIManager->Draw();
		Window->SwapBuffers();
	}
}

void CMainState::Begin()
{
	//Font.init("Media/OpenSans.ttf", 16);
	glClearColor(0.6f, 0.6f, 0.6f, 1.0f);
	RenderSize = Window->GetSize();

	// Texture and shader for draw operations
	Finalize = AssetManager->LoadShader("Finalize");
	CopyTexture = GraphicsAPI->CreateTexture2D(RenderSize, Graphics::ITexture::EMipMaps::False, Graphics::ITexture::EFormatComponents::RGBA, Graphics::ITexture::EInternalFormatType::Fix8);

	int Count;
	cudaGetDeviceCount(& Count);
	printf("There are %d devices.\n", Count);
	CheckedCudaCall(cudaGLSetGLDevice(0), "SetGLDevice");

	// Pixel Unpack Buffer for CUDA draw operations
	glGenBuffers(1, & CudaDrawBufferHandle);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, CudaDrawBufferHandle);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, RenderSize.X * RenderSize.Y * 4, NULL, GL_DYNAMIC_COPY);
	CheckedCudaCall(cudaGraphicsGLRegisterBuffer(& Resource, CudaDrawBufferHandle, cudaGraphicsRegisterFlagsWriteDiscard), "RegisterBuffer");
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	// Texture for screen copy
	//CheckedGLCall(glEnable(GL_TEXTURE_2D));
	CheckedGLCall(glGenTextures(1, & ScreenTextureHandle));
	CheckedGLCall(glBindTexture(GL_TEXTURE_2D, ScreenTextureHandle));
	CheckedGLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, RenderSize.X, RenderSize.Y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));
	CheckedGLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	CheckedGLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

	FractalRenderer.Init(cvec2u(RenderSize.X, RenderSize.Y));
}

bool CMainState::IsRenderReady() const
{
	return FractalRenderer.GetIterationMax() == FractalRenderer.Params.IterationMax;
}

void CMainState::Update(f32 const Elapsed)
{
	CheckedGLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	FrameRateCounter.Update(Elapsed);

	DoRender();
	DrawRenderToScreen();

	if (DumpFrames)
	{
		if (IsRenderReady())
		{
			DumpFrameToFile();
			DumpFrames = false;
		}
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

	CheckedGLCall(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, CudaDrawBufferHandle));
	CheckedGLCall(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, FractalRenderer.Params.ScreenSize.X, FractalRenderer.Params.ScreenSize.Y, GL_RGBA, GL_UNSIGNED_BYTE, NULL));
	CheckedGLCall(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
}

void CMainState::BindTexture()
{
	//CheckedGLCall(glEnable(GL_TEXTURE_2D));
	CheckedGLCall(glActiveTexture(GL_TEXTURE0));
	CheckedGLCall(glBindTexture(GL_TEXTURE_2D, ScreenTextureHandle));
}

void CMainState::DoTextureDraw()
{
	SRenderPass Pass(Window->GetContext());
	Pass.Shader = Finalize;
	if (Pass.Shader)
	{
		Pass.Textures["uColorMap"] = ScreenTextureHandle;
		Pass.DoPass();
	}
}

void CMainState::DumpFrameToFile()
{
	u32 const FrameWidth = RenderSize.X;
	u32 const FrameHeight = RenderSize.Y;

	unsigned char * ImageData = new unsigned char[FrameWidth * FrameHeight * 3];
	CheckedGLCall(glReadPixels(0, 0, FrameWidth, FrameHeight, GL_RGB, GL_UNSIGNED_BYTE, ImageData));

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

	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiSetCond_Once);
	if (ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("FPS: %.3f", FrameRateCounter.GetAverage());
		ImGui::Text("Max: %d of %d", FractalRenderer.GetIterationMax(), FractalRenderer.Params.IterationMax);
		ImGui::Text("Increment: %d", FractalRenderer.IterationIncrement);
	}
	ImGui::End();
}

void CMainState::PrintLocation()
{
	printf("sX: %.7f   sY: %.7f   cX: %.15f   cY: %.15f\n",
		FractalRenderer.Params.Scale.X, FractalRenderer.Params.Scale.Y,
		FractalRenderer.Params.Center.X, FractalRenderer.Params.Center.Y);
}
