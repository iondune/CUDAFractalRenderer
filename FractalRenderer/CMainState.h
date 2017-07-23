
#pragma once

#include <ionEngine.h>
#include "CudaFractalRenderer.cuh"
#include "CFrameRateCounter.h"


class CMainState : public Singleton<CMainState>, public ion::CDefaultApplication
{

public:

	CMainState();

	void Run();

	void Begin();
	void Update(f32 const Elapsed);

	void OnEvent(ion::SKeyboardEvent & Event);
	void OnEvent(ion::SMouseEvent & Event);

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

	u32 CudaDrawBufferHandle, ScreenTextureHandle;
	cudaGraphicsResource * Resource;
	SharedPointer<ion::Graphics::IShaderProgram> Finalize;
	SharedPointer<ion::Graphics::ITexture2D> CopyTexture;

	std::vector<ion::CImage *> ColorMaps;
	int CurrentColor;
	int Multisample;

	bool ShowText;

	vec2i RenderSize;

private:

	SingletonPointer<ion::CWindowManager> WindowManager;
	SingletonPointer<ion::CTimeManager> TimeManager;
	SingletonPointer<ion::CAssetManager> AssetManager;
	SingletonPointer<ion::CSceneManager> SceneManager;
	SingletonPointer<ion::CGUIManager> GUIManager;

	SingletonPointer<ion::CGraphicsAPI> GraphicsAPI;
	SharedPointer<ion::Graphics::IGraphicsContext> GraphicsContext;

	ion::CWindow * Window = nullptr;
	SharedPointer<ion::Graphics::IRenderTarget> BackBuffer;

};
