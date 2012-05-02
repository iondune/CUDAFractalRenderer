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

public:

	CMainState()
	{}

	void begin()
	{
		SDL_WM_SetCaption("Dinosaurs in Space!", "");

        glClearColor(0.6f, 0.6f, 0.6f, 1.0f);
		glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

	}

	void OnRenderStart(float const Elapsed)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        SDL_GL_SwapBuffers();
	}

	
    void OnKeyboardEvent(SKeyboardEvent const & Event)
    {
        switch (Event.Key)
        {


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

int main(int argc, char * argv[])
{
	CTextureLoader::ImageDirectory = "Media/";
	CMeshLoader::MeshDirectory = "Media/";
	CShaderLoader::ShaderDirectory = "Shaders/";

	CApplication & Application = CApplication::get();
	Application.init(SPosition2(1600, 900));

	Application.getStateManager().setState(& CMainState::get());

	Application.run();

	return 0;
}
