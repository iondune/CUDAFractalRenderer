#include <CabbageCore.h>
#include <CabbageScene.h>
#include <CabbageFramework.h>

#ifdef __unix__
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL/SDL.h>
#endif

#ifdef _WIN32
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "SDL.lib")
#pragma comment(lib, "SDLmain.lib")
#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "freetype.lib")

#include <GL/glew.h>
#include <SDL/SDL.h>
#endif

class CMainState : public CState<CMainState>
{

	CShader * Shader;

public:

	static GLuint QuadHandle;

	CMainState()
		: sX(1.0), sY(1.0), cX(0.0), cY(0.7)
	{}

	void begin()
	{
		SDL_WM_SetCaption("Fractal Renderer!", "");

		glClearColor(0.6f, 0.6f, 0.6f, 1.0f);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

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

		Shader = CShaderLoader::loadShader("QuadCopyUV.glsl", "Mandelbrot1.frag");
	}

	class SPostProcessPass
	{

	public:

		SPostProcessPass();

		CShader * Shader;

		std::map<std::string, float> Floats;
		std::map<std::string, double> Doubles;
		std::map<std::string, int> Ints;
		std::map<std::string, CTexture *> Textures;

		void doPass();

		void begin();
		void end();

		bool SetTarget;

		CShaderContext * Context;

	};

	double sX, sY;
	double cX, cY;

	void OnRenderStart(float const Elapsed)
	{
		SPostProcessPass Pass;
		Pass.Shader = Shader;
		Pass.Doubles["cX"] = cX;
		Pass.Doubles["cY"] = cY;
		Pass.Doubles["sX"] = sX;
		Pass.Doubles["sY"] = sY;
		Pass.doPass();

		SDL_GL_SwapBuffers();
	}


	void OnKeyboardEvent(SKeyboardEvent const & Event)
	{
		if (! Event.Pressed)
		{
			static double const MoveSpeed = 0.07;
			switch (Event.Key)
			{

			case SDLK_w:

				cY += sY * MoveSpeed;
				break;

			case SDLK_a:

				cX -= sX * MoveSpeed;
				break;

			case SDLK_s:

				cY -= sY * MoveSpeed;
				break;

			case SDLK_d:

				cX += sX * MoveSpeed;
				break;

			case SDLK_z:

				sX *= 0.5;
				sY *= 0.5;
				break;

			case SDLK_x:

				sX *= 2.0;
				sY *= 2.0;
				break;

			}
		}
	}

	void OnMouseEvent(SMouseEvent const & Event)
	{
		switch (Event.Type.Value)
		{

		case SMouseEvent::EType::Click:

			break;

		case SMouseEvent::EType::Move:

			break;

		}
	}

};

GLuint CMainState::QuadHandle = 0;

CMainState::SPostProcessPass::SPostProcessPass()
	: Shader(0), Context(0)
{}

void CMainState::SPostProcessPass::begin()
{
	if (! Shader)
		return;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
	glDisable(GL_DEPTH_TEST);

	Context = new CShaderContext(* Shader);
}

void CMainState::SPostProcessPass::end()
{
	if (! Context)
		begin();

	if (! Context)
		return;

	for (std::map<std::string, CTexture *>::iterator it = Textures.begin(); it != Textures.end(); ++ it)
		Context->bindTexture(it->first, it->second);

	for (std::map<std::string, float>::iterator it = Floats.begin(); it != Floats.end(); ++ it)
		Context->uniform(it->first, it->second);

	for (std::map<std::string, double>::iterator it = Doubles.begin(); it != Doubles.end(); ++ it)
		Context->uniform(it->first, it->second);

	for (std::map<std::string, int>::iterator it = Ints.begin(); it != Ints.end(); ++ it)
		Context->uniform(it->first, it->second);

	Context->bindBufferObject("aPosition", QuadHandle, 2);

	glDrawArrays(GL_QUADS, 0, 4);

	if (SetTarget)
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glEnable(GL_DEPTH_TEST);

	delete Context;
}

void CMainState::SPostProcessPass::doPass()
{
	begin();
	end();
}

int main(int argc, char * argv[])
{
	CTextureLoader::ImageDirectory = "Media/";
	CMeshLoader::MeshDirectory = "Media/";
	CShaderLoader::ShaderDirectory = "Shaders/";

	CApplication & Application = CApplication::get();
	Application.init(SPosition2(900, 900));

	if (! GLEW_ARB_gpu_shader_fp64)
	{
		std::cout << "FUCK WE DON'T HAVE DOUBLES IN SHADERS FUCK" << std::endl;
	}

	Application.getStateManager().setState(& CMainState::get());

	Application.run();

	return 0;
}
