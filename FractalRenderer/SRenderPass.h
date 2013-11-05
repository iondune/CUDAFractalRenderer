
#pragma once

#include <ionEngine.h>

class SRenderPass
{

public:

	SRenderPass();

	CShader * Shader;

	std::map<std::string, float> Floats;
	std::map<std::string, double> Doubles;
	std::map<std::string, int> Ints;
	std::map<std::string, vec3f> Vector3s;
	std::map<std::string, CTexture *> Textures;

	void DoPass();

	void Begin();
	void End();

	bool SetTarget;

	CShaderContext * Context;
	static GLuint QuadHandle;

};
