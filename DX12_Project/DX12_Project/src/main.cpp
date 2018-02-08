#include <graphics/Core.hpp>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	Core core;
	if(!core.Initialize(hInstance))
		core.Run();

	core.ShutDown();
	return EXIT_SUCCESS;
}