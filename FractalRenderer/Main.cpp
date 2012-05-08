#include <CabbageCore.h>
#include <CabbageScene.h>
#include <CabbageFramework.h>

#ifdef __unix__
//#include <GL/gl.h>
//#include <GL/glu.h>

#endif

#ifdef _WIN32
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "SDL.lib")
#pragma comment(lib, "SDLmain.lib")
#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "freetype.lib")

#include <SDL/SDL.h>
#endif

#include <GL/glew.h>
#include <SDL/SDL.h>

class CMainState : public CState<CMainState>
{

	CShader * Shader, * ShaderMS2, * ShaderMS3, * ShaderStoch, * ShaderStoch2;
	CTexture * ColorMap;
	SVector3 uSetColor;
	
	float TextureScaling;
	int ScaleFactor;
	
	void recalcScale()
	{
		if (-1 <= ScaleFactor && ScaleFactor <= 1)
			TextureScaling = 1.f;
		else if (ScaleFactor < -1)
		{
			TextureScaling = -ScaleFactor / 1.f;
		}
		else if (ScaleFactor > 1)
		{
			TextureScaling = 1.f / ScaleFactor;
		}
	}

public:

	static GLuint QuadHandle;

	CMainState()
		: sX(1.0), sY(1.0), cX(0.0), cY(0.7), max_iteration(1000), uSetColor(0.0f), Multisample(false), ScaleFactor(1), TextureScaling(1.f)
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
		ShaderMS2 = CShaderLoader::loadShader("QuadCopyUV.glsl", "Mandelbrot1-2x2MS.frag");
		ShaderMS3 = CShaderLoader::loadShader("QuadCopyUV.glsl", "Mandelbrot1-3x3MS.frag");
		ShaderStoch = CShaderLoader::loadShader("QuadCopyUV.glsl", "Mandelbrot1-Stoch.frag");
		ShaderStoch2 = CShaderLoader::loadShader("QuadCopyUV.glsl", "Mandelbrot1-2x2Stoch.frag");

		CImage * ColorImage = CTextureLoader::loadImage("Spectrum1.bmp");

		STextureCreationFlags Flags;
		Flags.MipMaps = false;
		ColorMap = new CTexture(ColorImage, Flags);
	}

	class SPostProcessPass
	{

	public:

		SPostProcessPass();

		CShader * Shader;

		std::map<std::string, float> Floats;
		std::map<std::string, double> Doubles;
		std::map<std::string, int> Ints;
		std::map<std::string, SVector3> Vector3s;
		std::map<std::string, CTexture *> Textures;

		void doPass();

		void begin();
		void end();

		bool SetTarget;

		CShaderContext * Context;

	};

	double sX, sY;
	double cX, cY;
	int max_iteration;

	int Multisample;

	void OnRenderStart(float const Elapsed)
	{
		SPostProcessPass Pass;

		switch (Multisample)
		{
		case 0:
			Pass.Shader = Shader;
			break;

		case 1:
			Pass.Shader = ShaderMS2;
			break;

		case 2:
			Pass.Shader = ShaderMS3;
			break;

		case 3:
			Pass.Shader = ShaderStoch;
			break;

		case 4:
			Pass.Shader = ShaderStoch2;
			break;
		}


		Pass.Doubles["cX"] = cX;
		Pass.Doubles["cY"] = cY;
		Pass.Doubles["sX"] = sX;
		Pass.Doubles["sY"] = sY;
		Pass.Floats["TextureScaling"] = TextureScaling;
		Pass.Ints["max_iteration"] = max_iteration;
		Pass.Textures["uColorMap"] = ColorMap;
		Pass.Vector3s["uSetColor"] = uSetColor;

		if (Multisample)
		{
			Pass.Ints["uScreenWidth"] = Application.getWindowSize().X;
			Pass.Ints["uScreenHeight"] = Application.getWindowSize().Y;
		}

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

			case SDLK_m:

				++ Multisample;
				if (Multisample > 4)
					Multisample = 0;
				std::cout << "Multisample: " << Multisample << std::endl;
				break;

			case SDLK_1:

				Multisample = 0;
				std::cout << "Multisample: " << Multisample << std::endl;
				break;

			case SDLK_2:

				Multisample = 1;
				std::cout << "Multisample: " << Multisample << std::endl;
				break;

			case SDLK_3:

				Multisample = 2;
				std::cout << "Multisample: " << Multisample << std::endl;
				break;

			case SDLK_4:

				Multisample = 3;
				std::cout << "Multisample: " << Multisample << std::endl;
				break;

			case SDLK_5:

				Multisample = 4;
				std::cout << "Multisample: " << Multisample << std::endl;
				break;

			case SDLK_z:

				sX *= 0.5;
				sY *= 0.5;
				break;

			case SDLK_x:

				sX *= 2.0;
				sY *= 2.0;
				break;
				
			case SDLK_g:
				
				if (max_iteration)
					max_iteration *= 2;
				else
					++ max_iteration;
				
				printf("iteration cap: %d\n", max_iteration);
				
				break;

			case SDLK_b:
				
				max_iteration /= 2;
				
				printf("iteration cap: %d\n", max_iteration);
				
				break;
				
			case SDLK_h:
				
				ScaleFactor --;
				recalcScale();
				
				break;
				
				case SDLK_n:
				
				ScaleFactor ++;
				recalcScale();
				
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

	for (std::map<std::string, SVector3>::iterator it = Vector3s.begin(); it != Vector3s.end(); ++ it)
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

	Application.getStateManager().setState(& CMainState::get());

	Application.run();

	return 0;
}
