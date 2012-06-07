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
#endif

#include <GL/glew.h>
#include <SDL/SDL.h>

enum EFractalType
{
	EFT_MANDEL,
	EFT_BURNING_SHIP,
	EFT_TRICORN,
	EFT_MULTIBROT_1,
	EFT_MULTIBROT_2,
	EFT_JULIA,
	EFT_COUNT
};

enum EShaderSettings
{
	ESS_DEFAULT,
	ESS_MS2,
	ESS_MS3,
	ESS_MS4,
	ESS_STOCH,
	ESS_STOCH2,
	ESS_COUNT
};

class CMainState : public CState<CMainState>
{

	CShader * Shader[EFT_COUNT][ESS_COUNT];

	int CurrentFractal;
	int CurrentSettings;
	int CurrentColor;

	std::vector<CTexture *> ColorMaps;
	SVector3 uSetColor;
	int SetColorCounter;
	
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

	void setSetColor()
	{
		if (SetColorCounter > 8)
			SetColorCounter = 0;
		if (SetColorCounter < 0)
			SetColorCounter = 8;

		switch (SetColorCounter)
		{
		case 0:
			uSetColor = SVector3(0.f);
			break;
		case 1:
			uSetColor = SVector3(1.f);
			break;
		case 2:
			uSetColor = SVector3(0.9f, 0.2f, 0.1f);
			break;
		case 3:
			uSetColor = SVector3(0.2f, 0.9f, 0.1f);
			break;
		case 4:
			uSetColor = SVector3(0.1f, 0.2f, 0.9f);
			break;
		case 5:
			uSetColor = SVector3(0.9f, 0.8f, 0.1f);
			break;
		case 6:
			uSetColor = SVector3(0.8f, 0.6f, 0.4f);
			break;
		case 7:
			uSetColor = SVector3(0.4f, 0.6f, 0.8f);
			break;
		case 8:
			uSetColor = SVector3(0.9f, 0.3f, 0.8f);
			break;
		};
	}

public:

	static GLuint QuadHandle;

	CMainState()
		: sX(1.0), sY(1.0), cX(0.0), cY(0.7), max_iteration(1000), uSetColor(0.0f), ScaleFactor(1), TextureScaling(1.f),
		CurrentFractal(EFT_MANDEL), CurrentSettings(ESS_DEFAULT), CurrentColor(0), SetColorCounter(0)
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

		for (int i = 0; i < EFT_COUNT; ++ i)
		{
			for (int j = 0; j < ESS_COUNT; ++ j)
			{
				Shader[i][j] = 0;
			}
		}

		Shader[EFT_MANDEL][ESS_DEFAULT] = CShaderLoader::loadShader("QuadCopyUV.glsl", "Mandelbrot1.frag");
		Shader[EFT_MANDEL][ESS_MS2] = CShaderLoader::loadShader("QuadCopyUV.glsl", "Mandelbrot1-2x2MS.frag");
		Shader[EFT_MANDEL][ESS_MS3] = CShaderLoader::loadShader("QuadCopyUV.glsl", "Mandelbrot1-3x3MS.frag");
		Shader[EFT_MANDEL][ESS_MS4] = CShaderLoader::loadShader("QuadCopyUV.glsl", "Mandelbrot1-4x4MS.frag");
		Shader[EFT_MANDEL][ESS_STOCH] = CShaderLoader::loadShader("QuadCopyUV.glsl", "Mandelbrot1-Stoch.frag");
		Shader[EFT_MANDEL][ESS_STOCH2] = CShaderLoader::loadShader("QuadCopyUV.glsl", "Mandelbrot1-2x2Stoch.frag");

		Shader[EFT_BURNING_SHIP][ESS_DEFAULT] = CShaderLoader::loadShader("QuadCopyUV.glsl", "BurningShip.frag");
		Shader[EFT_BURNING_SHIP][ESS_MS2] = CShaderLoader::loadShader("QuadCopyUV.glsl", "BurningShip-2x2MS.frag");
		Shader[EFT_BURNING_SHIP][ESS_MS3] = CShaderLoader::loadShader("QuadCopyUV.glsl", "BurningShip-3x3MS.frag");
		Shader[EFT_BURNING_SHIP][ESS_MS4] = CShaderLoader::loadShader("QuadCopyUV.glsl", "BurningShip-4x4MS.frag");

		Shader[EFT_TRICORN][ESS_DEFAULT] = CShaderLoader::loadShader("QuadCopyUV.glsl", "Tricorn.frag");
		Shader[EFT_TRICORN][ESS_MS2] = CShaderLoader::loadShader("QuadCopyUV.glsl", "Tricorn-2x2MS.frag");
		Shader[EFT_TRICORN][ESS_MS3] = CShaderLoader::loadShader("QuadCopyUV.glsl", "Tricorn-3x3MS.frag");
		Shader[EFT_TRICORN][ESS_MS4] = CShaderLoader::loadShader("QuadCopyUV.glsl", "Tricorn-4x4MS.frag");
		
