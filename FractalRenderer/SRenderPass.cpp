
#include "SRenderPass.h"
#include <ionGraphicsGL/Utilities.h>

using namespace ion;
using namespace ion::Graphics;


SharedPointer<ion::Graphics::IVertexBuffer> VertexBuffer;
SharedPointer<ion::Graphics::IIndexBuffer> IndexBuffer;

SRenderPass::SRenderPass(SharedPointer<ion::Graphics::IGraphicsContext> Graphics)
	: Shader(0), Context(0)
{
	this->Graphics = Graphics;

	if (! VertexBuffer)
	{
		vector<GLfloat> QuadVertices =
		{
			-1.0, -1.0,
			 1.0, -1.0,
			 1.0,  1.0,
			-1.0,  1.0,
		};

		vector<uint> QuadIndices =
		{
			0, 1, 2,
			0, 2, 3
		};

		Graphics::SInputLayoutElement InputLayout[] =
		{
			{ "aPosition", 2, Graphics::EAttributeType::Float },
		};
		VertexBuffer = GraphicsAPI->CreateVertexBuffer();
		VertexBuffer->SetInputLayout(InputLayout, ION_ARRAYSIZE(InputLayout));
		VertexBuffer->UploadData(QuadVertices);
		IndexBuffer = GraphicsAPI->CreateIndexBuffer();
		IndexBuffer->UploadData(QuadIndices);
	}
}

void SRenderPass::Begin()
{
	if (! Shader)
		return;

	CheckedGLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	Context = Graphics->CreatePipelineState();
	Context->SetProgram(Shader);
}

void SRenderPass::End()
{
	if (! Context)
		Begin();

	if (! Context)
		return;

	for (std::map<std::string, float>::iterator it = Floats.begin(); it != Floats.end(); ++ it)
		Context->SetUniform(it->first, CUniform<float>(it->second));

	for (std::map<std::string, int>::iterator it = Ints.begin(); it != Ints.end(); ++ it)
		Context->SetUniform(it->first, CUniform<int>(it->second));

	for (std::map<std::string, vec3f>::iterator it = Vector3s.begin(); it != Vector3s.end(); ++ it)
		Context->SetUniform(it->first, CUniform<vec3f>(it->second));

	Context->SetVertexBuffer(0, VertexBuffer);
	Context->SetIndexBuffer(IndexBuffer);

	int i = 0;
	for (auto it = Textures.begin(); it != Textures.end(); ++ it)
	{
		CheckedGLCall(glActiveTexture(GL_TEXTURE0 + i));
		CheckedGLCall(glBindTexture(GL_TEXTURE_2D, it->second));
		Context->SetUniform(it->first, CUniform<int>(i));

		i++;
	}

	Context->SetFeatureEnabled(EDrawFeature::DisableDepthTest, true);

	Graphics->Draw(Context);

	if (SetTarget)
		CheckedGLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void SRenderPass::DoPass()
{
	Begin();
	End();
}
