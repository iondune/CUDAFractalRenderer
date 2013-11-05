
#include <ionEngine.h>

#include "CMainState.h"


int main(int argc, char * argv[])
{
	vec2i ScreenSize(1600, 900);

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
