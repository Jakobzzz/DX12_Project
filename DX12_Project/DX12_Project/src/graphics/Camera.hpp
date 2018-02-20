#pragma once
#include <d3d12.h>
#include <SimpleMath.h>

using namespace DirectX;
using namespace DirectX::SimpleMath;

namespace dx
{
	class Camera
	{
	public:
		Camera(const Vector3 & camPosition = Vector3(0.0f, 0.0f, 0.0f), const Vector3 & camTarget = Vector3(0.0f, 0.0f, -50.0f),
			   const Vector3 & camUp = Vector3(0.0f, 1.0f, 0.0f), const float & speed = 250.0f, const float & sensitivity = 0.3f);
		void Update(const float & dt);

	public:
		Matrix GetViewMatrix() const;
		Matrix GetViewProjectionMatrix() const;
		Matrix GetProjectionMatrix() const;
		Vector3 GetCameraPosition() const;

	private:
		void MoveCamera(const float & dt);
		void RotateCamera();

	private:
		Vector3 m_cameraPos;
		Vector3 m_camTarget;
		Vector3 m_camUp;
		Matrix m_rotationMatrix;
		Matrix m_viewMatrix;

	private:
		float m_camYaw;
		float m_camPitch;
		float m_movementSpeed;
		float m_mouseSensivity;
	};
}



