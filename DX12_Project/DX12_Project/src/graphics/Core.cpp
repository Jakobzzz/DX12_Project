#include <graphics/Core.hpp>
#include <utils/Window.hpp>

namespace dx
{
	Core::Core(HINSTANCE hInstance)
	{
		//Create window
		m_hwnd = Window::InitWindow(hInstance);

		//Init the direct3D class
		m_direct3D = std::make_unique<D3D>();
		m_direct3D->Initialize(m_hwnd);
	}

	void Core::ShutDown()
	{
		m_direct3D->ShutDown();
	}

	void Core::Run()
	{
		MSG msg = { 0 };

		ShowWindow(m_hwnd, SW_SHOW);
		while (WM_QUIT != msg.message)
		{
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				m_direct3D->Render();
			}
		}

		DestroyWindow(m_hwnd);
	}
}