		Shader[EFT_MULTIBROT_1][ESS_DEFAULT] = CShaderLoader::loadShader("QuadCopyUV.glsl", "Multibrot1.frag");
		Shader[EFT_MULTIBROT_1][ESS_MS2] = CShaderLoader::loadShader("QuadCopyUV.glsl", "Multibrot1-2x2MS.frag");
		Shader[EFT_MULTIBROT_1][ESS_MS3] = CShaderLoader::loadShader("QuadCopyUV.glsl", "Multibrot1-3x3MS.frag");
		Shader[EFT_MULTIBROT_1][ESS_MS4] = CShaderLoader::loadShader("QuadCopyUV.glsl", "Multibrot1-4x4MS.frag");

		
		Shader[EFT_MULTIBROT_2][ESS_DEFAULT] = CShaderLoader::loadShader("QuadCopyUV.glsl", "Multibrot2.frag");
		
		//Shader[EFT_JULIA][ESS_MS3] = CShaderLoader::loadShader("QuadCopyUV.glsl", "Multibrot1.frag");

		STextureCreationFlags Flags;
		Flags.Wrap = GL_MIRRORED_REPEAT;
		Flags.MipMaps = false;
		
		ColorMaps.push_back(new CTexture(CTextureLoader::loadImage("Spectrum1.bmp"), Flags));
		ColorMaps.push_back(new CTexture(CTextureLoader::loadImage("Spectrum2.bmp"), Flags));
		ColorMaps.push_back(new CTexture(CTextureLoader::loadImage("Quadrant1.bmp"), Flags));
		ColorMaps.push_back(new CTexture(CTextureLoader::loadImage("Quadrant2.bmp"), Flags));
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

	void printLocation()
	{
		printf("sX: %f   sY: %f   cX: %f   cY: %f\n", sX, sY, cX, cY);
	}

	void OnRenderStart(float const Elapsed)
	{
		SPostProcessPass Pass;
		Pass.Shader = Shader[CurrentFractal][CurrentSettings];

		if (Pass.Shader)
		{
			Pass.Doubles["cX"] = cX;
			Pass.Doubles["cY"] = cY;
			Pass.Doubles["sX"] = sX;
			Pass.Doubles["sY"] = sY;
			Pass.Floats["TextureScaling"] = TextureScaling;
			Pass.Ints["max_iteration"] = max_iteration;
			Pass.Textures["uColorMap"] = ColorMaps[CurrentColor];
			Pass.Vector3s["uSetColor"] = uSetColor;

			if (CurrentSettings != ESS_DEFAULT)
			{
				Pass.Ints["uScreenWidth"] = Application.getWindowSize().X;
				Pass.Ints["uScreenHeight"] = Application.getWindowSize().Y;
			}

			Pass.doPass();
		}

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

				cY += (CurrentFractal == EFT_BURNING_SHIP ? -1 : 1) * sY * MoveSpeed;
				printLocation();
				break;

			case SDLK_a:

				cX -= sX * MoveSpeed;
				printLocation();
				break;

			case SDLK_s:

				cY -= (CurrentFractal == EFT_BURNING_SHIP ? -1 : 1) * sY * MoveSpeed;
				printLocation();
				break;

			case SDLK_d:

				cX += sX * MoveSpeed;
				printLocation();
				break;

			case SDLK_COMMA:

				++ CurrentFractal;
				if (CurrentFractal >= EFT_COUNT)
					CurrentFractal = 0;
				std::cout << "Fractal: " << CurrentFractal << std::endl;
				break;

			case SDLK_m:

				++ CurrentSettings;
				if (CurrentSettings >= ESS_COUNT)
					CurrentSettings = 0;
				std::cout << "Multisample: " << CurrentSettings << std::endl;
				break;

			case SDLK_1:
			case SDLK_2:
			case SDLK_3:
			case SDLK_4:
			case SDLK_5:
			case SDLK_6:
			case SDLK_7:
			case SDLK_8:

				CurrentSettings = Event.Key - SDLK_1;
				if (CurrentSettings >= ESS_COUNT)
					CurrentSettings = ESS_COUNT - 1;
				std::cout << "Multisample: " << CurrentSettings << std::endl;
				break;

			case SDLK_z:

				sX *= 0.5;
				sY *= 0.5;
				break;

			case SDLK_x:

				sX *= 2.0;
				sY *= 2.0;
				break;

			case SDLK_q:

				sX *= 0.75;
				sY *= 0.75;
				break;

			case SDLK_e:

				sX *= 1.33;
				sY *= 1.33;
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

			case SDLK_u:

				++ CurrentColor;
				if (CurrentColor >= (int) ColorMaps.size())
					CurrentColor = 0;

				break;

			case SDLK_j:

				-- CurrentColor;
				if (CurrentColor < 0)
					CurrentColor = ColorMaps.size() - 1;

				break;

			case SDLK_i:

				++ SetColorCounter;

				setSetColor();

				break;

			case SDLK_k:

				-- SetColorCounter;

				setSetColor();

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
