#pragma once
#include <Windows.h>
#include <stdio.h>
#include <assert.h>
#include <utils/Utility.hpp>

class Window
{
public:
	static HWND InitWindow(HINSTANCE hInstance)
	{
		//Allocate console
		AllocConsole();
		AttachConsole(GetCurrentProcessId());
		freopen("CON", "w", stdout);

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
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
	{
		switch (umessage)
		{
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			break;
		}
		}

		return DefWindowProc(hwnd, umessage, wparam, lparam);
	}
};