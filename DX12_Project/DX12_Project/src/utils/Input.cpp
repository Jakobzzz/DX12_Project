#include <utils/Input.hpp>

namespace dx
{
	//Static variables
	std::unique_ptr<Keyboard> Input::m_keyboard;
	std::unique_ptr<Mouse> Input::m_mouse;
	Keyboard::State Input::m_keyboardState;
	Mouse::State Input::m_mouseState;
	Keyboard::KeyboardStateTracker Input::m_keyboardTracker;
	Mouse::ButtonStateTracker Input::m_mouseTracker;

	void Input::Initialize(HWND wndHandle)
	{
		m_keyboard = std::make_unique<Keyboard>();
		m_mouse = std::make_unique<Mouse>();

		m_mouse->SetWindow(wndHandle);
		m_mouse->SetMode(Mouse::MODE_RELATIVE); //Set relative mode for mouse when using FPS-camera
	}

	void Input::SetMouseInputMode(Mouse::Mode mode)
	{
		m_mouse->SetMode(mode);
	}

	void Input::Update()
	{
		//Get state for keyboard and mouse
		m_keyboardState = m_keyboard->GetState();
		m_mouseState = m_mouse->GetState();

		//Update mouse and keyboard tracker
		m_keyboardTracker.Update(m_keyboardState);
		m_mouseTracker.Update(m_mouseState);
	}

	void Input::ResetScrollWheelValue()
	{
		m_mouse->ResetScrollWheelValue();
	}

	bool Input::GetKeyDown(Keyboard::Keys key)
	{
		return m_keyboardTracker.IsKeyPressed(key);
	}

	bool Input::GetKeyUp(Keyboard::Keys key)
	{
		return m_keyboardTracker.IsKeyReleased(key);
	}

	bool Input::GetKey(Keyboard::Keys key)
	{
		return m_keyboardState.IsKeyDown(key);
	}

	bool Input::GetMouseButtonDown(MouseButton button)
	{
		switch (button)
		{
		case MouseButton::LEFT:
			return (m_mouseTracker.leftButton == m_mouseTracker.PRESSED);
		case MouseButton::RIGHT:
			return (m_mouseTracker.rightButton == m_mouseTracker.PRESSED);
		case MouseButton::MIDDLE:
			return (m_mouseTracker.middleButton == m_mouseTracker.PRESSED);
		}

		return false;
	}

	bool Input::GetMouseButtonUp(MouseButton button)
	{
		switch (button)
		{
		case MouseButton::LEFT:
			return (m_mouseTracker.leftButton == m_mouseTracker.RELEASED);
		case MouseButton::RIGHT:
			return (m_mouseTracker.rightButton == m_mouseTracker.RELEASED);
		case MouseButton::MIDDLE:
			return (m_mouseTracker.middleButton == m_mouseTracker.RELEASED);
		}
		return false;
	}

	bool Input::GetMouseButton(MouseButton button)
	{
		switch (button)
		{
		case MouseButton::LEFT:
			return m_mouseState.leftButton;
		case MouseButton::RIGHT:
			return m_mouseState.rightButton;
		case MouseButton::MIDDLE:
			return m_mouseState.middleButton;
		}
		return false;
	}

	int Input::GetMousePositionX()
	{
		return m_mouseState.x;
	}

	int Input::GetMousePositionY()
	{
		return m_mouseState.y;
	}

	int Input::GetMouseScrollWheel()
	{
		return m_mouseState.scrollWheelValue;
	}
}


