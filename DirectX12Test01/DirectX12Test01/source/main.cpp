#include <stdafx.h>
//#include "../header/DirectX12Test01.h"

//_Use_decl_annotations_
//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
//{
//    D3D12HelloWindow sample(1280, 720, L"D3D12 Hello Window");
//    return Win32Application::Run(&sample, hInstance, nCmdShow);
//}

#include "../header/hellowTriangle.h"

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	D3D12HellowTriangle sample(1280, 720, L"D3D12 Hello Triangle");
	return Win32Application::Run(&sample, hInstance, nCmdShow);
}

//#include "../header/ConstBuffer.h"
//
//_Use_decl_annotations_
//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
//{
//	D3D12HelloConstBuffer sample(1280, 720, L"D3D12 Hello Constant Buffer");
//	return Win32Application::Run(&sample, hInstance, nCmdShow);
//}