#include <graphics/Core.hpp>
#include <utils/Window.hpp>

Core::Core() : m_direct3D(nullptr), m_model(nullptr)
{
}

bool Core::Initialize(HINSTANCE hInstance)
{
	//Create window
	m_hwnd = Window::InitWindow(hInstance);

	//Create objects
	m_direct3D = std::make_unique<D3D>();

	//Init classes
	if (!m_direct3D->Initialize(m_hwnd))
	{
		MessageBox(m_hwnd, (LPCSTR)L"Could not initialize Direct3D", (LPCSTR)L"Error", MB_OK);
		return false;
	}

	m_model = std::make_unique<Model>(m_direct3D->GetDevice(), m_direct3D->GetCommandList());

	m_direct3D->ExecuteCommandList();
	m_direct3D->WaitForPreviousFrame();

	return true;
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
			Update();
		}
	}

	DestroyWindow(m_hwnd);
}

void Core::Update()
{
	m_direct3D->BeginScene();

	m_model->Render(m_direct3D->GetCommandList());

	m_direct3D->EndScene();
}