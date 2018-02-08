#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include <memory>
#include <graphics/D3D.hpp>
#include <graphics/Shader.hpp>
#include <graphics/Model.hpp>

class Core
{
public:
	Core();

public:
	bool Initialize(HINSTANCE hInstance);
	void ShutDown();
	void Run();

private:
	void Update();

private:
	HWND m_hwnd;

private:
	std::unique_ptr<D3D> m_direct3D;
	std::unique_ptr<Shader> m_colorShader;
	std::unique_ptr<Model> m_model;
};