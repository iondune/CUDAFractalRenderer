
#include <ionWindow.h>
#include <ionScene.h>
#include <ionFramework.h>

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

class CMainState : public CContextState<CMainState>
{

	CShader * Shader[EFT_COUNT][ESS_COUNT];

	int CurrentFractal;
	int CurrentSettings;
	int CurrentColor;

	std::vector<CTexture *> ColorMaps;
	vec3f uSetColor;
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
			uSetColor = vec3f(0.f);
			break;
		case 1:
			uSetColor = vec3f(1.f);
			break;
		case 2:
			uSetColor = vec3f(0.9f, 0.2f, 0.1f);
			break;
		case 3:
			uSetColor = vec3f(0.2f, 0.9f, 0.1f);
			break;
		case 4:
			uSetColor = vec3f(0.1f, 0.2f, 0.9f);
			break;
		case 5:
			uSetColor = vec3f(0.9f, 0.8f, 0.1f);
			break;
		case 6:
			uSetColor = vec3f(0.8f, 0.6f, 0.4f);
			break;
		case 7:
			uSetColor = vec3f(0.4f, 0.6f, 0.8f);
			break;
		case 8:
			uSetColor = vec3f(0.9f, 0.3f, 0.8f);
			break;
		};
	}

public:

	static GLuint QuadHandle;

	CMainState()
		: sX(1.0), sY(1.0), cX(0.0), cY(0.7), max_iteration(1000), uSetColor(0.0f), ScaleFactor(1), TextureScaling(1.f),
		CurrentFractal(EFT_MANDEL), CurrentSettings(ESS_DEFAULT), CurrentColor(0), SetColorCounter(0)
	{}

	void Begin()
	{
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
		Shader[EFT_MULTIBROT_2][ESS_MS2] = CShaderLoader::loadShader("QuadCopyUV.glsl", "Multibrot2-2x2MS.frag");
		Shader[EFT_MULTIBROT_2][ESS_MS3] = CShaderLoader::loadShader("QuadCopyUV.glsl", "Multibrot2-3x3MS.frag");
		Shader[EFT_MULTIBROT_2][ESS_MS4] = CShaderLoader::loadShader("QuadCopyUV.glsl", "Multibrot2-4x4MS.frag");
		
		//Shader[EFT_JULIA][ESS_MS3] = CShaderLoader::loadShader("QuadCopyUV.glsl", "Multibrot1.frag");

		STextureCreationFlags Flags;
		Flags.Wrap = GL_MIRRORED_REPEAT;
		Flags.MipMaps = false;
		
		ColorMaps.push_back(new CTexture(CImageLoader::LoadImage("Spectrum1.bmp"), Flags));
		ColorMaps.push_back(new CTexture(CImageLoader::LoadImage("Spectrum2.bmp"), Flags));
		ColorMaps.push_back(new CTexture(CImageLoader::LoadImage("Quadrant1.bmp"), Flags));
		ColorMaps.push_back(new CTexture(CImageLoader::LoadImage("Quadrant2.bmp"), Flags));
	}

	class SPostProcessPass
	{

	public:

		SPostProcessPass();

		CShader * Shader;

		std::map<std::string, float> Floats;
		std::map<std::string, double> Doubles;
		std::map<std::string, int> Ints;
		std::map<std::string, vec3f> Vector3s;
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
		printf("sX: %f   sY: %f   cX: %.15f   cY: %.15f\n", sX, sY, cX, cY);
	}

	void Update(f32 const Elapsed)
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
				Pass.Ints["uScreenWidth"] = Application->GetWindow().GetSize().X;
				Pass.Ints["uScreenHeight"] = Application->GetWindow().GetSize().Y;
			}

			Pass.doPass();
		}

		Application->GetWindow().SwapBuffers();
	}


	void OnEvent(SKeyboardEvent & Event)
	{
		if (! Event.Pressed)
		{
			static double const MoveSpeed = 0.07;
			switch (Event.Key)
			{

			case EKey::W:

				cY += (CurrentFractal == EFT_BURNING_SHIP ? -1 : 1) * sY * MoveSpeed;
				printLocation();
				break;

			case EKey::A:

				cX -= sX * MoveSpeed;
				printLocation();
				break;

			case EKey::S:

				cY -= (CurrentFractal == EFT_BURNING_SHIP ? -1 : 1) * sY * MoveSpeed;
				printLocation();
				break;

			case EKey::D:

				cX += sX * MoveSpeed;
				printLocation();
				break;

			case EKey::Comma:

				++ CurrentFractal;
				if (CurrentFractal >= EFT_COUNT)
					CurrentFractal = 0;
				std::cout << "Fractal: " << CurrentFractal << std::endl;
				break;

			case EKey::M:

				++ CurrentSettings;
				if (CurrentSettings >= ESS_COUNT)
					CurrentSettings = 0;
				std::cout << "Multisample: " << CurrentSettings << std::endl;
				break;

			case EKey::Num1:
			case EKey::Num2:
			case EKey::Num3:
			case EKey::Num4:
			case EKey::Num5:
			case EKey::Num6:
			case EKey::Num7:
			case EKey::Num8:

				CurrentSettings = (int) Event.Key - (int) EKey::Num1;
				if (CurrentSettings >= ESS_COUNT)
					CurrentSettings = ESS_COUNT - 1;
				std::cout << "Multisample: " << CurrentSettings << std::endl;
				break;

			case EKey::Z:

				sX *= 0.5;
				sY *= 0.5;
				break;

			case EKey::X:

				sX *= 2.0;
				sY *= 2.0;
				break;

			case EKey::Q:

				sX *= 0.75;
				sY *= 0.75;
				break;

			case EKey::E:

				sX *= 1.33;
				sY *= 1.33;
				break;
				
			case EKey::G:
				
				if (max_iteration)
					max_iteration *= 2;
				else
					++ max_iteration;
				
				printf("iteration cap: %d\n", max_iteration);
				
				break;

			case EKey::B:
				
				max_iteration /= 2;
				
				printf("iteration cap: %d\n", max_iteration);
				
				break;
				
			case EKey::H:
				
				ScaleFactor --;
				recalcScale();
				
				break;
				
			case EKey::N:
				
				ScaleFactor ++;
				recalcScale();
				
				break;

			case EKey::U:

				++ CurrentColor;
				if (CurrentColor >= (int) ColorMaps.size())
					CurrentColor = 0;

				break;

			case EKey::J:

				-- CurrentColor;
				if (CurrentColor < 0)
					CurrentColor = ColorMaps.size() - 1;

				break;

			case EKey::I:

				++ SetColorCounter;

				setSetColor();

				break;

			case EKey::K:

				-- SetColorCounter;

				setSetColor();

				break;
				
			}
		}
	}

	void OnEvent(SMouseEvent & Event)
	{
		switch (Event.Type)
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

void CMainState::SPostProcessPass::doPass()
{
	begin();
	end();
}

int main(int argc, char * argv[])
{
	vec2i ScreenSize(900, 900);

	CShaderLoader::ShaderDirectory = "Shaders/";
	CImageLoader::ImageDirectory = "Media/";
	CMeshLoader::MeshDirectory = "Media/";
	
	CApplication & Application = CApplication::Get();
	Application.Init(ScreenSize, "Fractal Renderer", false);
	Application.GetSceneManager().init(true, true);

	if (! GLEW_ARB_gpu_shader_fp64)
	{
		std::cout << "FUCK WE DON'T HAVE DOUBLES IN SHADERS FUCK" << std::endl;
	}

	CMainState & State = CMainState::Get();

	Application.GetStateManager().SetState(& State);
	Application.Run();

	return 0;
}
