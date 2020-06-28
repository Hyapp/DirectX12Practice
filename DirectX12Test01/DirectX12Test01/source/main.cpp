#include <stdafx.h>

#include <HelloWindow.h>
int HelloWindow(HINSTANCE hInstance, int nCmdShow)
{
	D3D12HelloWindow sample(1280, 720, L"D3D12 Hello Window");
	return Win32Application::Run(&sample, hInstance, nCmdShow);
}

#include <hellowTriangle.h>
int TestTriangle(HINSTANCE hInstance, int nCmdShow)
{
	D3D12HellowTriangle sample(1280, 720, L"D3D12 Hello Triangle");
	return Win32Application::Run(&sample, hInstance, nCmdShow);
}

#include <ConstBuffer.h>
int TestConstBuffer(HINSTANCE hInstance, int nCmdShow)
{
	D3D12HelloConstBuffer sample(1280, 720, L"D3D12 Hello Constant Buffer");
	return Win32Application::Run(&sample, hInstance, nCmdShow);
}

#include <TestFrameBuffer.h>
int TestFrameBuffer(HINSTANCE hInstance, int nCmdShow)
{
	D3D12HelloFrameBuffering sample(1280, 720, L"D3D12 Hello Frame Buffering");
	return Win32Application::Run(&sample, hInstance, nCmdShow);
}

#include<TestBundles.h>
int TestBundles(HINSTANCE hInstance, int nCmdShow)
{
	D3D12HellowBoundles sample(1280, 720, L"D3D12 Hello Bundle");
	return Win32Application::Run(&sample, hInstance, nCmdShow);
}

#include <TestTexture.h>
int TestTexture(HINSTANCE hInstance, int nCmdShow)
{
	D3D12HelloTexture sample(1280, 720, L"D3D12 Hello Texture");
	return Win32Application::Run(&sample, hInstance, nCmdShow);
}

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	TestFrameBuffer(hInstance, nCmdShow);
	//TestTriangle(hInstance, nCmdShow);
}