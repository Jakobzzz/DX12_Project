#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <graphics/D3D.hpp>

namespace dx
{
	class Core
	{
	public:
		Core(HINSTANCE hInstance);
		void ShutDown();
		void Run();

	private:
		HWND m_hwnd;

	private:
		std::unique_ptr<D3D> m_direct3D;
	};
}