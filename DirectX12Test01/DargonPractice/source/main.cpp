#include <iostream>
#include <future>

#include <Init.h>
//#include <Box_C6.h>
#include <C7/C7_Geometry/Shapes_C7.hpp>
#include <C7/C7_LandAndWave/LandAndWaves_C7.hpp>



//int Init_Main(HINSTANCE hInstance)
//{
//#if defined(DEBUG) | defined(_DEBUG)
//	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
//#endif
//
//	try
//	{
//		//InitDirect3DApp theApp(hInstance);
//		ShapesApp theApp(hInstance);
//		if (!theApp.Initialize())
//		{
//			return 0;
//		}
//		return theApp.Run();
//	}
//	catch (const DxException& e)
//	{
//		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
//		return 0;
//	}
//}

//int Box_C6_Main(HINSTANCE hInstance)
//{
//#if defined(DEBUG) | defined(_DEBUG)
//	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
//#endif
//	try
//	{
//		BoxApp theApp(hInstance);
//		if (!theApp.Initialize())
//		{
//			return 0;
//		}
//		return theApp.Run();
//	}
//	catch (const DxException& e)
//	{
//		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
//		return 0;
//	}
//}


int WINAPI main(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{

#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	//return C7_Geometry::Main(hInstance);
	return C7_LandAndWaves::Main(hInstance);
}
