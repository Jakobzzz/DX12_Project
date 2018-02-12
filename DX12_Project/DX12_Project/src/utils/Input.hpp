#pragma once
#include <Keyboard.h>
#include <Mouse.h>

using namespace DirectX;

namespace dx
{
	class Input
	{
	public:
		enum class MouseButton
		{
			LEFT,
			RIGHT,
			MIDDLE
		};

	public:
		static void Initialize(HWND wndHandle);
		static void Update();

	public:
		static void SetMouseInputMode(Mouse::Mode mode);

	public:
		static bool GetKeyDown(Keyboard::Keys key);
		static bool GetKeyUp(Keyboard::Keys key);
		static bool GetKey(Keyboard::Keys key);
		static bool GetMouseButtonDown(MouseButton button);
		static bool GetMouseButtonUp(MouseButton button);
		static bool GetMouseButton(MouseButton button);
		static int GetMousePositionX();
		static int GetMousePositionY();
		static int GetMouseScrollWheel();

	private:
		//Keyboard
		static std::unique_ptr<Keyboard> m_keyboard;
		static Keyboard::State m_keyboardState;
		static Keyboard::KeyboardStateTracker m_keyboardTracker;

	private:
		//Mouse
		static std::unique_ptr<Mouse> m_mouse;
		static Mouse::State m_mouseState;
		static Mouse::ButtonStateTracker m_mouseTracker;
	};
}
