#include <graphics/Core.hpp>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	dx::Core core(hInstance);
	core.Run();
	core.ShutDown();

	return EXIT_SUCCESS;
}