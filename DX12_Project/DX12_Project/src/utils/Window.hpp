#pragma once
#include <stdio.h>
#include <assert.h>
#include <utils/Utility.hpp>
#include <utils/Input.hpp>
#include <iostream>

namespace dx
{
	class Window
	{
	public:
		static HWND InitWindow(HINSTANCE hInstance)
		{
#ifdef _DEBUG
			//Allocate console
			AllocConsole();
			AttachConsole(GetCurrentProcessId());
			freopen("CON", "w", stdout);
#endif

			//Create window
			WNDCLASSEX wcex = { 0 };
			wcex.cbSize = sizeof(WNDCLASSEX);
			wcex.lpfnWndProc = WndProc;
			wcex.hInstance = hInstance;
			wcex.lpszClassName = "D3D_12_DEMO";
			assert(RegisterClassEx(&wcex));

			RECT rc = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
			AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);

			return CreateWindowEx(
				WS_EX_OVERLAPPEDWINDOW,
				"D3D_12_DEMO",
				"Window",
				WS_OVERLAPPEDWINDOW,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				rc.right - rc.left,
				rc.bottom - rc.top,
				nullptr,
				nullptr,
				hInstance,
				nullptr);
		}

	private:
		static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
		{
			switch (message)
			{
			case WM_ACTIVATEAPP:
				Keyboard::ProcessMessage(message, wParam, lParam);
				Mouse::ProcessMessage(message, wParam, lParam);
				break;

			case WM_INPUT:
			case WM_MOUSEMOVE:
			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
			case WM_RBUTTONDOWN:
			case WM_RBUTTONUP:
			case WM_MBUTTONDOWN:
			case WM_MBUTTONUP:
			case WM_MOUSEWHEEL:
			case WM_XBUTTONDOWN:
			case WM_XBUTTONUP:
			case WM_MOUSEHOVER:
				Mouse::ProcessMessage(message, wParam, lParam);
				break;
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
			case WM_KEYUP:
			case WM_SYSKEYUP:
				Keyboard::ProcessMessage(message, wParam, lParam);
				break;
			case WM_DESTROY:
				PostQuitMessage(0);
				break;
			}

			return DefWindowProc(hwnd, message, wParam, lParam);
		}
	};
}
