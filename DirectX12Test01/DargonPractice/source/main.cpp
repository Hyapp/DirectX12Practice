#include <iostream>
#include <future>

#include <C4/Init.hpp>
#include <C6/Box_C6.hpp>
#include <C7/C7_Geometry/Shapes_C7.hpp>
#include <C7/C7_LandAndWave/LandAndWaves_C7.hpp>


int WINAPI main(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
	//return C4::Main(hInstance);
	//return C6::Main(hInstance);
	//return C7_Geometry::Main(hInstance);
	return C7_LandAndWaves::Main(hInstance);
}
