
#pragma once

#include <ionEngine.h>


class SRenderPass
{

public:

	SRenderPass(SharedPointer<ion::Graphics::IGraphicsContext> Graphics);

	SharedPointer<ion::Graphics::IShaderProgram> Shader;

	std::map<std::string, float> Floats;
	std::map<std::string, int> Ints;
	std::map<std::string, vec3f> Vector3s;
	std::map<std::string, u32> Textures;

	void DoPass();

	void Begin();
	void End();

	bool SetTarget;

	SharedPointer<ion::Graphics::IPipelineState> Context;
	SharedPointer<ion::Graphics::IGraphicsContext> Graphics;

	SingletonPointer<ion::CGraphicsAPI> GraphicsAPI;

};
