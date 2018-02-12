#include <graphics/Camera.hpp>
#include <utils/Input.hpp>
#include <utils/Utility.hpp>

namespace dx
{
	Camera::Camera(const Vector3 & camPosition, const Vector3 & camTarget, const Vector3 & camUp, const float & speed, const float & sensitivity) :
				   m_cameraPos(camPosition), m_camTarget(camTarget), m_camUp(camUp), m_movementSpeed(speed), m_mouseSensivity(sensitivity),
				   m_camYaw(0.f), m_camPitch(0.f)
	{
	}

	void Camera::Update(const float & dt)
	{
		MoveCamera(dt);
		RotateCamera();
	}

	void Camera::MoveCamera(const float & dt)
	{
		float velocity = m_movementSpeed * dt;

		//Move camera
		if (Input::GetKey(Keyboard::Keys::W))
			m_cameraPos += m_camTarget * velocity;
		if (Input::GetKey(Keyboard::Keys::S))
			m_cameraPos -= m_camTarget * velocity;
		if (Input::GetKey(Keyboard::Keys::D))
			m_cameraPos += XMVector3Normalize(XMVector3Cross(m_camUp, m_camTarget)) * velocity;
		if (Input::GetKey(Keyboard::Keys::A))
			m_cameraPos -= XMVector3Normalize(XMVector3Cross(m_camUp, m_camTarget)) * velocity;
	}

	void Camera::RotateCamera()
	{
		if (Mouse::MODE_RELATIVE)
		{
			float currMousePosX = static_cast<float>(Input::GetMousePositionX());
			float currMousePosY = static_cast<float>(Input::GetMousePositionY());

			//As we use relative mouse-coordinates delta is already calculated
			Vector2 mouseDelta = Vector2(currMousePosX, currMousePosY) * m_mouseSensivity;
			m_camYaw += mouseDelta.x;
			m_camPitch += mouseDelta.y;

			//Restrict pitch angle
			if (m_camPitch > 89.0f)
				m_camPitch = 89.0f;

			if (m_camPitch < -89.0f)
				m_camPitch = -89.0f;

			//Calculate target vector with Euler angles
			m_rotationMatrix = XMMatrixRotationRollPitchYaw(XMConvertToRadians(m_camPitch), XMConvertToRadians(m_camYaw), 0.f);
			m_camTarget = XMVector3TransformCoord(Vector3(0.0f, 0.0f, 1.0f), m_rotationMatrix);
			m_camTarget = XMVector3Normalize(m_camTarget);

			//Set view matrix
			m_viewMatrix = XMMatrixLookAtLH(m_cameraPos, m_camTarget + m_cameraPos, m_camUp);
		}
	}

	Matrix Camera::GetViewMatrix() const
	{
		return m_viewMatrix;
	}

	Matrix Camera::GetViewProjectionMatrix() const
	{
		return m_viewMatrix * GetProjectionMatrix();
	}

	Matrix Camera::GetProjectionMatrix() const
	{
		return XMMatrixPerspectiveFovLH(0.4f * 3.14f, static_cast<float>(SCREEN_WIDTH / SCREEN_HEIGHT), 0.1f, 1000.0f);
	}

	Vector3 Camera::GetCameraPosition() const
	{
		return m_cameraPos;
	}
}

