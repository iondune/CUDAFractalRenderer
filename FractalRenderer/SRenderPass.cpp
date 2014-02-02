
#include "SRenderPass.h"


GLuint SRenderPass::QuadHandle = 0;

SRenderPass::SRenderPass()
	: Shader(0), Context(0)
{
	if (! QuadHandle)
	{
		GLfloat QuadVertices[] =
		{
			-1.0, -1.0,
			 1.0, -1.0,
			 1.0,  1.0,
			-1.0,  1.0
		};

		glGenBuffers(1, & QuadHandle);
		glBindBuffer(GL_ARRAY_BUFFER, QuadHandle);
		glBufferData(GL_ARRAY_BUFFER, sizeof(QuadVertices), QuadVertices, GL_STATIC_DRAW);
	}
}

void SRenderPass::Begin()
{
	if (! Shader)
		return;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);

	Context = new CShaderContext(* Shader);
}

void SRenderPass::End()
{
	if (! Context)
		Begin();

	if (! Context)
		return;

	for (std::map<std::string, u32>::iterator it = Textures.begin(); it != Textures.end(); ++ it)
		Context->bindTexture(it->first, it->second);

	for (std::map<std::string, float>::iterator it = Floats.begin(); it != Floats.end(); ++ it)
		Context->uniform(it->first, it->second);

	for (std::map<std::string, double>::iterator it = Doubles.begin(); it != Doubles.end(); ++ it)
		Context->uniform<f64>(it->first, it->second);

	for (std::map<std::string, int>::iterator it = Ints.begin(); it != Ints.end(); ++ it)
		Context->uniform(it->first, it->second);

	for (std::map<std::string, vec3f>::iterator it = Vector3s.begin(); it != Vector3s.end(); ++ it)
		Context->uniform(it->first, it->second);

	Context->bindBufferObject("aPosition", QuadHandle, 2);

	glDrawArrays(GL_QUADS, 0, 4);

	if (SetTarget)
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glEnable(GL_DEPTH_TEST);

	delete Context;
}

void SRenderPass::DoPass()
{
	Begin();
	End();
}
