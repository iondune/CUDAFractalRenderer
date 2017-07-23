
#include <ionEngine.h>

#include "CMainState.h"


int main(int argc, char * argv[])
{
	Log::AddDefaultOutputs();

	CMainState & State = CMainState::Get();
	State.Run();

	return 0;
}
