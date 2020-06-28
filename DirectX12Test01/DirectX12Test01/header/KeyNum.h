#pragma once
#include <Windows.h>
namespace Utility
{
	enum class KeyNum : UINT8
	{
		tap = 9,

		enter = 13,

		l_shift = 16,
		r_shift = 16,
		l_ctrl = 17,
		r_ctrl = 17,

		cap = 20,

		esc = 27,

		_0 = 48,
		_1 = 49,
		_2 = 50,
		_3 = 51,
		_4 = 52,
		_5 = 53,
		_6 = 54,
		_7 = 55,
		_8 = 56,
		_9 = 57,

		num_0 = 96,
		num_1 = 97,
		num_2 = 98,
		num_3 = 99,
		num_4 = 100,
		num_5 = 101,
		num_6 = 102,
		num_7 = 103,
		num_8 = 104,
		num_9 = 105,

		inv_q = 192,
	};
}