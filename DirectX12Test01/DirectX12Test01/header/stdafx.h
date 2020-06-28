#pragma once

//	Cpp ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//		Stream
#include <iostream>
#include <fstream>
#include <sstream>

//		Containor
#include <vector>
#include <deque>
#include <array>
#include <unordered_set>
#include <unordered_map>
#include <string>

//		Iterator
#include <iterator>

//		Algorithm
#include <algorithm>

//		Math
#include <numbers>
#include <numeric>
#include <limits>
#include <cmath>
#include <random>

//		Time
#include <chrono>

//		Thread and Atomic
#include <thread>
#include <future>
#include <mutex>
#include <condition_variable>
#include <atomic>

//	Cpp ----------------------------------------------------------------------------------------------------

//	Windows ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

#include <windows.h>
#include <wrl.h>
#include <shellapi.h>

//	Windows ------------------------------------------------------------------------------------------------

//	Dx +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "d3dx12.h"

//	Dx -----------------------------------------------------------------------------------------------------


//#pragma comment(lib,"d3d12.lib")
//#pragma comment(lib, "dxgi.lib")
//#pragma comment(lib, "d3dcompiler.lib")